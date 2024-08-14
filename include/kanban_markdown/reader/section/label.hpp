#pragma once

#include <pugixml.hpp>

#include <kanban_markdown/reader/internal.hpp>

namespace kanban_markdown::reader::section::label {
	using namespace kanban_markdown::reader::internal;

	static inline void read_text(KanbanReader* kanban_reader, const std::string& text_content) {
		switch (kanban_reader->list_item_level) {
		case 1:
		{
			LabelDetail label_detail;
			label_detail.name = text_content;
			kanban_reader->label_section.label_details[text_content] = label_detail;
			kanban_reader->label_section.current_label_name = text_content;
			break;
		}
		case 2:
		{
			kanban_reader->label_section.label_details[kanban_reader->label_section.current_label_name].list_items.push_back(text_content);
			break;
		}
		}
	}

	static inline void read_html(KanbanReader* kanban_reader, const pugi::xml_document& xml) {
		kanban_reader->label_section.label_details[kanban_reader->label_section.current_label_name].color = xml.child("span").attribute("data-color").value();
	}
}