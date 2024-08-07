#pragma once

#include "../internal.hpp"
#include "../kanban_path_visitor.hpp"

#include <fmt/format.h>
#include <re2/re2.h>

namespace server::commands
{
	class UpdateCommandVisitor : public KanbanPathVisitor {
	public:
		UpdateCommandVisitor(kanban_markdown::KanbanBoard* kanban_board, std::string path, void* userdata) : KanbanPathVisitor(kanban_board, path, userdata) {}

	private:
		void visitBoard() final {
			throw std::runtime_error("Invalid path: KanbanBoard is a object and cannot be modified.");
		}

		void editBoardName() final {
			this->kanban_board->name = yyjson_get_string_object((yyjson_val*)userdata);
		}

		void editBoardDescription() final {
			this->kanban_board->description = yyjson_get_string_object((yyjson_val*)userdata);
		}

		void editBoardColor() final {
			this->kanban_board->color = yyjson_get_string_object((yyjson_val*)userdata);
		}

		void editBoardList() final {
			throw std::runtime_error("Invalid path: KanbanBoard.list is a key and cannot be modified through the update command.");
		}

		void editBoardLabels() final {
			throw std::runtime_error("Invalid path: KanbanBoard.labels is a key and cannot be modified through the update command.");
		}

		void visitList(std::vector<std::shared_ptr<kanban_markdown::KanbanList>>::iterator kanban_list_iterator) final {
			throw std::runtime_error("Invalid path: KanbanList is a object and cannot be modified.");
		}

		void editListName(std::shared_ptr<kanban_markdown::KanbanList> kanban_list) final {
			kanban_list->name = yyjson_get_string_object((yyjson_val*)userdata);
		}
		void editListChecked(std::shared_ptr<kanban_markdown::KanbanList> kanban_list) final {
			kanban_list->checked = yyjson_get_bool((yyjson_val*)userdata);
		}
		void editListTasks(std::shared_ptr<kanban_markdown::KanbanList> kanban_list) final {
			throw std::runtime_error("Invalid path: KanbanList.tasks is a object and cannot be modified.");
		}

		void visitTask(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::vector<std::shared_ptr<kanban_markdown::KanbanTask>>::iterator kanban_task_iterator) final {
			throw std::runtime_error("Invalid path: KanbanTask is a object and cannot be modified.");
		}

		void editTaskName(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) final {
			kanban_task->name = yyjson_get_string_object((yyjson_val*)userdata);
		}
		void editTaskDescription(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) final {
			kanban_task->description = split(yyjson_get_string_object((yyjson_val*)userdata), "\n");
		}
		void editTaskChecked(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) final {
			kanban_task->checked = yyjson_get_bool((yyjson_val*)userdata);
		}
		void editTaskLabels(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) final {
			throw std::runtime_error("Invalid path: KanbanTask.labels is a object and cannot be modified.");
		}
		void editTaskAttachments(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) final {
			throw std::runtime_error("Invalid path: KanbanTask.attachments is a object and cannot be modified.");
		}
		void editTaskChecklist(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) final {
			throw std::runtime_error("Invalid path: KanbanTask.checklist is a object and cannot be modified.");
		}

		void visitTaskLabel(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::vector<std::shared_ptr<kanban_markdown::KanbanLabel>>::iterator kanban_label_iterator) final {
			throw std::runtime_error("Invalid path: KanbanLabel is a object and cannot be modified.");
		}

		void editTaskLabelName(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label) final {
			kanban_label->name = yyjson_get_string_object((yyjson_val*)userdata);
		}

		void editTaskLabelColor(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label) final {
			kanban_label->color = yyjson_get_string_object((yyjson_val*)userdata);
		}

		void visitTaskAttachment(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::vector<std::shared_ptr<kanban_markdown::KanbanAttachment>>::iterator kanban_attachment_iterator) final {
			throw std::runtime_error("Invalid path: KanbanAttachment is a object and cannot be modified.");
		}

		void editTaskAttachmentName(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanAttachment> kanban_attachment) final {
			kanban_attachment->name = yyjson_get_string_object((yyjson_val*)userdata);
		}
		void editTaskAttachmentUrl(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanAttachment> kanban_attachment) final {
			kanban_attachment->url = yyjson_get_string_object((yyjson_val*)userdata);
		}

		void visitTaskChecklist(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::vector<std::shared_ptr<kanban_markdown::KanbanChecklistItem>>::iterator kanban_checklist_item_iterator) final {
			throw std::runtime_error("Invalid path: KanbanChecklistItem is a object and cannot be modified.");
		}

		void editTaskChecklistName(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanChecklistItem> kanban_checklist_item) final {
			kanban_checklist_item->name = yyjson_get_string_object((yyjson_val*)userdata);
		}
		void editTaskChecklistChecked(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanChecklistItem> kanban_checklist_item) final {
			kanban_checklist_item->checked = yyjson_get_bool((yyjson_val*)userdata);
		}

		void visitLabel(std::vector<std::shared_ptr<kanban_markdown::KanbanLabel>>::iterator kanban_label_iterator) final {
			throw std::runtime_error("Invalid path: KanbanLabel is a object and cannot be modified.");
		}

		void editLabelName(std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label) final {
			kanban_label->name = yyjson_get_string_object((yyjson_val*)userdata);
		}

		void editLabelColor(std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label) final {
			kanban_label->color = yyjson_get_string_object((yyjson_val*)userdata);
		}
	};

	void command_update(KanbanTuple& kanban_tuple, yyjson_val* command)
	{
		yyjson_val* path = yyjson_obj_get(command, "path");
		if (path == NULL)
		{
			throw std::runtime_error("Error: Missing required 'path' field in command object");
		}
		yyjson_val* value = yyjson_obj_get(command, "value");
		if (value == NULL)
		{
			throw std::runtime_error("Error: Missing required 'value' field in command object");
		}
		std::string path_str = yyjson_get_string_object(path);

		UpdateCommandVisitor visitor(&kanban_tuple.kanban_board, path_str, (void*)value);
		visitor.run();
	}
}