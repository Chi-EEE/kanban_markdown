#include <re2/re2.h>

namespace server::constants {
	static const re2::RE2 vertical_whitespace_regex_pattern("\\v");
}