#pragma once

#include <kanban_markdown/constants.hpp>
#include <kanban_markdown/reader/internal.hpp>

namespace kanban_markdown::reader::section::none {
	using namespace kanban_markdown::reader::internal;

	static inline void read_text(KanbanReader* kanban_reader, const std::string& text_content) {
		switch (kanban_reader->header_level) {
		case 0:
		{
			if (!kanban_reader->read_kanban_board_description && kanban_reader->read_kanban_board_name && kanban_reader->kanban_board_description.empty())
			{
				if (text_content == constants::default_description)
					kanban_reader->kanban_board_description = "";
				else
					kanban_reader->kanban_board_description = text_content;
				kanban_reader->read_kanban_board_description = true;
			}
			break;
		}
		case 1:
		{
			if (!kanban_reader->read_kanban_board_name)
			{
				if (text_content == constants::default_description)
					kanban_reader->kanban_board_name = "";
				else
					kanban_reader->kanban_board_name = text_content;
				kanban_reader->read_kanban_board_name = true;
			}
			break;
		}
		case 2:
		{
			if (text_content == "Labels:")
				kanban_reader->state = KanbanState::Labels;
			else if (text_content == "Board:")
				kanban_reader->state = KanbanState::Board;
			break;
		}
		default:
		{
			break;
		}
		}
	}
}