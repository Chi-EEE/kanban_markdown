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
		std::string title = "dsa";
		std::string description = "dsa";
		std::vector<KanbanLabel> labels;
		std::vector<KanbanList> list;
	};

	struct KanbanLabel
	{
		std::string title;
		std::vector<std::shared_ptr<KanbanTask>> tasks;
	};

	struct KanbanList
	{
		std::string title = "TODO:";
		std::vector<KanbanTask> tasks;
	};

	struct KanbanTask
	{
		bool checked = false;
		std::string title;
		std::string description;
		std::vector<std::shared_ptr<KanbanLabel>> labels;
		std::vector<KanbanAttachment> attachments;
		std::vector<KanbanChecklistItem> checklist;
	};

	struct KanbanAttachment
	{
		std::string title;
		std::string url;
	};

	struct KanbanChecklistItem
	{
		bool checked = false;
		std::string title;
	};
}