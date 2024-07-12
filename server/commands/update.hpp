#pragma once

#include "../internal.hpp"

#include <fmt/format.h>
#include <re2/re2.h>

namespace server::commands
{
	namespace internal_update
	{
		void parsePath_1(KanbanTuple &kanban_tuple, std::vector<std::string> &split_result, yyjson_val *value);

		void parsePath_1_list(KanbanTuple &kanban_tuple, std::vector<std::string> &split_result, yyjson_val *value, std::string vector_index_name);
		void parsePath_2_tasks(KanbanTuple &kanban_tuple, std::vector<std::string> &split_result, yyjson_val *value, std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::string list_vector_index_name);
		void parsePath_3_labels(KanbanTuple &kanban_tuple, std::vector<std::string> &split_result, yyjson_val *value, std::shared_ptr<kanban_markdown::KanbanTask> task, std::string task_vector_index_name);
		void parsePath_3_attachments(KanbanTuple &kanban_tuple, std::vector<std::string> &split_result, yyjson_val *value, std::shared_ptr<kanban_markdown::KanbanTask> task, std::string task_vector_index_name);
		void parsePath_3_checklist(KanbanTuple &kanban_tuple, std::vector<std::string> &split_result, yyjson_val *value, std::shared_ptr<kanban_markdown::KanbanTask> task, std::string task_vector_index_name);

		void parsePath_1_labels(KanbanTuple &kanban_tuple, std::vector<std::string> &split_result, yyjson_val *value, std::string vector_index_name);

		void parsePath_1(KanbanTuple &kanban_tuple, std::vector<std::string> &split_result, yyjson_val *value)
		{
			std::string first = split_result[0];

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
			case hash("labels"):
			{
				parsePath_1_labels(kanban_tuple, split_result, value, vector_index_name);
				break;
			}
			default:
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanBoard named "{}")", first));
			}
		}

		void parsePath_1_list(KanbanTuple &kanban_tuple, std::vector<std::string> &split_result, yyjson_val *value, std::string vector_index_name)
		{
			if (!kanban_tuple.kanban_board.list.contains(vector_index_name))
			{
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanBoard.list named "{}")", vector_index_name));
			}
			std::shared_ptr<kanban_markdown::KanbanList> kanban_list = kanban_tuple.kanban_board.list[vector_index_name];
			std::string second = split_result[1];
			switch (hash(second))
			{
			case hash("name"):
			{
				kanban_list->name = yyjson_get_string_object(value);
				kanban_tuple.kanban_board.list[kanban_list->name] = kanban_list;
				kanban_tuple.kanban_board.list.erase(vector_index_name);
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
					throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanList named "{}")", second));
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
			case hash("name"):
			{
				task->name = yyjson_get_string_object(value);
				kanban_list->tasks[task->name] = task;
				kanban_list->tasks.erase(list_vector_index_name);
				break;
			}
			case hash("description"):
			{
				task->description = split(yyjson_get_string_object(value), "\n");
				break;
			}
			case hash("checked"):
			{
				task->checked = yyjson_get_bool(value);
				break;
			}
			default:
			{
				static re2::RE2 path_pattern(R"((\w+)\[(.+)\])");
				std::string task_vector_name;
				std::string task_vector_index_name;
				if (!RE2::PartialMatch(third, path_pattern, &task_vector_name, &task_vector_index_name))
				{
					throw std::runtime_error("Invalid path: The first field must be a vector name with an index inside of KanbanTask");
				}

				switch (hash(task_vector_name))
				{
				case hash("labels"):
				{
					parsePath_3_labels(kanban_tuple, split_result, value, task, task_vector_index_name);
					break;
				}
				case hash("attachments"):
				{
					parsePath_3_attachments(kanban_tuple, split_result, value, task, task_vector_index_name);
					break;
				}
				case hash("checklist"):
				{
					parsePath_3_checklist(kanban_tuple, split_result, value, task, task_vector_index_name);
					break;
				}
				default:
					throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanTask named "{}")", third));
				}

				break;
			}
			}
		}

		void parsePath_3_labels(KanbanTuple &kanban_tuple, std::vector<std::string> &split_result, yyjson_val *value, std::shared_ptr<kanban_markdown::KanbanTask> task, std::string task_vector_index_name)
		{
			if (!task->labels.contains(task_vector_index_name))
			{
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanTask.labels named "{}")", task_vector_index_name));
			}
			std::string fourth = split_result[3];
			switch (hash(fourth))
			{
			case hash("name"):
			{
				std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label = task->labels[task_vector_index_name];
				kanban_label->name = yyjson_get_string_object(value);
				for (auto &[task_name, task] : kanban_label->tasks)
				{
					task->labels[kanban_label->name] = kanban_label;
					task->labels.erase(task_vector_index_name);
				}
				kanban_tuple.kanban_board.labels[kanban_label->name] = kanban_label;
				kanban_tuple.kanban_board.labels.erase(task_vector_index_name);
				break;
			}
			default:
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanLabel named "{}")", fourth));
			}
		}

		void parsePath_3_attachments(KanbanTuple &kanban_tuple, std::vector<std::string> &split_result, yyjson_val *value, std::shared_ptr<kanban_markdown::KanbanTask> task, std::string task_vector_index_name)
		{
			if (!task->attachments.contains(task_vector_index_name))
			{
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanTask.attachments named "{}")", task_vector_index_name));
			}
			std::string fourth = split_result[3];
			switch (hash(fourth))
			{
			case hash("name"):
			{
				std::shared_ptr<kanban_markdown::KanbanAttachment> kanban_attachment = task->attachments[task_vector_index_name];
				kanban_attachment->name = yyjson_get_string_object(value);
				task->attachments[kanban_attachment->name] = kanban_attachment;
				task->attachments.erase(task_vector_index_name);
				break;
			}
			case hash("url"):
			{
				task->attachments[task_vector_index_name]->url = yyjson_get_string_object(value);
				break;
			}
			default:
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanAttachment named "{}")", fourth));
			}
		}

		void parsePath_3_checklist(KanbanTuple &kanban_tuple, std::vector<std::string> &split_result, yyjson_val *value, std::shared_ptr<kanban_markdown::KanbanTask> task, std::string task_vector_index_name)
		{
			if (!task->checklist.contains(task_vector_index_name))
			{
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanTask.checklist named "{}")", task_vector_index_name));
			}
			std::string fourth = split_result[3];
			switch (hash(fourth))
			{
			case hash("name"):
			{
				std::shared_ptr<kanban_markdown::KanbanChecklistItem> kanban_checklist_item = task->checklist[task_vector_index_name];
				kanban_checklist_item->name = yyjson_get_string_object(value);
				task->checklist[kanban_checklist_item->name] = kanban_checklist_item;
				task->checklist.erase(task_vector_index_name);
				break;
			}
			case hash("checked"):
			{
				task->checklist[task_vector_index_name]->checked = yyjson_get_bool(value);
				break;
			}
			default:
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanChecklistItem named "{}")", fourth));
			}
		}

		void parsePath_1_labels(KanbanTuple &kanban_tuple, std::vector<std::string> &split_result, yyjson_val *value, std::string vector_index_name)
		{
			if (!kanban_tuple.kanban_board.labels.contains(vector_index_name))
			{
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanBoard.labels named "{}")", vector_index_name));
			}
			std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label = kanban_tuple.kanban_board.labels[vector_index_name];
			std::string second = split_result[1];
			switch (hash(second))
			{
			case hash("name"):
			{
				kanban_label->name = yyjson_get_string_object(value);
				for (auto &[task_name, task] : kanban_label->tasks)
				{
					task->labels[kanban_label->name] = kanban_label;
					task->labels.erase(vector_index_name);
				}
				kanban_tuple.kanban_board.labels[kanban_label->name] = kanban_label;
				kanban_tuple.kanban_board.labels.erase(vector_index_name);
				break;
			}
			default:
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanLabel named "{}")", second));
			}
		}
	}
	void command_update(KanbanTuple &kanban_tuple, yyjson_val *command)
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

		if (split_result.size() == 1)
		{
			std::string first = split_result[0];
			switch (hash(first))
			{
			case hash("name"):
				kanban_tuple.kanban_board.name = yyjson_get_string_object(value);
				break;
			case hash("description"):
				kanban_tuple.kanban_board.description = yyjson_get_string_object(value);
				break;
			default:
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanBoard named "{}")", first));
			}
		}
		else
		{
			internal_update::parsePath_1(kanban_tuple, split_result, value);
		}
	}
}