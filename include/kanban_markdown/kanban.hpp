#pragma once

#include <string>
#include <vector>
#include <memory>

namespace kanban_markdown {
	struct KanbanBoard;
	struct KanbanLabel;
	struct KanbanList;
	struct KanbanTask;
	struct KanbanAttachment;
	struct KanbanChecklistItem;

	struct KanbanBoard
	{
		std::string name;
		std::string description;
		std::vector<KanbanLabel> labels;
		std::vector<KanbanList> list;
	};

	struct KanbanLabel
	{
		std::string name;
		std::vector<std::shared_ptr<KanbanTask>> tasks;
	};

	struct KanbanList
	{
		std::string name;
		std::vector<KanbanTask> tasks;
	};

	struct KanbanTask
	{
		bool checked = false;
		std::string name;
		std::string description;
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