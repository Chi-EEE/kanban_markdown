#pragma once

#include "../internal.hpp"
#include "../kanban_path_visitor.hpp"

#include <fmt/format.h>
#include <re2/re2.h>

namespace server::commands
{
	struct MoveValue {
		unsigned int index;
		std::string destination;
	};

	class MoveCommandVisitor : public KanbanPathVisitor {
	public:
		MoveCommandVisitor(kanban_markdown::KanbanBoard* kanban_board, std::string path, void* userdata) : KanbanPathVisitor(kanban_board, path, userdata) {}

	private:
		void visitBoard() final {
			throw std::runtime_error("Invalid path: KanbanBoard is a key object and cannot be moved");
		}

		void editBoardName() final {
			throw std::runtime_error("Invalid path: KanbanBoard.name is a key and cannot be moved");
		}

		void editBoardDescription() final {
			throw std::runtime_error("Invalid path: KanbanBoard.description is a key and cannot be moved");
		}

		void editBoardColor() final {
			throw std::runtime_error("Invalid path: KanbanBoard.color is a key and cannot be moved");
		}

		void editBoardList() final {
			throw std::runtime_error("Invalid path: KanbanBoard.list is a key and cannot be moved");
		}

		void editBoardLabels() final {
			throw std::runtime_error("Invalid path: KanbanBoard.labels is a key and cannot be moved");
		}

		void visitList(std::vector<std::shared_ptr<kanban_markdown::KanbanList>>::iterator kanban_list_iterator) final {
			const MoveValue* move_value = (MoveValue*)userdata;
			std::shared_ptr<kanban_markdown::KanbanList> kanban_list = *kanban_list_iterator;
			kanban_list_iterator = this->kanban_board->list.erase(kanban_list_iterator);
			this->kanban_board->list.insert(this->kanban_board->list.begin() + move_value->index, kanban_list);
		}

		void editListName(std::shared_ptr<kanban_markdown::KanbanList> kanban_list) final {
			throw std::runtime_error("Invalid path: KanbanList.name is a key and cannot be moved");
		}
		void editListChecked(std::shared_ptr<kanban_markdown::KanbanList> kanban_list) final {
			throw std::runtime_error("Invalid path: KanbanList.checked is a key and cannot be moved");
		}
		void editListTasks(std::shared_ptr<kanban_markdown::KanbanList> kanban_list) final {
			throw std::runtime_error("Invalid path: KanbanList.tasks is a key and cannot be moved");
		}

		void visitTask(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::vector<std::shared_ptr<kanban_markdown::KanbanTask>>::iterator kanban_task_iterator) final {
			const MoveValue* move_value = (MoveValue*)userdata;
			if (move_value->destination.empty()) {
				std::shared_ptr<kanban_markdown::KanbanTask> kanban_task = *kanban_task_iterator;
				kanban_task_iterator = kanban_list->tasks.erase(kanban_task_iterator);
				kanban_list->tasks.insert(kanban_list->tasks.begin() + move_value->index, kanban_task);
			}
			else {
				static re2::RE2 destination_pattern(R"(\w+\[\"(.+)\"\].tasks)");
				std::string destination_list;
				if (!RE2::PartialMatch(move_value->destination, destination_pattern, &destination_list))
				{
					throw std::runtime_error("Invalid path: The destination must be a list name");
				}

				auto parent_it = std::find_if(this->kanban_board->list.begin(), this->kanban_board->list.end(), [&destination_list](const auto& x)
					{ return x->name == destination_list; });

				if (parent_it == this->kanban_board->list.end())
				{
					throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanBoard.list named "{}")", destination_list));
				}
				std::shared_ptr<kanban_markdown::KanbanList> parent_list = *parent_it;
				int old_index = std::distance(kanban_list->tasks.begin(), kanban_task_iterator);

				std::shared_ptr<kanban_markdown::KanbanTask> task = kanban_list->tasks[old_index];
				task->checked = parent_list->checked;

				parent_list->tasks.insert(parent_list->tasks.begin() + move_value->index, task);
				kanban_list->tasks.erase(kanban_list->tasks.begin() + old_index);
			}
		}

		void editTaskName(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) final {
			throw std::runtime_error("Invalid path: KanbanTask.name is a key and cannot be moved");
		}
		void editTaskDescription(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) final {
			throw std::runtime_error("Invalid path: KanbanTask.description is a key and cannot be moved");
		}
		void editTaskChecked(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) final {
			throw std::runtime_error("Invalid path: KanbanTask.checked is a key and cannot be moved");
		}
		void editTaskLabels(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) final {
			throw std::runtime_error("Invalid path: KanbanTask.labels is a key and cannot be moved");
		}
		void editTaskAttachments(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) final {
			throw std::runtime_error("Invalid path: KanbanTask.attachments is a key and cannot be moved");
		}
		void editTaskChecklist(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) final {
			throw std::runtime_error("Invalid path: KanbanTask.checklist is a key and cannot be moved");
		}

		void visitTaskLabel(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::vector<std::shared_ptr<kanban_markdown::KanbanLabel>>::iterator kanban_label_iterator) final {
			throw std::runtime_error("Invalid path: Moving KanbanLabel is not supported");
		}

		void editTaskLabelName(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label) final {
			throw std::runtime_error("Invalid path: KanbanLabel.name is a key and cannot be moved");
		}

		void editTaskLabelColor(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label) final {
			throw std::runtime_error("Invalid path: KanbanLabel.url is a key and cannot be moved");
		}

		void visitTaskAttachment(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::vector<std::shared_ptr<kanban_markdown::KanbanAttachment>>::iterator kanban_attachment_iterator) final {
			throw std::runtime_error("Invalid path: Moving KanbanAttachment is currently not supported");
		}

		void editTaskAttachmentName(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanAttachment> kanban_attachment) final {
			throw std::runtime_error("Invalid path: KanbanAttachment.name is a key and cannot be moved");
		}
		void editTaskAttachmentUrl(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanAttachment> kanban_attachment) final {
			throw std::runtime_error("Invalid path: KanbanAttachment.url is a key and cannot be moved");
		}

		void visitTaskChecklist(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::vector<std::shared_ptr<kanban_markdown::KanbanChecklistItem>>::iterator kanban_checklist_item_iterator) final {
			throw std::runtime_error("Invalid path: Moving KanbanChecklistItem is currently not supported");
		}

		void editTaskChecklistName(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanChecklistItem> kanban_checklist_item) final {
			throw std::runtime_error("Invalid path: KanbanChecklistItem.name is a key and cannot be moved");
		}
		void editTaskChecklistChecked(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanChecklistItem> kanban_checklist_item) final {
			throw std::runtime_error("Invalid path: KanbanChecklistItem.checked is a key and cannot be moved");
		}

		void visitLabel(std::vector<std::shared_ptr<kanban_markdown::KanbanLabel>>::iterator kanban_label_iterator) final {
			throw std::runtime_error("Invalid path: Moving KanbanLabel is currently not supported");
		}

		void editLabelName(std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label) final {
			throw std::runtime_error("Invalid path: KanbanLabel.name is a key and cannot be moved");
		}

		void editLabelColor(std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label) final {
			throw std::runtime_error("Invalid path: KanbanLabel.color is a key and cannot be moved");
		}
	};

	void command_move(KanbanTuple& kanban_tuple, yyjson_val* command)
	{
		yyjson_val* path = yyjson_obj_get(command, "path");
		if (path == NULL)
		{
			throw std::runtime_error("Unable to find path");
		}
		yyjson_val* value = yyjson_obj_get(command, "value");
		if (value == NULL)
		{
			throw std::runtime_error("Unable to find value");
		}

		yyjson_val* index = yyjson_obj_get(value, "index");
		if (index == NULL)
		{
			throw std::runtime_error("Unable to find index");
		}

		yyjson_val* destination = yyjson_obj_get(value, "destination");
		std::string destination_str;
		if (destination != NULL)
		{
			destination_str = yyjson_get_string_object(destination);
		}

		unsigned int index_int = yyjson_get_uint(index);

		MoveValue move_value;
		move_value.destination = destination_str;
		move_value.index = index_int;

		std::string path_str = yyjson_get_string_object(path);

		MoveCommandVisitor visitor(&kanban_tuple.kanban_board, path_str, &move_value);
		visitor.run();
	}
}