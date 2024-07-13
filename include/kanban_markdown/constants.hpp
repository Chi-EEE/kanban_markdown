#pragma once

#include <string>

namespace kanban_markdown::constants {
	const std::string END_OF_MARKDOWN_LINE = "  \n";
	const std::string tile = "Tile: ";
	const std::string first_header = "# ";
	const std::string second_header = "## ";
	const std::string third_header = "### ";
	const std::string ul_list_item = "- ";
	const std::string github_added_tag = "user-content-";

	const std::string kanban_md = "kanban_md";

	const std::string default_color = "#A0A0A0";
	const std::string default_board_name = "Untitled Board";
	const std::string default_description = "No description available.";
}