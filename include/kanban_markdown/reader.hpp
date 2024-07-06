#pragma once

#include <iostream>
#include <vector>
#include <fmt/format.h>
#include <tl/expected.hpp>
#include <algorithm> 
#include <cctype>
#include <locale>

#include <re2/re2.h>

#include "kanban.hpp"
#include "constants.hpp"

namespace kanban_markdown {
	const char* ws = " \t\n\r\f\v";

	// trim from end of string (right)
	inline std::string& rtrim(std::string& s, const char* t = ws)
	{
		s.erase(s.find_last_not_of(t) + 1);
		return s;
	}

	// trim from beginning of string (left)
	inline std::string& ltrim(std::string& s, const char* t = ws)
	{
		s.erase(0, s.find_first_not_of(t));
		return s;
	}

	// trim from both ends of string (right then left)
	inline std::string& trim(std::string& s, const char* t = ws)
	{
		return ltrim(rtrim(s, t), t);
	}

	tl::expected<KanbanBoard, std::string> parse(std::string md_string) {
		const std::string_view md_string_view(md_string);
		std::size_t offset = 0;

		KanbanBoard kanban_board;

		auto first_header_pos = md_string.find(constants::first_header);
		if (first_header_pos == std::string::npos) {
			return tl::make_unexpected("Invalid file");
		}
		offset += first_header_pos;

		auto first_header_eol_pos = md_string_view.substr(offset + 1).find("\n");
		if (first_header_eol_pos == std::string::npos) {
			return tl::make_unexpected("Invalid file");
		}

		std::string name_text = md_string.substr(first_header_pos + constants::first_header.size(), first_header_eol_pos);
		kanban_board.name = name_text;
		offset += constants::first_header.size() + first_header_eol_pos;

		auto second_header_pos = md_string_view.substr(offset + 1).find(constants::second_header);
		if (second_header_pos == std::string::npos) {
			return tl::make_unexpected("Invalid file");
		}

		std::string description_text = md_string.substr(offset, second_header_pos);
		kanban_board.description = description_text;
		offset += constants::second_header.size() + second_header_pos;

		bool found_labels = false;
		bool found_board = false;

		while (!found_board || !found_labels)
		{
			auto second_header_eol_pos = md_string_view.substr(offset + 1).find("\n");
			if (second_header_eol_pos == std::string::npos) {
				return tl::make_unexpected("Invalid file");
			}

			std::string secondary_header_text = md_string.substr(offset + 1, second_header_eol_pos);
			//std::cout << "s[" << secondary_header_text << "]" << "\n";
			offset += secondary_header_text.size();

			if (secondary_header_text == "Labels:") {
				found_labels = true;

				auto li_pos = md_string_view.substr(offset).find(constants::ul_list_item);
				if (li_pos == std::string::npos) {
					return tl::make_unexpected("Invalid file");
				}
				offset += li_pos;

				auto li_eol_pos = md_string_view.substr(offset + 1).find("\n");
				if (li_eol_pos == std::string::npos) {
					return tl::make_unexpected("Invalid file");
				}

				std::string label_name_html = md_string.substr(offset + 1, li_eol_pos);

				re2::RE2 word_pattern("<span id=\"kanban_md-label-.+\">(.*)<\/span>");
				std::string label_name;

				if (!RE2::PartialMatch(trim(label_name_html), word_pattern, &label_name)) {
					return tl::make_unexpected("Invalid file");
				}

				KanbanLabel label;
				label.name = trim(label_name);
				offset += li_eol_pos;

				KanbanTask task;
				std::string task_item = md_string.substr(offset + 1, li_eol_pos);
				re2::RE2 word_pattern_1(R"(- \[(.*)\])");
				std::string task_name;

				if (!RE2::PartialMatch(trim(task_item), word_pattern_1, &task_name)) {
					return tl::make_unexpected("Invalid file..");
				}

				task.name = task_name;
				offset += task_item.size();
				std::cout << "[" << task.name << "]" << "\n";

				auto li_pos_2 = md_string_view.substr(offset + 1).find(constants::ul_list_item);
				if (li_pos_2 == std::string::npos) {
					return tl::make_unexpected("Invalid file");
				}
				offset += li_pos_2;

				auto li_eol_pos_2 = md_string_view.substr(offset + 1).find("\n");
				if (li_eol_pos_2 == std::string::npos) {
					return tl::make_unexpected("Invalid file");
				}

				KanbanTask task_2;
				std::string task_item_2 = md_string.substr(offset + 1, li_eol_pos_2);
				std::string task_name_2;
				if (!RE2::PartialMatch(trim(task_item_2), word_pattern_1, &task_name_2)) {
					return tl::make_unexpected("Invalid file..");
				}

				task_2.name = task_name_2;
				offset += task_item_2.size();
				std::cout << "[" << task_2.name << "]" << "\n";

			}
			break;
		}


		/*auto second_horizonal_rule = md_string.substr(first_header_pos + 1).find(horizonal_line_break);
		if (second_horizonal_rule == std::string::npos)
		{
			return tl::make_unexpected("Invalid file");
		}

		std::string top_section = md_string.substr(first_header_pos, second_horizonal_rule);
		const std::string a = "Title: ";
		const std::string b = "Description: ";
		auto name = top_section.find(a);
		auto description = top_section.find(b);

		std::string name_text = top_section.substr(name + a.size(), description - (b.size() - 1));
		std::string description_text = top_section.substr(description + b.size(), second_horizonal_rule - (horizonal_line_break.size() - 1));

		kanban_board.name = name_text;
		kanban_board.description = description_text;*/



		return kanban_board;
	}
}