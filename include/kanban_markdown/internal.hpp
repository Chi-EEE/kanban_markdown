#pragma once

#include <chrono>

#include <asap/asap.h>

namespace kanban_markdown::internal {
	static inline std::string string_to_id(const std::string& string)
	{
		std::string buffer;
		buffer.reserve(string.size() * 1.1);
		for (char character : string) {
			switch (character) {
			case ' ':
				buffer += '_';
				break;
			case '<':
				buffer += "&lt;";
				break;
			case '>':
				buffer += "&gt;";
				break;
			case '&':
				buffer += "&amp;";
				break;
			case '"':
				buffer += "&quot;";
				break;
			case '\'':
				buffer += "&apos;";
				break;
			default:
				if ((character >= 'A' && character <= 'Z') || (character >= 'a' && character <= 'z')) {
					buffer += std::tolower(character);
				}
				else {
					buffer += character;
				}
				break;
			}
		}
		return buffer;
	}

	static constexpr inline uint32_t hash(const std::string_view s) noexcept
	{
		uint32_t hash = 5381;

		for (const char* c = s.data(); c < s.data() + s.size(); ++c)
			hash = ((hash << 5) + hash) + (unsigned char)*c;

		return hash;
	}

	const char* ws = " \t\n\r\f\v";

	// trim from end of string (right)
	static inline std::string& rtrim(std::string& s, const char* t = ws)
	{
		s.erase(s.find_last_not_of(t) + 1);
		return s;
	}

	// trim from beginning of string (left)
	static inline std::string& ltrim(std::string& s, const char* t = ws)
	{
		s.erase(0, s.find_first_not_of(t));
		return s;
	}

	// trim from both ends of string (right then left)
	static inline std::string& trim(std::string& s, const char* t = ws)
	{
		return ltrim(rtrim(s, t), t);
	}

	static inline bool to_bool(std::string str) {
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);
		std::istringstream is(str);
		bool b;
		is >> std::boolalpha >> b;
		return b;
	}

	static inline asap::datetime now_utc() {
		auto now = std::chrono::system_clock::now();
		std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
		std::tm* utc_tm = std::gmtime(&now_time_t);
		asap::datetime now_utc;
		now_utc.when = *utc_tm;
		return now_utc;
	}
}