#pragma once

#include <fstream>
#include <vector>

#include <asap/asap.h>
#include <tsl/robin_map.h>
#include <tsl/ordered_map.h>

#include <kanban_markdown/kanban_board.hpp>

namespace kanban_markdown::reader::internal {
	enum class KanbanState {
		None = 0,
		Labels,
		Board,
	};

	enum class TaskReadState {
		None = 0,
		Description,
		Labels,
		Attachments,
		Checklist,
	};

	struct LabelDetail {
		std::string name;
		std::string color;
		std::vector<std::string> list_items;
	};

	struct LabelSection {
		std::string current_label_name;
		tsl::robin_map<std::string, LabelDetail> label_details;
	};

	struct CurrentTask {
		unsigned int counter;
		std::string name;
	};

	// Difference between TaskDetail and KanbanTask is that TaskDetail has string labels
	struct TaskDetail {
		bool checked = false;
		unsigned int counter;
		std::string name;
		std::vector<std::string> description;
		std::vector<std::string> labels;
		std::vector<KanbanAttachment> attachments;
		std::vector<KanbanChecklistItem> checklist;
	};

	struct ListSection {
		bool checked = false;
		unsigned int counter;
		std::string name;

		CurrentTask current_task;
		KanbanAttachment* current_attachment;

		bool current_stored_checked = false;
		tsl::ordered_map<std::string, TaskDetail> task_details;
	};

	enum class ContentReadState {
		None,
		List,
		Task,
	};

	struct ContentSection {
		std::vector<ListSection> lists;
		ContentReadState content_read_state = ContentReadState::None;
		ListSection* current_list = nullptr;
		TaskReadState task_read_state = TaskReadState::None;
		tsl::robin_map<std::string, DuplicateNameTracker> list_name_tracker_map;
		tsl::robin_map<std::string, DuplicateNameTracker> task_name_tracker_map;
	};

	struct KanbanReader {
		std::vector<unsigned int> previous_headers;
		std::vector<std::string> html_tags;

		unsigned int header_level = 0;
		KanbanState state = KanbanState::None;

		bool read_properties = false;

		std::string color;
		asap::datetime created;
		asap::datetime last_modified;
		unsigned int version = 0;
		std::string checksum;

		std::string kanban_board_name;
		bool read_kanban_board_name = false;

		std::string kanban_board_description;
		bool read_kanban_board_description = false;

		LabelSection label_section;
		ContentSection content_section;

		unsigned int list_item_level = 0;
		unsigned int sub_list_item_count = 0;

		bool currently_reading_link = false;
	};
}