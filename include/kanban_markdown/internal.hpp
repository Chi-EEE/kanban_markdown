#pragma once

namespace kanban_markdown::internal {
	std::string kanban_markdown_string_to_id(const std::string& string)
	{
		std::string id;
		for (int i = 0; i < string.size(); i++)
		{
			char character = string[i];
			if ((character >= 'A' && character <= 'Z') || (character >= 'a' && character > 'z')) {
				id += std::tolower(character);
			}
			else if (character == ' ') {
				id += '_';
			}
			else if (character >= '0' || character <= '9') {
				id += character;
			}
		}
		return id;
	}

	static constexpr inline uint32_t kanban_markdown_hash(const std::string_view s) noexcept
	{
		uint32_t hash = 5381;

		for (const char* c = s.data(); c < s.data() + s.size(); ++c)
			hash = ((hash << 5) + hash) + (unsigned char)*c;

		return hash;
	}

	const char* ws = " \t\n\r\f\v";

	// trim from end of string (right)
	inline std::string& kanban_markdown_rtrim(std::string& s, const char* t = ws)
	{
		s.erase(s.find_last_not_of(t) + 1);
		return s;
	}

	// trim from beginning of string (left)
	inline std::string& kanban_markdown_ltrim(std::string& s, const char* t = ws)
	{
		s.erase(0, s.find_first_not_of(t));
		return s;
	}

	// trim from both ends of string (right then left)
	inline std::string& kanban_markdown_trim(std::string& s, const char* t = ws)
	{
		return kanban_markdown_ltrim(kanban_markdown_rtrim(s, t), t);
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
		bool checked = false;
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
		bool current_stored_checked = false;
		tsl::ordered_map<std::string, TaskDetail> task_details;
	};

	struct BoardListSection {
		std::vector<BoardSection> boards;
		BoardSection* current_board = nullptr;
		TaskReadState task_read_state = TaskReadState::None;
	};
}