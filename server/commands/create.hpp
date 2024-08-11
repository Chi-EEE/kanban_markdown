#pragma once

#include "../internal.hpp"
#include "../kanban_path_visitor.hpp"

#include <fmt/format.h>
#include <re2/re2.h>

namespace server::commands
{
	class CreateCommandVisitor : public KanbanPathVisitor {
	public:
		CreateCommandVisitor(kanban_markdown::KanbanBoard* kanban_board, std::string path, void* userdata) : KanbanPathVisitor(kanban_board, path, userdata) {}

	private:
		void visitBoard() final {
			throw std::runtime_error("Invalid path: KanbanBoard is a object and cannot be used for creation.");
		}

		void editBoardName() final {
			throw std::runtime_error("Invalid path: KanbanBoard.name is a key and cannot be used for creation.");
		}

		void editBoardDescription() final {
			throw std::runtime_error("Invalid path: KanbanBoard.description is a key and cannot be used for creation.");
		}

		void editBoardColor() final {
			throw std::runtime_error("Invalid path: KanbanBoard.color is a key and cannot be used for creation.");
		}

		void editBoardList() final {
			yyjson_val* value = (yyjson_val*)userdata;

			yyjson_val* name = yyjson_obj_get(value, "name");
			if (name == NULL)
			{
				throw std::runtime_error("Unable to find name");
			}
			yyjson_val* checked = yyjson_obj_get(value, "checked");
			if (checked == NULL)
			{
				throw std::runtime_error("Unable to find checked");
			}
			std::string name_str = yyjson_get_string_object(name);
			re2::RE2::GlobalReplace(&name_str, constants::vertical_whitespace_regex_pattern, "");

			std::shared_ptr<kanban_markdown::KanbanList> kanban_list = std::make_shared<kanban_markdown::KanbanList>();
			kanban_list->counter = kanban_markdown::utils::kanban_get_counter_with_name(name_str, this->kanban_board->list_name_tracker_map);
			kanban_list->name = name_str;
			kanban_list->checked = yyjson_get_bool(checked);
			this->kanban_board->list.push_back(kanban_list);
		}

		void editBoardLabels() final {
			yyjson_val* value = (yyjson_val*)userdata;

			yyjson_val* name = yyjson_obj_get(value, "name");
			if (name == NULL)
			{
				throw std::runtime_error("Unable to find name");
			}
			yyjson_val* color = yyjson_obj_get(value, "color");
			if (color == NULL)
			{
				throw std::runtime_error("Unable to find color");
			}
			std::string name_str = yyjson_get_string_object(name);
			re2::RE2::GlobalReplace(&name_str, constants::vertical_whitespace_regex_pattern, "");

			std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label = std::make_shared<kanban_markdown::KanbanLabel>();
			kanban_label->color = yyjson_get_string_object(color);
			kanban_label->name = name_str;
			this->kanban_board->labels.push_back(kanban_label);
		}

		void visitList(std::vector<std::shared_ptr<kanban_markdown::KanbanList>>::iterator kanban_list_iterator) final {
			throw std::runtime_error("Invalid path: KanbanList is a object and cannot be used for creation.");
		}

		void editListName(std::shared_ptr<kanban_markdown::KanbanList> kanban_list) final {
			throw std::runtime_error("Invalid path: KanbanList.name is a key and cannot be used for creation.");
		}
		void editListChecked(std::shared_ptr<kanban_markdown::KanbanList> kanban_list) final {
			throw std::runtime_error("Invalid path: KanbanList.checked is a key and cannot be used for creation.");
		}
		void editListTasks(std::shared_ptr<kanban_markdown::KanbanList> kanban_list) final {
			yyjson_val* value = (yyjson_val*)userdata;

			yyjson_val* name = yyjson_obj_get(value, "name");
			if (name == NULL)
			{
				throw std::runtime_error("Unable to find name");
			}

			yyjson_val* description = yyjson_obj_get(value, "description");
			if (description == NULL)
			{
				throw std::runtime_error("Unable to find description");
			}

			yyjson_val* labels = yyjson_obj_get(value, "labels");
			if (labels == NULL)
			{
				throw std::runtime_error("Unable to find labels");
			}

			yyjson_val* attachments = yyjson_obj_get(value, "attachments");
			if (attachments == NULL)
			{
				throw std::runtime_error("Unable to find attachments");
			}

			yyjson_val* checklist = yyjson_obj_get(value, "checklist");
			if (checklist == NULL)
			{
				throw std::runtime_error("Unable to find checklist");
			}
			std::string name_str = yyjson_get_string_object(name);
			re2::RE2::GlobalReplace(&name_str, constants::vertical_whitespace_regex_pattern, "");

			std::shared_ptr<kanban_markdown::KanbanTask> kanban_task = std::make_shared<kanban_markdown::KanbanTask>();
			kanban_task->counter = kanban_markdown::utils::kanban_get_counter_with_name(name_str, this->kanban_board->task_name_tracker_map);
			kanban_task->name = name_str;
			const std::string description_str = yyjson_get_string_object(description);
			if (!description_str.empty())
				kanban_task->description = split(description_str, "\n");

			size_t idx, max;
			yyjson_val* label;
			yyjson_arr_foreach(labels, idx, max, label)
			{
				std::string label_name = yyjson_get_string_object(yyjson_obj_get(label, "name"));
				re2::RE2::GlobalReplace(&label_name, constants::vertical_whitespace_regex_pattern, "");
				auto it = std::find_if(this->kanban_board->labels.begin(), this->kanban_board->labels.end(), [&label_name](const auto& x)
					{ return x->name == label_name; });
				std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label;
				if (it == this->kanban_board->labels.end())
				{
					kanban_label = std::make_shared<kanban_markdown::KanbanLabel>();
					kanban_label->name = label_name;
					this->kanban_board->labels.push_back(kanban_label);
				}
				else {
					kanban_label = *it;
				}
				kanban_task->labels.push_back(kanban_label);
				kanban_label->tasks.push_back(kanban_task);
			}

			yyjson_val* attachment;
			yyjson_arr_foreach(attachments, idx, max, attachment)
			{
				std::string attachment_name = yyjson_get_string_object(yyjson_obj_get(attachment, "name"));
				re2::RE2::GlobalReplace(&attachment_name, constants::vertical_whitespace_regex_pattern, "");
				std::string attachment_url = yyjson_get_string_object(yyjson_obj_get(attachment, "url"));
				auto kanban_attachment = std::make_shared<kanban_markdown::KanbanAttachment>();
				kanban_attachment->name = attachment_name;
				kanban_attachment->url = attachment_url;
				kanban_task->attachments.push_back(kanban_attachment);
			}

			yyjson_val* checklist_item;
			yyjson_arr_foreach(checklist, idx, max, checklist_item)
			{
				std::string checklist_item_name = yyjson_get_string_object(yyjson_obj_get(checklist_item, "name"));
				re2::RE2::GlobalReplace(&checklist_item_name, constants::vertical_whitespace_regex_pattern, "");
				bool checklist_item_checked = yyjson_get_bool(yyjson_obj_get(checklist_item, "checked"));
				auto kanban_checklist_item = std::make_shared<kanban_markdown::KanbanChecklistItem>();
				kanban_checklist_item->name = checklist_item_name;
				kanban_checklist_item->checked = checklist_item_checked;
				kanban_task->checklist.push_back(kanban_checklist_item);
			}

			kanban_list->tasks.push_back(kanban_task);
		}

		void visitTask(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::vector<std::shared_ptr<kanban_markdown::KanbanTask>>::iterator kanban_task_iterator) final {
			throw std::runtime_error("Invalid path: KanbanTask is a object and cannot be used for creation.");
		}

		void editTaskName(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) final {
			throw std::runtime_error("Invalid path: KanbanTask.name is a key and cannot be used for creation.");
		}
		void editTaskDescription(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) final {
			throw std::runtime_error("Invalid path: KanbanTask.description is a key and cannot be used for creation.");
		}
		void editTaskChecked(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) final {
			throw std::runtime_error("Invalid path: KanbanTask.checked is a key and cannot be used for creation.");
		}
		void editTaskLabels(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) final {
			yyjson_val* value = (yyjson_val*)userdata;

			yyjson_val* name = yyjson_obj_get(value, "name");
			if (name == NULL)
			{
				throw std::runtime_error("Unable to find name");
			}
			std::string name_str = yyjson_get_string_object(name);
			re2::RE2::GlobalReplace(&name_str, constants::vertical_whitespace_regex_pattern, "");

			auto kanban_label_it = std::find_if(kanban_task->labels.begin(), kanban_task->labels.end(), [&name_str](const std::shared_ptr<kanban_markdown::KanbanLabel>& x)
				{ return x->name == name_str; });

			if (kanban_label_it == kanban_task->labels.end())
			{
				throw std::runtime_error("Unable to create another KanbanLabel, it already exists inside of the task.");
			}

			auto it = std::find_if(this->kanban_board->labels.begin(), this->kanban_board->labels.end(), [&name_str](const std::shared_ptr<kanban_markdown::KanbanLabel>& x)
				{ return x->name == name_str; });

			std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label;
			if (it == this->kanban_board->labels.end())
			{
				kanban_label = std::make_shared<kanban_markdown::KanbanLabel>();
				kanban_label->name = name_str;
				this->kanban_board->labels.push_back(kanban_label);
			}
			else {
				kanban_label = *it;
			}
			kanban_task->labels.push_back(kanban_label);
			kanban_label->tasks.push_back(kanban_task);
		}
		void editTaskAttachments(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) final {
			yyjson_val* value = (yyjson_val*)userdata;

			yyjson_val* name = yyjson_obj_get(value, "name");
			if (name == NULL)
			{
				throw std::runtime_error("Unable to find name");
			}
			yyjson_val* url = yyjson_obj_get(value, "url");
			if (url == NULL)
			{
				throw std::runtime_error("Unable to find url");
			}
			std::string attachment_name = yyjson_get_string_object(name);
			re2::RE2::GlobalReplace(&attachment_name, constants::vertical_whitespace_regex_pattern, "");

			std::string attachment_url = yyjson_get_string_object(url);
			auto kanban_attachment = std::make_shared<kanban_markdown::KanbanAttachment>();
			kanban_attachment->name = attachment_name;
			kanban_attachment->url = attachment_url;
			kanban_task->attachments.push_back(kanban_attachment);
		}
		void editTaskChecklist(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) final {
			yyjson_val* value = (yyjson_val*)this->userdata;

			yyjson_val* name = yyjson_obj_get(value, "name");
			if (name == NULL)
			{
				throw std::runtime_error("Unable to find name");
			}
			yyjson_val* checked = yyjson_obj_get(value, "checked");
			if (checked == NULL)
			{
				throw std::runtime_error("Unable to find checked");
			}
			std::string checklist_item_name = yyjson_get_string_object(name);
			re2::RE2::GlobalReplace(&checklist_item_name, constants::vertical_whitespace_regex_pattern, "");

			bool checklist_item_checked = yyjson_get_bool(checked);
			auto kanban_checklist_item = std::make_shared<kanban_markdown::KanbanChecklistItem>();
			kanban_checklist_item->name = checklist_item_name;
			kanban_checklist_item->checked = checklist_item_checked;
			kanban_task->checklist.push_back(kanban_checklist_item);
		}

		void visitTaskLabel(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::vector<std::shared_ptr<kanban_markdown::KanbanLabel>>::iterator kanban_label_iterator) final {
			throw std::runtime_error("Invalid path: KanbanLabel is a object and cannot be used for creation.");
		}

		void editTaskLabelName(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label) final {
			throw std::runtime_error("Invalid path: KanbanLabel.name is a key and cannot be used for creation.");
		}

		void editTaskLabelColor(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label) final {
			throw std::runtime_error("Invalid path: KanbanLabel.color is a key and cannot be used for creation.");
		}

		void visitTaskAttachment(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::vector<std::shared_ptr<kanban_markdown::KanbanAttachment>>::iterator kanban_attachment_iterator) final {
			throw std::runtime_error("Invalid path: KanbanAttachment is a object and cannot be used for creation.");
		}

		void editTaskAttachmentName(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanAttachment> kanban_attachment) final {
			throw std::runtime_error("Invalid path: KanbanAttachment.name is a key and cannot be used for creation.");
		}
		void editTaskAttachmentUrl(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanAttachment> kanban_attachment) final {
			throw std::runtime_error("Invalid path: KanbanAttachment.url is a key and cannot be used for creation.");
		}

		void visitTaskChecklist(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::vector<std::shared_ptr<kanban_markdown::KanbanChecklistItem>>::iterator kanban_checklist_item_iterator) final {
			throw std::runtime_error("Invalid path: KanbanChecklistItem is a object and cannot be used for creation.");
		}

		void editTaskChecklistName(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanChecklistItem> kanban_checklist_item) final {
			throw std::runtime_error("Invalid path: KanbanChecklistItem.name is a key and cannot be used for creation.");
		}
		void editTaskChecklistChecked(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanChecklistItem> kanban_checklist_item) final {
			throw std::runtime_error("Invalid path: KanbanChecklistItem.checked is a key and cannot be used for creation.");
		}

		void visitLabel(std::vector<std::shared_ptr<kanban_markdown::KanbanLabel>>::iterator kanban_label_iterator) final {
			throw std::runtime_error("Invalid path: KanbanLabel is a object and cannot be used for creation.");
		}

		void editLabelName(std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label) final {
			throw std::runtime_error("Invalid path: KanbanLabel.name is a key and cannot be used for creation.");
		}

		void editLabelColor(std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label) final {
			throw std::runtime_error("Invalid path: KanbanLabel.color is a key and cannot be used for creation.");
		}
	};

	void command_create(KanbanTuple& kanban_tuple, yyjson_val* command)
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

		CreateCommandVisitor visitor(&kanban_tuple.kanban_board, path_str, value);
		visitor.run();
	}
}
