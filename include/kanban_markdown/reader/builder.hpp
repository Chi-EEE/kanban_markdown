#pragma once

#include <kanban_markdown/kanban_board.hpp>
#include <kanban_markdown/reader/internal.hpp>

namespace kanban_markdown::reader::builder {
	using namespace kanban_markdown::reader::internal;

	inline KanbanBoard create(KanbanReader& kanban_reader) {
		KanbanBoard kanban_board;
		kanban_board.color = kanban_reader.color;
		kanban_board.created = kanban_reader.created;
		kanban_board.last_modified = kanban_reader.last_modified;
		kanban_board.version = kanban_reader.version;
		kanban_board.checksum = kanban_reader.checksum;

		kanban_board.name = kanban_reader.kanban_board_name;
		kanban_board.description = kanban_reader.kanban_board_description;

		kanban_board.list_name_tracker_map = kanban_reader.content_section.list_name_tracker_map;
		kanban_board.task_name_tracker_map = kanban_reader.content_section.task_name_tracker_map;

		for (auto& [_, label_detail] : kanban_reader.label_section.label_details) {
			std::shared_ptr<KanbanLabel> kanban_label = std::make_shared<KanbanLabel>();
			kanban_label->name = label_detail.name;
			kanban_label->color = label_detail.color;
			kanban_board.labels.push_back(kanban_label);
		}

		for (ListSection list_section : kanban_reader.content_section.lists) {
			std::shared_ptr<KanbanList> kanban_list = std::make_shared<KanbanList>();
			kanban_list->checked = list_section.checked;
			kanban_list->counter = list_section.counter;
			kanban_list->name = list_section.name;
			for (auto& [_, task_detail] : list_section.task_details) {
				std::shared_ptr<KanbanTask> kanban_task = std::make_shared<KanbanTask>();
				kanban_task->checked = task_detail.checked;
				kanban_task->counter = task_detail.counter;
				kanban_task->name = task_detail.name;
				kanban_task->description = task_detail.description;
				for (const std::string& label : task_detail.labels) {
					auto it = std::find_if(kanban_board.labels.begin(), kanban_board.labels.end(), [&label](const std::shared_ptr<KanbanLabel>& x) { return x->name == label; });
					std::shared_ptr<KanbanLabel> kanban_label;
					if (it == kanban_board.labels.end()) {
						kanban_label = std::make_shared<KanbanLabel>();
						kanban_label->name = label;
						kanban_board.labels.push_back(kanban_label);
					}
					else {
						kanban_label = *it;
					}
					kanban_label->tasks.push_back(kanban_task);
					kanban_task->labels.push_back(kanban_label);
				}
				for (const KanbanAttachment& attachment : task_detail.attachments) {
					kanban_task->attachments.push_back(std::make_shared<KanbanAttachment>(attachment));
				}
				for (const KanbanChecklistItem& checkbox : task_detail.checklist) {
					kanban_task->checklist.push_back(std::make_shared<KanbanChecklistItem>(checkbox));
				}
				kanban_list->tasks.push_back(kanban_task);
			}
			kanban_board.list.push_back(kanban_list);
		}
		return kanban_board;
	}
}