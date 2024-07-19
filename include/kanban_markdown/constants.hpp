#pragma once

#include <string>

namespace kanban_markdown::constants {
	const std::string END_OF_MARKDOWN_LINE = "  \n";
	const std::string github_added_tag = "user-content-";

	const std::string kanban_md = "kanban_md";

	const std::string default_color = "#A0A0A0";
	const std::string default_board_name = "Untitled Board";
	const std::string default_description = "No description available.";

	const std::string checklist_item_checked = "[x] ";
	const std::string checklist_item_unchecked = "[ ] ";

	const std::string end_of_html_tag = "</";

	const std::string time_format = "%Y-%m-%d %H:%M:%S UTC";

}