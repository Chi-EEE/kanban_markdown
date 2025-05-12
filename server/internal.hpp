#pragma once

#include <cstdint>
#include <string_view>
#include <iomanip>

#include <yyjson.h>

#include <kanban_markdown/kanban_board.hpp>
#include <kanban_markdown/utils.hpp>

namespace server
{
	static std::vector<std::string> split(std::string s, std::string delimiter) {
		size_t pos_start = 0, pos_end, delim_len = delimiter.length();
		std::string token;
		std::vector<std::string> res;

		while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
			token = s.substr(pos_start, pos_end - pos_start);
			pos_start = pos_end + delim_len;
			res.push_back(token);
		}

		res.push_back(s.substr(pos_start));
		return res;
	}

	static inline std::string yyjson_get_string_object(yyjson_val* val)
	{
		const char* val_data = yyjson_get_str(val);
		int val_size = yyjson_get_len(val);
		return std::string(val_data, val_size);
	}

	static inline std::string urlDecode(std::string text)
	{
		std::string escaped;

		for (auto i = text.begin(), nd = text.end(); i < nd; ++i)
		{
			auto c = (*i);

			switch (c)
			{
			case '%':
				if (i[1] && i[2]) {
					char hs[]{ i[1], i[2] };
					escaped += static_cast<char>(strtol(hs, nullptr, 16));
					i += 2;
				}
				break;
			case '+':
				escaped += ' ';
				break;
			default:
				escaped += c;
			}
		}

		return escaped;
	}

	struct KanbanTuple
	{
		std::string file_path;
		kanban_markdown::KanbanBoard kanban_board;
	};
}
