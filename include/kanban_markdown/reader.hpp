#pragma once

#include <iostream>
#include <vector>
#include <fmt/format.h>
#include <tl/expected.hpp>

#include "kanban.hpp"

namespace kanban_markdown {
	tl::expected<KanbanBoard, std::string> parse(std::string md_string) {
		KanbanBoard kanban_board;
		const std::string horizonal_line_break = "---";
		auto first_horizonal_rule = md_string.find(horizonal_line_break);
		if (first_horizonal_rule != 0) {
			return tl::make_unexpected("Invalid file");
		}

		auto second_horizonal_rule = md_string.substr(first_horizonal_rule + 1).find(horizonal_line_break);
		if (second_horizonal_rule == std::string::npos)
		{
			return tl::make_unexpected("Invalid file");
		}

		std::string top_section = md_string.substr(first_horizonal_rule, second_horizonal_rule);
		const std::string a = "Title: ";
		const std::string b = "Description: ";
		auto title = top_section.find(a);
		auto description = top_section.find(b);

		std::string title_text = top_section.substr(title + a.size(), description - (b.size() - 1));
		std::string description_text = top_section.substr(description + b.size(), second_horizonal_rule - (horizonal_line_break.size() - 1));

		kanban_board.title = title_text;
		kanban_board.description = description_text;



		return kanban_board;
	}
}