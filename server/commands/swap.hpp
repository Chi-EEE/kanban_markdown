#pragma once

#include "../internal.hpp"

#include <fmt/format.h>
#include <re2/re2.h>

namespace server::commands
{
	namespace internal_swap
	{
		void parsePath_1(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, int new_index);

		void parsePath_1_list(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, int new_index, std::string vector_index_name);
		void parsePath_2_tasks(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, int new_index, std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::string list_vector_index_name);
		void parsePath_3_labels(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, int new_index, std::shared_ptr<kanban_markdown::KanbanTask> task, std::string task_vector_index_name);
		void parsePath_3_attachments(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, int new_index, std::shared_ptr<kanban_markdown::KanbanTask> task, std::string task_vector_index_name);
		void parsePath_3_checklist(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, int new_index, std::shared_ptr<kanban_markdown::KanbanTask> task, std::string task_vector_index_name);

		void parsePath_1_labels(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, int new_index, std::string vector_index_name);

		void parsePath_1(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, int new_index)
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
				parsePath_1_list(kanban_tuple, split_result, new_index, vector_index_name);
				break;
			}
			case hash("labels"):
			{
				parsePath_1_labels(kanban_tuple, split_result, new_index, vector_index_name);
				break;
			}
			default:
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanBoard named "{}")", first));
			}
		}

		void parsePath_1_list(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, int new_index, std::string vector_index_name)
		{
			auto it = std::find_if(kanban_tuple.kanban_board.list.begin(), kanban_tuple.kanban_board.list.end(), [&vector_index_name](const auto& x)
				{ return x->name == vector_index_name; });
			if (it == kanban_tuple.kanban_board.list.end())
			{
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanBoard.list named "{}")", vector_index_name));
			}
			if (split_result.size() == 1)
			{
				if (new_index >= kanban_tuple.kanban_board.list.size())
				{
					throw std::runtime_error(fmt::format("Invalid path: Index out of bounds. The list has {} items", kanban_tuple.kanban_board.list.size()));
				}
				int old_index = std::distance(kanban_tuple.kanban_board.list.begin(), it);
				std::swap(kanban_tuple.kanban_board.list[old_index], kanban_tuple.kanban_board.list[new_index]);
			}
			else
			{
				std::shared_ptr<kanban_markdown::KanbanList> kanban_list = *it;
				std::string second = split_result[1];
				switch (hash(second))
				{
				case hash("name"):
					throw std::runtime_error("Invalid path: KanbanList.name is a key and cannot be swapped");
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
						parsePath_2_tasks(kanban_tuple, split_result, new_index, kanban_list, list_vector_index_name);
						break;
					}
					default:
						throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanList named "{}")", second));
					}
					break;
				}
				}
			}
		}

		void parsePath_2_tasks(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, int new_index, std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::string list_vector_index_name)
		{
			auto it = std::find_if(kanban_list->tasks.begin(), kanban_list->tasks.end(), [&list_vector_index_name](const auto& x)
				{ return x->name == list_vector_index_name; });
			if (it == kanban_list->tasks.end())
			{
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanList.tasks named "{}")", list_vector_index_name));
			}
			if (split_result.size() == 2)
			{
				if (new_index >= kanban_list->tasks.size())
				{
					throw std::runtime_error(fmt::format("Invalid path: Index out of bounds. The tasks has {} items", kanban_list->tasks.size()));
				}
				int old_index = std::distance(kanban_list->tasks.begin(), it);
				std::swap(kanban_list->tasks[old_index], kanban_list->tasks[new_index]);
			}
			else
			{
				std::shared_ptr<kanban_markdown::KanbanTask> task = *it;
				std::string third = split_result[2];
				switch (hash(third))
				{
				case hash("name"):
					throw std::runtime_error("Invalid path: KanbanTask.name is a key and cannot be swapped");
				case hash("description"):
					throw std::runtime_error("Invalid path: KanbanTask.description is a key and cannot be swapped");
				case hash("checked"):
					throw std::runtime_error("Invalid path: KanbanTask.checked is a key and cannot be swapped");
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
						parsePath_3_labels(kanban_tuple, split_result, new_index, task, task_vector_index_name);
						break;
					}
					case hash("attachments"):
					{
						parsePath_3_attachments(kanban_tuple, split_result, new_index, task, task_vector_index_name);
						break;
					}
					case hash("checklist"):
					{
						parsePath_3_checklist(kanban_tuple, split_result, new_index, task, task_vector_index_name);
						break;
					}
					default:
						throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanTask named "{}")", third));
					}

					break;
				}
				}
			}
		}

		void parsePath_3_labels(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, int new_index, std::shared_ptr<kanban_markdown::KanbanTask> task, std::string task_vector_index_name)
		{
			auto it = std::find_if(task->labels.begin(), task->labels.end(), [&task_vector_index_name](const auto& x)
				{ return x->name == task_vector_index_name; });
			if (it == task->labels.end())
			{
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanTask.labels named "{}")", task_vector_index_name));
			}
			if (split_result.size() == 3)
			{
				if (new_index >= task->labels.size())
				{
					throw std::runtime_error(fmt::format("Invalid path: Index out of bounds. The labels has {} items", task->labels.size()));
				}
				int old_index = std::distance(task->labels.begin(), it);
				std::swap(task->labels[old_index], task->labels[new_index]);
			}
			else
			{
				std::string fourth = split_result[3];
				switch (hash(fourth))
				{
				case hash("name"):
					throw std::runtime_error("Invalid path: KanbanLabel.name is a key and cannot be swapped");
				default:
					throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanLabel named "{}")", fourth));
				}
			}
		}

		void parsePath_3_attachments(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, int new_index, std::shared_ptr<kanban_markdown::KanbanTask> task, std::string task_vector_index_name)
		{
			auto it = std::find_if(task->attachments.begin(), task->attachments.end(), [&task_vector_index_name](const auto& x)
				{ return x->name == task_vector_index_name; });
			if (it == task->attachments.end())
			{
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanTask.attachments named "{}")", task_vector_index_name));
			}
			if (split_result.size() == 3)
			{
				if (new_index >= task->attachments.size())
				{
					throw std::runtime_error(fmt::format("Invalid path: Index out of bounds. The attachments has {} items", task->attachments.size()));
				}
				int old_index = std::distance(task->attachments.begin(), it);
				std::swap(task->attachments[old_index], task->attachments[new_index]);
			}
			else
			{
				std::string fourth = split_result[3];
				switch (hash(fourth))
				{
				case hash("name"):
					throw std::runtime_error("Invalid path: KanbanAttachment.name is a key and cannot be swapped");
				case hash("url"):
					throw std::runtime_error("Invalid path: KanbanAttachment.url is a key and cannot be swapped");
				default:
					throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanAttachment named "{}")", fourth));
				}
			}
		}

		void parsePath_3_checklist(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, int new_index, std::shared_ptr<kanban_markdown::KanbanTask> task, std::string task_vector_index_name)
		{
			auto it = std::find_if(task->checklist.begin(), task->checklist.end(), [&task_vector_index_name](const auto& x)
				{ return x->name == task_vector_index_name; });
			if (it == task->checklist.end())
			{
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanTask.checklist named "{}")", task_vector_index_name));
			}
			if (split_result.size() == 3)
			{
				if (new_index >= task->checklist.size())
				{
					throw std::runtime_error(fmt::format("Invalid path: Index out of bounds. The checklist has {} items", task->checklist.size()));
				}
				int old_index = std::distance(task->checklist.begin(), it);
				std::swap(task->checklist[old_index], task->checklist[new_index]);
			}
			else
			{
				std::string fourth = split_result[3];
				switch (hash(fourth))
				{
				case hash("name"):
					throw std::runtime_error("Invalid path: KanbanChecklistItem.name is a key and cannot be swapped");
				case hash("checked"):
					throw std::runtime_error("Invalid path: KanbanChecklistItem.checked is a key and cannot be swapped");
				default:
					throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanChecklistItem named "{}")", fourth));
				}
			}
		}

		void parsePath_1_labels(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, int new_index, std::string vector_index_name)
		{
			auto it = std::find_if(kanban_tuple.kanban_board.labels.begin(), kanban_tuple.kanban_board.labels.end(), [&vector_index_name](const auto& x)
				{ return x->name == vector_index_name; });
			if (it == kanban_tuple.kanban_board.labels.end())
			{
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanBoard.labels named "{}")", vector_index_name));
			}
			std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label = *it;
			std::string second = split_result[1];
			switch (hash(second))
			{
			case hash("name"):
				throw std::runtime_error("Invalid path: KanbanLabel.name is a key and cannot be swapped");
			default:
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanLabel named "{}")", second));
			}
		}
	}

	void command_swap(KanbanTuple& kanban_tuple, yyjson_val* command)
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
		int new_index = yyjson_get_uint(value);

		std::string path_str = yyjson_get_string_object(path);
		std::vector<std::string> split_result = split(path_str, ".");

		if (split_result.empty())
		{
			throw std::runtime_error("Invalid path: There are no fields");
		}

		std::string first = split_result[0];
		switch (hash(first))
		{
		case hash("name"):
			throw std::runtime_error("Invalid path: KanbanBoard.name is a key and cannot be swapped");
		case hash("description"):
			throw std::runtime_error("Invalid path: KanbanBoard.description is a key and cannot be swapped");
		case hash("list"):
			throw std::runtime_error("Invalid path: KanbanBoard.list is a key and cannot be swapped");
		case hash("labels"):
			throw std::runtime_error("Invalid path: KanbanBoard.labels is a key and cannot be swapped");
		default:
			internal_swap::parsePath_1(kanban_tuple, split_result, new_index);
			break;
		}
	}
}