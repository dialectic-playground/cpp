#include "dp_hello/cli.hpp"

#include <array>
#include <cstddef>
#include <string_view>
#include <utility>

#include "dp_hello/encoding.hpp"

namespace dp_hello {
namespace {

enum class OptionId { name, count, help, version };

struct OptionSpec {
  OptionId id;
  char short_name;  // '\0' when the option has no short form
  std::string_view long_name;
  bool takes_value;
};

constexpr std::array<OptionSpec, 4> kOptions{{
    {OptionId::name, 'n', "name", true},
    {OptionId::count, 'c', "count", true},
    {OptionId::help, '\0', "help", false},
    {OptionId::version, '\0', "version", false},
}};

const OptionSpec* find_short_option(std::string_view letter) {
  for (const OptionSpec& option : kOptions) {
    if (option.short_name != '\0' && letter.size() == 1 && letter[0] == option.short_name) {
      return &option;
    }
  }
  return nullptr;
}

// Exact match only: long options are never abbreviated (CONV-08).
const OptionSpec* find_long_option(std::string_view name) {
  for (const OptionSpec& option : kOptions) {
    if (option.long_name == name) {
      return &option;
    }
  }
  return nullptr;
}

UsageError make_error(std::string_view before, std::string_view value, std::string_view after) {
  std::string message;
  message.reserve(before.size() + value.size() + after.size());
  message.append(before).append(value).append(after);
  return UsageError{std::move(message)};
}

// One left-to-right scan over the arguments (CONV-12). Each token handler returns an error,
// or nothing; `finished_` becomes true once --help or --version has been reached.
class ArgumentScanner {
 public:
  explicit ArgumentScanner(std::span<const std::string> arguments) : arguments_(arguments) {}

  ParseResult scan() {
    while (!finished_ && next_index_ < arguments_.size()) {
      const std::string_view token = arguments_[next_index_++];
      std::optional<UsageError> error;
      if (token == "--") {
        take_remaining_as_operands();
      } else if (token.starts_with("--")) {
        error = scan_long_option(token);
      } else if (token.size() > 1 && token.front() == '-') {
        error = scan_short_options(token);
      } else {
        parsed_.operands.emplace_back(token);
      }
      if (error) {
        return *std::move(error);
      }
    }
    return std::move(parsed_);
  }

 private:
  bool has_next_argument() const { return next_index_ < arguments_.size(); }

  std::string take_next_argument() { return arguments_[next_index_++]; }

  void take_remaining_as_operands() {
    while (has_next_argument()) {
      parsed_.operands.push_back(take_next_argument());
    }
  }

  // "--name", "--name=VALUE", "--name VALUE", "--help", "--version" (CONV-08).
  std::optional<UsageError> scan_long_option(std::string_view token) {
    const std::string_view body = token.substr(2);
    const std::size_t equals = body.find('=');
    const std::string_view name = body.substr(0, equals);
    const bool has_inline_value = equals != std::string_view::npos;

    const OptionSpec* option = find_long_option(name);
    if (option == nullptr) {
      return make_error("unrecognized option '", token, "'");
    }
    if (!option->takes_value) {
      if (has_inline_value) {
        return make_error("option '--", name, "' doesn't allow an argument");
      }
      store_flag(*option);
      return std::nullopt;
    }
    std::string value;
    if (has_inline_value) {
      value = body.substr(equals + 1);
    } else if (has_next_argument()) {
      value = take_next_argument();
    } else {
      return make_error("option '--", name, "' requires an argument");
    }
    store_value(*option, std::move(value), "--" + std::string(name));
    return std::nullopt;
  }

  // "-n NAME", "-nNAME", and bundles such as "-c2" (CONV-07). An option that takes a value
  // consumes the rest of the token, or the next argument when the token ends.
  std::optional<UsageError> scan_short_options(std::string_view token) {
    std::size_t position = 1;
    while (position < token.size()) {
      const std::size_t letter_length = utf8_sequence_length(token.substr(position));
      const std::string_view letter = token.substr(position, letter_length);
      position += letter_length;

      const OptionSpec* option = find_short_option(letter);
      if (option == nullptr) {
        return make_error("invalid option -- '", letter, "'");
      }
      if (!option->takes_value) {
        store_flag(*option);
        if (finished_) {
          return std::nullopt;
        }
        continue;
      }
      std::string value;
      if (position < token.size()) {
        value = token.substr(position);
      } else if (has_next_argument()) {
        value = take_next_argument();
      } else {
        return make_error("option requires an argument -- '", letter, "'");
      }
      store_value(*option, std::move(value), "-" + std::string(letter));
      return std::nullopt;
    }
    return std::nullopt;
  }

  void store_flag(const OptionSpec& option) {
    if (option.id == OptionId::help) {
      parsed_.action = Action::help;
      finished_ = true;
    } else if (option.id == OptionId::version) {
      parsed_.action = Action::version;
      finished_ = true;
    }
  }

  // Last occurrence wins (CONV-11); validation happens later, on the winner only (FR-06).
  void store_value(const OptionSpec& option, std::string value, std::string spelling) {
    OptionValue stored{std::move(value), std::move(spelling)};
    if (option.id == OptionId::name) {
      parsed_.name = std::move(stored);
    } else if (option.id == OptionId::count) {
      parsed_.count = std::move(stored);
    }
  }

  std::span<const std::string> arguments_;
  std::size_t next_index_ = 0;
  ParsedArguments parsed_;
  bool finished_ = false;
};

}  // namespace

ParseResult parse_arguments(std::span<const std::string> arguments) {
  return ArgumentScanner(arguments).scan();
}

}  // namespace dp_hello
