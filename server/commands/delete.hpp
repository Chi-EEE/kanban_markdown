#pragma once

#include "../internal.hpp"
#include "../kanban_path_visitor.hpp"

#include <fmt/format.h>
#include <re2/re2.h>

namespace server::commands
{
	class DeleteCommandVisitor : public KanbanPathVisitor {
	public:
		DeleteCommandVisitor(kanban_markdown::KanbanBoard* kanban_board, std::string path, void* userdata) : KanbanPathVisitor(kanban_board, path, userdata) {}

	private:
		void editBoardName() final {
			throw std::runtime_error("Invalid path: KanbanBoard.name is a key and cannot be deleted");
		}

		void editBoardDescription() final {
			throw std::runtime_error("Invalid path: KanbanBoard.description is a key and cannot be deleted");
		}

		void editBoardColor() final {
			throw std::runtime_error("Invalid path: KanbanBoard.color is a key and cannot be deleted");
		}

		void visitList(std::vector<std::shared_ptr<kanban_markdown::KanbanList>>::iterator kanban_list_iterator) final {
			std::shared_ptr<kanban_markdown::KanbanList> kanban_list = *kanban_list_iterator;
			for (auto& kanban_task : kanban_list->tasks) {
				for (auto& kanban_label : kanban_task->labels) {
					kanban_label->tasks.erase(std::remove_if(kanban_label->tasks.begin(), kanban_label->tasks.end(), [&kanban_task](const std::shared_ptr<kanban_markdown::KanbanTask>& x)
						{ return *x == *kanban_task; }), kanban_label->tasks.end());
				}
			}
			this->kanban_board->list.erase(kanban_list_iterator);
		}

		void editListName(std::shared_ptr<kanban_markdown::KanbanList> kanban_list) final {
			throw std::runtime_error("Invalid path: KanbanList.name is a key and cannot be deleted");
		}
		void editListChecked(std::shared_ptr<kanban_markdown::KanbanList> kanban_list) final {
			throw std::runtime_error("Invalid path: KanbanList.checked is a key and cannot be deleted");
		}

		void visitTask(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::vector<std::shared_ptr<kanban_markdown::KanbanTask>>::iterator kanban_task_iterator) final {
			std::shared_ptr<kanban_markdown::KanbanTask> kanban_task = *kanban_task_iterator;
			for (auto& kanban_label : kanban_task->labels) {
				kanban_label->tasks.erase(std::remove_if(kanban_label->tasks.begin(), kanban_label->tasks.end(), [&kanban_task](const std::shared_ptr<kanban_markdown::KanbanTask>& x)
					{ return *x == *kanban_task; }), kanban_label->tasks.end());
			}
			auto& task_name_tracker = this->kanban_board->task_name_tracker_map[kanban_task->name];
			task_name_tracker.used_hash.erase(kanban_task->counter);
			if (task_name_tracker.counter == kanban_task->counter)
			{
				task_name_tracker.counter--;
			}
			else if (task_name_tracker.counter > kanban_task->counter)
			{
				task_name_tracker.counter = kanban_task->counter - 1;
			}
			kanban_list->tasks.erase(kanban_task_iterator);
		}

		void editTaskName(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) final {
			throw std::runtime_error("Invalid path: KanbanTask.name is a key and cannot be deleted");
		}
		void editTaskDescription(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) final {
			throw std::runtime_error("Invalid path: KanbanTask.description is a key and cannot be deleted");
		}
		void editTaskChecked(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) final {
			throw std::runtime_error("Invalid path: KanbanTask.checked is a key and cannot be deleted");
		}
		void editTaskLabels(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) final {
			throw std::runtime_error("Invalid path: KanbanTask.labels is a key and cannot be deleted");
		}
		void editTaskAttachments(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) final {
			throw std::runtime_error("Invalid path: KanbanTask.attachments is a key and cannot be deleted");
		}
		void editTaskChecklist(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) final {
			throw std::runtime_error("Invalid path: KanbanTask.checklist is a key and cannot be deleted");
		}

		void visitTaskLabel(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::vector<std::shared_ptr<kanban_markdown::KanbanLabel>>::iterator kanban_label_iterator) final {
			std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label = *kanban_label_iterator;
			kanban_label->tasks.erase(std::remove_if(kanban_label->tasks.begin(), kanban_label->tasks.end(), [&kanban_task](const std::shared_ptr<kanban_markdown::KanbanTask>& x)
				{ return *x == *kanban_task; }), kanban_label->tasks.end());
		}

		void editTaskLabelName(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label) final {
			throw std::runtime_error("Invalid path: KanbanLabel.name is a key and cannot be deleted");
		}
		void editTaskLabelColor(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label) final {
			throw std::runtime_error("Invalid path: KanbanLabel.color is a key and cannot be deleted");
		}

		void visitTaskAttachment(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::vector<std::shared_ptr<kanban_markdown::KanbanAttachment>>::iterator kanban_attachment_iterator) final {
			kanban_task->attachments.erase(kanban_attachment_iterator);
		}

		void editTaskAttachmentName(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanAttachment> kanban_attachment) final {
			throw std::runtime_error("Invalid path: KanbanAttachment.name is a key and cannot be deleted");
		}
		void editTaskAttachmentUrl(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanAttachment> kanban_attachment) final {
			throw std::runtime_error("Invalid path: KanbanAttachment.url is a key and cannot be deleted");
		}

		void visitTaskChecklist(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::vector<std::shared_ptr<kanban_markdown::KanbanChecklistItem>>::iterator kanban_checklist_item_iterator) final {
			kanban_task->checklist.erase(kanban_checklist_item_iterator);
		}

		void editTaskChecklistName(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanChecklistItem> kanban_checklist_item) final {
			throw std::runtime_error("Invalid path: KanbanChecklistItem.name is a key and cannot be deleted");
		}
		void editTaskChecklistChecked(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanChecklistItem> kanban_checklist_item) final {
			throw std::runtime_error("Invalid path: KanbanChecklistItem.checked is a key and cannot be deleted");
		}

		void visitLabel(std::vector<std::shared_ptr<kanban_markdown::KanbanLabel>>::iterator kanban_label_iterator) final {
			std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label = *kanban_label_iterator;
			for (auto& kanban_task : kanban_label->tasks) {
				kanban_task->labels.erase(std::remove_if(kanban_task->labels.begin(), kanban_task->labels.end(), [&kanban_label](const std::shared_ptr<kanban_markdown::KanbanLabel>& x)
					{ return *x == *kanban_label; }), kanban_task->labels.end());
			}
			this->kanban_board->labels.erase(kanban_label_iterator);
		}

		void editLabelName(std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label) final {
			throw std::runtime_error("Invalid path: KanbanLabel.name is a key and cannot be deleted");
		}

		void editLabelColor(std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label) final {
			throw std::runtime_error("Invalid path: KanbanLabel.color is a key and cannot be deleted");
		}
	};

	void command_delete(KanbanTuple& kanban_tuple, yyjson_val* command)
	{
		yyjson_val* path = yyjson_obj_get(command, "path");
		if (path == NULL)
		{
			throw std::runtime_error("Unable to find path");
		}
		std::string path_str = yyjson_get_string_object(path);
		DeleteCommandVisitor visitor(&kanban_tuple.kanban_board, path_str, (void*)nullptr);
		visitor.visitKanbanBoard();
	}
}