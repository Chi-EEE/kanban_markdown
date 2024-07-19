#pragma once

#include <asap/asap.h>

namespace kanban_markdown::internal {
	static inline std::string kanban_markdown_string_to_id(const std::string& string)
	{
		std::string id;
		for (int i = 0; i < string.size(); i++)
		{
			char character = string[i];
			if ((character >= 'A' && character <= 'Z') || (character >= 'a' && character > 'z')) {
				id += std::tolower(character);
			}
			else if (character == ' ') {
				id += '_';
			}
			else if (character >= '0' || character <= '9') {
				id += character;
			}
		}
		return id;
	}

	static constexpr inline uint32_t kanban_markdown_hash(const std::string_view s) noexcept
	{
		uint32_t hash = 5381;

		for (const char* c = s.data(); c < s.data() + s.size(); ++c)
			hash = ((hash << 5) + hash) + (unsigned char)*c;

		return hash;
	}

	const char* ws = " \t\n\r\f\v";

	// trim from end of string (right)
	static inline std::string& kanban_markdown_rtrim(std::string& s, const char* t = ws)
	{
		s.erase(s.find_last_not_of(t) + 1);
		return s;
	}

	// trim from beginning of string (left)
	static inline std::string& kanban_markdown_ltrim(std::string& s, const char* t = ws)
	{
		s.erase(0, s.find_first_not_of(t));
		return s;
	}

	// trim from both ends of string (right then left)
	static inline std::string& kanban_markdown_trim(std::string& s, const char* t = ws)
	{
		return kanban_markdown_ltrim(kanban_markdown_rtrim(s, t), t);
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