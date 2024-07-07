#pragma once

#include <string>
#include <vector>
#include <memory>
#include <tsl/ordered_map.h>

namespace kanban_markdown {
	struct KanbanBoard;
	struct KanbanLabel;
	struct KanbanList;
	struct KanbanTask;
	struct KanbanAttachment;
	struct KanbanChecklistItem;

	struct KanbanList
	{
		std::string name;
		tsl::ordered_map<std::string, std::shared_ptr<KanbanTask>> tasks;
	};

	struct KanbanBoard
	{
		std::string name;
		std::string description;
		tsl::ordered_map<std::string, std::shared_ptr<KanbanLabel>> labels;
		tsl::ordered_map<std::string, KanbanList> list;
	};

	struct KanbanLabel
	{
		std::string name;
		std::vector<std::shared_ptr<KanbanTask>> tasks;
	};

	struct KanbanTask
	{
		bool checked = false;
		std::string name;
		std::vector<std::string> description;
		std::vector<std::shared_ptr<KanbanLabel>> labels;
		std::vector<KanbanAttachment> attachments;
		std::vector<KanbanChecklistItem> checklist;
	};

	struct KanbanAttachment
	{
		std::string name;
		std::string url;
	};

	struct KanbanChecklistItem
	{
		bool checked = false;
		std::string name;
	};
}