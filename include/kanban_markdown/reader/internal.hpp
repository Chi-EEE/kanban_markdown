#pragma once

#include <tsl/ordered_map.h>
#include <vector>

#include "../kanban.hpp"

namespace kanban_markdown::internal {
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
		std::vector<std::string> list_items;
	};

	struct LabelSection {
		std::string current_label_name;
		tsl::ordered_map<std::string, LabelDetail> label_details;
	};

	struct TaskDetail {
		bool checked = false;
		std::string name;
		std::vector<std::string> description;
		std::vector<std::string> labels;
		std::vector<KanbanAttachment> attachments;
		std::vector<KanbanChecklistItem> checklist;

		KanbanAttachment* currentAttachment;
	};

	struct BoardSection {
		std::string name;
		std::string current_task_name;
		bool current_stored_checked = false;
		tsl::ordered_map<std::string, TaskDetail> task_details;
	};

	struct BoardListSection {
		std::vector<BoardSection> boards;
		BoardSection* current_board = nullptr;
		TaskReadState task_read_state = TaskReadState::None;
	};
}