#pragma once

#include "../internal.hpp"

#include <fmt/format.h>
#include <re2/re2.h>

namespace server::commands
{
	namespace internal_create
	{
		void parsePath_1(KanbanTuple &kanban_tuple, std::vector<std::string> &split_result, yyjson_val *value);

		void parsePath_1_list(KanbanTuple &kanban_tuple, std::vector<std::string> &split_result, yyjson_val *value, std::string vector_index_name);
		void parsePath_2_tasks(KanbanTuple &kanban_tuple, std::vector<std::string> &split_result, yyjson_val *value, std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::string list_vector_index_name);

		void parsePath_1(KanbanTuple &kanban_tuple, std::vector<std::string> &split_result, yyjson_val *value)
		{
			std::string first = split_result[0];

			switch (hash(first))
			{
			case hash("labels"):
			{
				yyjson_val *name = yyjson_obj_get(value, "name");
				if (name == NULL)
				{
					throw std::runtime_error("Unable to find name");
				}
				std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label = std::make_shared<kanban_markdown::KanbanLabel>();
				kanban_label->name = yyjson_get_string_object(name);
				kanban_tuple.kanban_board.labels.insert({kanban_label->name, kanban_label});
				break;
			}
			case hash("list"):
			{
				yyjson_val *name = yyjson_obj_get(value, "name");
				if (name == NULL)
				{
					throw std::runtime_error("Unable to find name");
				}
				std::shared_ptr<kanban_markdown::KanbanList> kanban_list = std::make_shared<kanban_markdown::KanbanList>();
				kanban_list->name = yyjson_get_string_object(name);
				kanban_tuple.kanban_board.list.insert({kanban_list->name, kanban_list});
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

		void parsePath_1_list(KanbanTuple &kanban_tuple, std::vector<std::string> &split_result, yyjson_val *value, std::string vector_index_name)
		{
			if (!kanban_tuple.kanban_board.list.contains(vector_index_name))
			{
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanBoard.list named "")", vector_index_name));
			}
			std::shared_ptr<kanban_markdown::KanbanList> kanban_list = kanban_tuple.kanban_board.list[vector_index_name];
			std::string second = split_result[1];

			switch (hash(second))
			{
			case hash("tasks"):
			{
				yyjson_val *name = yyjson_obj_get(value, "name");
				if (name == NULL)
				{
					throw std::runtime_error("Unable to find name");
				}

				yyjson_val *description = yyjson_obj_get(value, "description");
				if (description == NULL)
				{
					throw std::runtime_error("Unable to find description");
				}

				yyjson_val *labels = yyjson_obj_get(value, "labels");
				if (labels == NULL)
				{
					throw std::runtime_error("Unable to find labels");
				}

				yyjson_val *attachments = yyjson_obj_get(value, "attachments");
				if (attachments == NULL)
				{
					throw std::runtime_error("Unable to find attachments");
				}

				yyjson_val *checklist = yyjson_obj_get(value, "checklist");
				if (checklist == NULL)
				{
					throw std::runtime_error("Unable to find checklist");
				}

				std::shared_ptr<kanban_markdown::KanbanTask> kanban_task = std::make_shared<kanban_markdown::KanbanTask>();
				kanban_task->name = yyjson_get_string_object(name);
				kanban_task->description = split(yyjson_get_string_object(description), "\n");

				size_t idx, max;
				yyjson_val *label;
				yyjson_arr_foreach(labels, idx, max, label)
				{
					std::string label_name = yyjson_get_string_object(yyjson_obj_get(label, "name"));
					if (!kanban_tuple.kanban_board.labels.contains(label_name))
					{
						std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label = std::make_shared<kanban_markdown::KanbanLabel>();
						kanban_label->name = label_name;
						kanban_tuple.kanban_board.labels.insert({label_name, kanban_label});
					}
					std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label = kanban_tuple.kanban_board.labels[label_name];
					kanban_task->labels[label_name] = kanban_label;
				}

				yyjson_val *attachment;
				yyjson_arr_foreach(attachments, idx, max, attachment)
				{
					std::string attachment_name = yyjson_get_string_object(yyjson_obj_get(attachment, "name"));
					std::string attachment_url = yyjson_get_string_object(yyjson_obj_get(attachment, "url"));
					auto kanban_attachment = std::make_shared<kanban_markdown::KanbanAttachment>();
					kanban_attachment->name = attachment_name;
					kanban_attachment->url = attachment_url;
					kanban_task->attachments[attachment_name] = kanban_attachment;
				}

				yyjson_val *checklist_item;
				yyjson_arr_foreach(checklist, idx, max, checklist_item)
				{
					std::string checklist_item_name = yyjson_get_string_object(yyjson_obj_get(checklist_item, "name"));
					bool checklist_item_checked = yyjson_get_bool(yyjson_obj_get(checklist_item, "checked"));
					auto kanban_checklist_item = std::make_shared<kanban_markdown::KanbanChecklistItem>();
					kanban_checklist_item->name = checklist_item_name;
					kanban_checklist_item->checked = checklist_item_checked;
					kanban_task->checklist[checklist_item_name] = kanban_checklist_item;
				}

				kanban_list->tasks.insert({kanban_task->name, kanban_task});
				break;
			}
			default:
			{
				static re2::RE2 path_pattern(R"((\w+)\[(.+)\])");
				std::string list_vector_name;
				std::string list_vector_index_name;
				if (!RE2::PartialMatch(second, path_pattern, &list_vector_name, &list_vector_index_name))
				{
					throw std::runtime_error("Invalid path: The first field must be a vector name with an index inside of KanbanList");
				}

				switch (hash(list_vector_name))
				{
				case hash("tasks"):
				{
					parsePath_2_tasks(kanban_tuple, split_result, value, kanban_list, list_vector_index_name);
					break;
				}
				default:
					throw std::runtime_error("Invalid path: There are no fields inside KanbanList with " + second);
				}
				break;
			}
			}
		}

		void parsePath_2_tasks(KanbanTuple &kanban_tuple, std::vector<std::string> &split_result, yyjson_val *value, std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::string list_vector_index_name)
		{
			if (!kanban_list->tasks.contains(list_vector_index_name))
			{
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanList.tasks named "{}")", list_vector_index_name));
			}
			std::shared_ptr<kanban_markdown::KanbanTask> task = kanban_list->tasks[list_vector_index_name];
			std::string third = split_result[2];
			switch (hash(third))
			{
			case hash("labels"):
			{
				yyjson_val *name = yyjson_obj_get(value, "name");
				if (name == NULL)
				{
					throw std::runtime_error("Unable to find name");
				}
				std::string label_name = yyjson_get_string_object(name);
				if (!kanban_tuple.kanban_board.labels.contains(label_name))
				{
					std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label = std::make_shared<kanban_markdown::KanbanLabel>();
					kanban_label->name = label_name;
					kanban_tuple.kanban_board.labels.insert({label_name, kanban_label});
				}
				std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label = kanban_tuple.kanban_board.labels[label_name];
				task->labels[label_name] = kanban_label;
				break;
			}
			case hash("attachments"):
			{
				yyjson_val *name = yyjson_obj_get(value, "name");
				if (name == NULL)
				{
					throw std::runtime_error("Unable to find name");
				}
				yyjson_val *url = yyjson_obj_get(value, "url");
				if (url == NULL)
				{
					throw std::runtime_error("Unable to find url");
				}
				std::string attachment_name = yyjson_get_string_object(name);
				std::string attachment_url = yyjson_get_string_object(url);
				auto kanban_attachment = std::make_shared<kanban_markdown::KanbanAttachment>();
				kanban_attachment->name = attachment_name;
				kanban_attachment->url = attachment_url;
				task->attachments[attachment_name] = kanban_attachment;
				break;
			}
			case hash("checklist"):
			{
				yyjson_val *name = yyjson_obj_get(value, "name");
				if (name == NULL)
				{
					throw std::runtime_error("Unable to find name");
				}
				yyjson_val *checked = yyjson_obj_get(value, "checked");
				if (checked == NULL)
				{
					throw std::runtime_error("Unable to find checked");
				}
				std::string checklist_item_name = yyjson_get_string_object(name);
				bool checklist_item_checked = yyjson_get_bool(checked);
				auto kanban_checklist_item = std::make_shared<kanban_markdown::KanbanChecklistItem>();
				kanban_checklist_item->name = checklist_item_name;
				kanban_checklist_item->checked = checklist_item_checked;
				task->checklist[checklist_item_name] = kanban_checklist_item;
				break;
			}
			default:
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no vector fields inside KanbanTask named "{}")", third));
			}
		}
	}
	void command_create(KanbanTuple &kanban_tuple, yyjson_val *command)
	{
		yyjson_val *path = yyjson_obj_get(command, "path");
		if (path == NULL)
		{
			throw std::runtime_error("Unable to find path");
		}
		yyjson_val *value = yyjson_obj_get(command, "value");
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

		if (split_result.size() >= 2)
		{
			internal_create::parsePath_1(kanban_tuple, split_result, value);
		}
	}
}
