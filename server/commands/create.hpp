#pragma once

#include "../internal.hpp"

#include <fmt/format.h>
#include <re2/re2.h>

namespace server::commands
{
	namespace internal_create
	{
		void parsePath_1(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, yyjson_val* value);

		void parsePath_1_list(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, yyjson_val* value, std::string vector_index_name);
		void parsePath_2_tasks(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, yyjson_val* value, std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::string list_vector_index_name, unsigned int list_vector_index_counter);

		void parsePath_1(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, yyjson_val* value)
		{
			std::string first = split_result[0];

			switch (hash(first))
			{
			case hash("name"):
				throw std::runtime_error("Invalid path: The first field must be a vector name");
			case hash("description"):
				throw std::runtime_error("Invalid path: The first field must be a vector name");
			case hash("labels"):
			{
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
				std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label = std::make_shared<kanban_markdown::KanbanLabel>();
				kanban_label->color = yyjson_get_string_object(color);
				kanban_label->name = yyjson_get_string_object(name);
				kanban_tuple.kanban_board.labels.push_back(kanban_label);
				break;
			}
			case hash("list"):
			{
				yyjson_val* name = yyjson_obj_get(value, "name");
				if (name == NULL)
				{
					throw std::runtime_error("Unable to find name");
				}
				std::shared_ptr<kanban_markdown::KanbanList> kanban_list = std::make_shared<kanban_markdown::KanbanList>();
				kanban_list->name = yyjson_get_string_object(name);
				kanban_tuple.kanban_board.list.push_back(kanban_list);
				break;
			}
			default:
			{
				static re2::RE2 path_pattern(R"((\w+)\[(.+)\])");
				std::string vector_name;
				std::string vector_index_name;

				if (!RE2::PartialMatch(first, path_pattern, &vector_name, &vector_index_name))
				{
					throw std::runtime_error("Invalid path: The first field must be a vector name with an index");
				}

				switch (hash(vector_name))
				{

				case hash("list"):
				{
					parsePath_1_list(kanban_tuple, split_result, value, vector_index_name);
					break;
				}
				default:
					throw std::runtime_error(fmt::format(R"(Invalid path: There are no vector fields inside KanbanBoard named "{}")", first));
				}
				break;
			}
			}
		}

		void parsePath_1_list(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, yyjson_val* value, std::string vector_index_name)
		{
			auto it = std::find_if(kanban_tuple.kanban_board.list.begin(), kanban_tuple.kanban_board.list.end(), [&vector_index_name](const auto& x)
				{ return x->name == vector_index_name; });
			if (it == kanban_tuple.kanban_board.list.end())
			{
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanBoard.list named "")", vector_index_name));
			}
			std::shared_ptr<kanban_markdown::KanbanList> kanban_list = *it;
			std::string second = split_result[1];

			switch (hash(second))
			{
			case hash("tasks"):
			{
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

				std::shared_ptr<kanban_markdown::KanbanTask> kanban_task = std::make_shared<kanban_markdown::KanbanTask>();
				std::string name_str = yyjson_get_string_object(name);

				kanban_task->counter = kanban_markdown::utils::kanban_get_counter_with_name(name_str, kanban_tuple.kanban_board.task_name_tracker_map);
				kanban_task->name = name_str;
				kanban_task->description = split(yyjson_get_string_object(description), "\n");

				size_t idx, max;
				yyjson_val* label;
				yyjson_arr_foreach(labels, idx, max, label)
				{
					std::string label_name = yyjson_get_string_object(yyjson_obj_get(label, "name"));
					auto it = std::find_if(kanban_tuple.kanban_board.labels.begin(), kanban_tuple.kanban_board.labels.end(), [&label_name](const auto& x)
						{ return x->name == label_name; });
					std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label;
					if (it == kanban_tuple.kanban_board.labels.end())
					{
						kanban_label = std::make_shared<kanban_markdown::KanbanLabel>();
						kanban_label->name = label_name;
						kanban_tuple.kanban_board.labels.push_back(kanban_label);
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
					bool checklist_item_checked = yyjson_get_bool(yyjson_obj_get(checklist_item, "checked"));
					auto kanban_checklist_item = std::make_shared<kanban_markdown::KanbanChecklistItem>();
					kanban_checklist_item->name = checklist_item_name;
					kanban_checklist_item->checked = checklist_item_checked;
					kanban_task->checklist.push_back(kanban_checklist_item);
				}

				kanban_list->tasks.push_back(kanban_task);
				break;
			}
			default:
			{
				static re2::RE2 path_pattern(R"((\w+)\[(.+)\]\[(.+)\])");
				std::string list_vector_name;
				std::string list_vector_index_name;
				std::string list_vector_index_counter_str;
				if (!RE2::PartialMatch(second, path_pattern, &list_vector_name, &list_vector_index_name, &list_vector_index_counter_str))
				{
					throw std::runtime_error("Invalid path: The first field must be a vector name with an index inside of KanbanList");
				}

				unsigned int list_vector_index_counter = std::stoi(list_vector_index_counter_str);

				switch (hash(list_vector_name))
				{
				case hash("tasks"):
				{
					parsePath_2_tasks(kanban_tuple, split_result, value, kanban_list, list_vector_index_name, list_vector_index_counter);
					break;
				}
				default:
					throw std::runtime_error("Invalid path: There are no fields inside KanbanList with " + second);
				}
				break;
			}
			}
		}

		void parsePath_2_tasks(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, yyjson_val* value, std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::string list_vector_index_name, unsigned int list_vector_index_counter)
		{
			auto it = std::find_if(kanban_list->tasks.begin(), kanban_list->tasks.end(), [&list_vector_index_name, &list_vector_index_counter](const std::shared_ptr<kanban_markdown::KanbanTask>& x)
				{ return x->name == list_vector_index_name && x->counter == list_vector_index_counter; });
			if (it == kanban_list->tasks.end())
			{
				throw std::runtime_error(
					fmt::format(R"(Invalid path: There are no keys inside KanbanList.tasks named "{}" and with counter "{}")",
						list_vector_index_name,
						list_vector_index_counter
					));
			}
			std::shared_ptr<kanban_markdown::KanbanTask> task = *it;
			std::string third = split_result[2];
			switch (hash(third))
			{
			case hash("labels"):
			{
				yyjson_val* name = yyjson_obj_get(value, "name");
				if (name == NULL)
				{
					throw std::runtime_error("Unable to find name");
				}
				std::string label_name = yyjson_get_string_object(name);
				auto it = std::find_if(kanban_tuple.kanban_board.labels.begin(), kanban_tuple.kanban_board.labels.end(), [&label_name](const auto& x)
					{ return x->name == label_name; });
				std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label;
				if (it == kanban_tuple.kanban_board.labels.end())
				{
					kanban_label = std::make_shared<kanban_markdown::KanbanLabel>();
					kanban_label->name = label_name;
					kanban_tuple.kanban_board.labels.push_back(kanban_label);
				}
				else {
					kanban_label = *it;
				}
				task->labels.push_back(kanban_label);
				kanban_label->tasks.push_back(task);
				break;
			}
			case hash("attachments"):
			{
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
				std::string attachment_url = yyjson_get_string_object(url);
				auto kanban_attachment = std::make_shared<kanban_markdown::KanbanAttachment>();
				kanban_attachment->name = attachment_name;
				kanban_attachment->url = attachment_url;
				task->attachments.push_back(kanban_attachment);
				break;
			}
			case hash("checklist"):
			{
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
				bool checklist_item_checked = yyjson_get_bool(checked);
				auto kanban_checklist_item = std::make_shared<kanban_markdown::KanbanChecklistItem>();
				kanban_checklist_item->name = checklist_item_name;
				kanban_checklist_item->checked = checklist_item_checked;
				task->checklist.push_back(kanban_checklist_item);
				break;
			}
			default:
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no vector fields inside KanbanTask named "{}")", third));
			}
		}
	}
	void command_create(KanbanTuple& kanban_tuple, yyjson_val* command)
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
		std::string path_str = yyjson_get_string_object(path);
		std::vector<std::string> split_result = split(path_str, ".");

		if (split_result.empty())
		{
			throw std::runtime_error("Invalid path: There are no fields");
		}

		internal_create::parsePath_1(kanban_tuple, split_result, value);
	}
}
