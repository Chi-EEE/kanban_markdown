#pragma once

namespace kanban_markdown::internal {
	static constexpr uint32_t hash(const std::string_view s) noexcept
	{
		uint32_t hash = 5381;

		for (const char* c = s.data(); c < s.data() + s.size(); ++c)
			hash = ((hash << 5) + hash) + (unsigned char)*c;

		return hash;
	}

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

	struct Attachment {
		std::string name;
		std::string url;
	};

	struct TaskDetail {
		std::string name;
		std::vector<std::string> description;
		std::vector<std::string> labels;
		std::vector<Attachment> attachments;
		std::vector<KanbanChecklistItem> checklist;

		Attachment* currentAttachment;
	};

	struct BoardSection {
		std::string name;
		std::string current_task_name;
		tsl::ordered_map<std::string, TaskDetail> task_details;
	};

	struct BoardListSection {
		std::vector<BoardSection> boards;
		BoardSection* current_board = nullptr;
		TaskReadState task_read_state = TaskReadState::None;
	};
}