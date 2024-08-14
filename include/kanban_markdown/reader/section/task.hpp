#pragma once

#include <pugixml.hpp>

#include <kanban_markdown/reader/internal.hpp>

namespace kanban_markdown::reader::section::task {
	using namespace kanban_markdown::reader::internal;

	namespace internal {
		void parse_task_property(KanbanReader* kanban_reader, const std::string& text_content) {
			bool is_task_property = false;
			switch (kanban_markdown::internal::hash(text_content)) {
			case kanban_markdown::internal::hash("Description"):
				kanban_reader->content_section.task_read_state = TaskReadState::Description;
				is_task_property = true;
				break;
			case kanban_markdown::internal::hash("Labels"):
				kanban_reader->content_section.task_read_state = TaskReadState::Labels;
				is_task_property = true;
				break;
			case kanban_markdown::internal::hash("Attachments"):
				kanban_reader->content_section.task_read_state = TaskReadState::Attachments;
				is_task_property = true;
				break;
			case kanban_markdown::internal::hash("Checklist"):
				kanban_reader->content_section.task_read_state = TaskReadState::Checklist;
				is_task_property = true;
				break;
			}
			if (!is_task_property && kanban_reader->content_section.task_read_state == TaskReadState::Description) {
				ListSection* current_list = kanban_reader->content_section.current_list;
				if (current_list == nullptr) {
					std::cerr << "Current board is null." << '\n';
					return;
				}
				CurrentTask& current_task = current_list->current_task;
				const std::string current_task_index = fmt::format("{}-{}", current_task.name, current_task.counter);
				TaskDetail& current_task_detail = current_list->task_details[current_task_index];
				if (current_task_detail.description.empty()) {
					std::string description = text_content;
					description = kanban_markdown::internal::ltrim(description);
					if (description.find(":") == 0) // TODO: Fix this
					{
						description = description.substr(1);
					}
					description = kanban_markdown::internal::trim(description);
					if (!description.empty()) {
						current_task_detail.description.push_back(description);
					}
				}
				else {
					std::string description = text_content;
					description = kanban_markdown::internal::trim(description);
					if (!description.empty()) {
						current_task_detail.description.push_back(description);
					}
				}
			}
		}

		void parse_task_sub_section(KanbanReader* kanban_reader, const std::string& text_content) {
			switch (kanban_reader->content_section.task_read_state) {
			case TaskReadState::Labels:
			{
				ListSection* current_list = kanban_reader->content_section.current_list;
				if (current_list == nullptr) {
					std::cerr << "Current board is null." << '\n';
					return;
				}
				CurrentTask& current_task = current_list->current_task;
				const std::string current_task_index = fmt::format("{}-{}", current_task.name, current_task.counter);
				current_list->task_details[current_task_index].labels.push_back(text_content);
				break;
			}
			case TaskReadState::Attachments:
			{
				if (kanban_reader->currently_reading_link) {
					ListSection* current_list = kanban_reader->content_section.current_list;
					if (current_list == nullptr) {
						std::cerr << "Current board is null." << '\n';
						return;
					}
					KanbanAttachment* current_attachment = current_list->current_attachment;
					current_attachment->name = text_content;
				}
				break;
			}
			case TaskReadState::Checklist:
			{
				bool is_checked = false;
				std::string checkbox_characters = text_content.substr(0, 4);
				if (checkbox_characters == constants::checklist_item_unchecked) {
					is_checked = false;
				}
				else if (checkbox_characters == constants::checklist_item_checked) {
					is_checked = true;
				}
				else {
					std::cerr << "Invalid checklist item: " << text_content << '\n';
				}
				KanbanChecklistItem checkbox;
				checkbox.checked = is_checked;
				checkbox.name = text_content.substr(4);
				ListSection* current_list = kanban_reader->content_section.current_list;
				if (current_list == nullptr) {
					std::cerr << "Current board is null." << '\n';
					return;
				}
				CurrentTask& current_task = current_list->current_task;
				const std::string current_task_index = fmt::format("{}-{}", current_task.name, current_task.counter);
				current_list->task_details[current_task_index].checklist.push_back(checkbox);
				break;
			}
			}
		}
	}

	static inline void read_text(KanbanReader* kanban_reader, const std::string& text_content) {
		if (text_content == constants::checklist_item_unchecked || text_content == constants::checklist_item_checked) {
			bool is_checked = false;
			std::string checkbox_characters = text_content.substr(0, 4);
			if (checkbox_characters == constants::checklist_item_unchecked) {
				is_checked = false;
			}
			else if (checkbox_characters == constants::checklist_item_checked) {
				is_checked = true;
			}
			else {
				std::cerr << "Invalid checklist item: " << text_content << '\n';
			}
			ListSection* current_list = kanban_reader->content_section.current_list;
			if (current_list == nullptr) {
				std::cerr << "Current board is null." << '\n';
				return;
			}
			current_list->current_stored_checked = is_checked;
			return;
		}
		switch (kanban_reader->list_item_level) {
		case 0: // It is not in a list
		{
			// Check if it is not a main header or a description
			if (kanban_reader->header_level <= 2) {
				return;
			}
			ListSection list_section;
			list_section.counter = utils::kanban_get_counter_with_name(text_content, kanban_reader->content_section.list_name_tracker_map);
			list_section.name = text_content;
			kanban_reader->content_section.lists.push_back(list_section);
			kanban_reader->content_section.current_list = &kanban_reader->content_section.lists.back();
			kanban_reader->content_section.task_read_state = TaskReadState::None;
			kanban_reader->content_section.content_read_state = ContentReadState::List;
			break;
		}
		case 1: // It is in the first level of a list
		{
			ListSection* current_list = kanban_reader->content_section.current_list;
			if (current_list == nullptr) {
				std::cerr << "Current board is null." << '\n';
				return;
			}
			const std::string name = text_content;
			const unsigned int counter = utils::kanban_get_counter_with_name(name, kanban_reader->content_section.task_name_tracker_map);

			current_list->current_task.counter = counter;
			current_list->current_task.name = name;

			kanban_reader->content_section.content_read_state = ContentReadState::Task;

			TaskDetail task_detail;
			task_detail.counter = counter;
			task_detail.name = name;
			task_detail.checked = current_list->current_stored_checked;

			current_list->task_details[fmt::format("{}-{}", name, counter)] = task_detail;
			break;
		}
		case 2: // It is in the second level of a list
		{
			internal::parse_task_property(kanban_reader, text_content);
			break;
		}
		case 3: // It is in the third level of a list
		{
			internal::parse_task_sub_section(kanban_reader, text_content);
			break;
		}
		}
	}

	static inline void read_html(KanbanReader* kanban_reader, const pugi::xml_document& xml) {
		switch (kanban_reader->content_section.content_read_state) {
		case ContentReadState::List:
		{
			ListSection* current_list = kanban_reader->content_section.current_list;
			// If a counter already exists, replace the counter with the new counter
			unsigned int counter = std::atoll(xml.child("span").attribute("data-counter").value());
			if (counter > 0) {
				if (current_list->counter != counter) {
					kanban_reader->content_section.list_name_tracker_map[current_list->name].used_hash.erase(current_list->counter);
				}
				current_list->counter = counter;
				kanban_reader->content_section.list_name_tracker_map[current_list->name].used_hash.insert(counter);
			}
			current_list->checked = kanban_markdown::internal::to_bool(xml.child("span").attribute("data-checked").value());
			break;
		}
		case ContentReadState::Task:
		{
			CurrentTask& current_task = kanban_reader->content_section.current_list->current_task;
			const std::string current_task_index = fmt::format("{}-{}", current_task.name, current_task.counter);

			TaskDetail current_task_detail_cloned = kanban_reader->content_section.current_list->task_details[current_task_index];
			// If a counter already exists, replace the counter with the new counter
			unsigned int counter = std::atoll(xml.child("span").attribute("data-counter").value());
			if (counter > 0) {
				if (current_task_detail_cloned.counter != counter) {
					kanban_reader->content_section.task_name_tracker_map[current_task_detail_cloned.name].used_hash.erase(current_task_detail_cloned.counter);
				}
				current_task.counter = counter;
				current_task_detail_cloned.counter = counter;

				kanban_reader->content_section.task_name_tracker_map[current_task_detail_cloned.name].used_hash.insert(counter);

				kanban_reader->content_section.current_list->task_details.erase(current_task_index);
				kanban_reader->content_section.current_list->task_details[fmt::format("{}-{}", current_task.name, current_task.counter)] = current_task_detail_cloned;
			}
			break;
		}
		default:
			break;
		}
	}
}