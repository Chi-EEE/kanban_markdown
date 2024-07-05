#pragma once

#include <iostream>
#include <vector>
#include <fmt/format.h>
#include <tl/expected.hpp>

#include "kanban.hpp"

namespace kanban_markdown {
	tl::expected<KanbanBoard, std::string> parse(std::string md_string) {
		KanbanBoard kanban_board;
		const std::string first_header = "# ";
		const std::string second_header = "## ";

		auto first_header_pos = md_string.find(first_header);
		if (first_header_pos == std::string::npos) {
			return tl::make_unexpected("Invalid file");
		}
		auto first_header_eol_pos = md_string.substr(first_header_pos + 1).find("\n");
		if (first_header_eol_pos == std::string::npos) {
			return tl::make_unexpected("Invalid file");
		}
		std::string title_text = md_string.substr(first_header_pos + first_header.size(), first_header_eol_pos);
		//std::cout << "Title: " << title_text << "\n";

		std::string x = md_string.substr(first_header_pos + first_header.size() + first_header_eol_pos);
		auto second_header_pos = x.find(second_header);
		//std::cout << second_header_pos << "\n";
		auto second_header_eol_pos = md_string.substr(second_header_pos + 1).find("\n");
		//std::cout << second_header_eol_pos << "\n";
		if (second_header_eol_pos == std::string::npos) {
			return tl::make_unexpected("Invalid file");
		}
		std::string description_text = md_string.substr(second_header_pos - second_header.size(), second_header_eol_pos);
		//std::cout << "Description: " << description_text << "\n";

		
		/*auto second_horizonal_rule = md_string.substr(first_header_pos + 1).find(horizonal_line_break);
		if (second_horizonal_rule == std::string::npos)
		{
			return tl::make_unexpected("Invalid file");
		}

		std::string top_section = md_string.substr(first_header_pos, second_horizonal_rule);
		const std::string a = "Title: ";
		const std::string b = "Description: ";
		auto title = top_section.find(a);
		auto description = top_section.find(b);

		std::string title_text = top_section.substr(title + a.size(), description - (b.size() - 1));
		std::string description_text = top_section.substr(description + b.size(), second_horizonal_rule - (horizonal_line_break.size() - 1));

		kanban_board.title = title_text;
		kanban_board.description = description_text;*/



		return kanban_board;
	}
}