// Text encoding helpers (CONV-05).
#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace dp_hello {

/// Converts UTF-16 to UTF-8. A valid surrogate pair becomes one 4-byte sequence; an unpaired
/// surrogate is encoded on its own (3 bytes), so no input is lost or rejected.
[[nodiscard]] std::string utf16_to_utf8(std::u16string_view text);

/// Length in bytes of the UTF-8 sequence that starts at text[0], capped at text.size().
/// Invalid lead bytes count as one byte. Returns 0 for an empty view.
[[nodiscard]] std::size_t utf8_sequence_length(std::string_view text);

}  // namespace dp_hello
