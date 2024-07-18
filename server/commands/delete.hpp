#pragma once

#include "../internal.hpp"

#include <fmt/format.h>
#include <re2/re2.h>

namespace server::commands
{
	namespace internal_delete
	{
		void parsePath_1(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result);

		void parsePath_1_list(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, std::string vector_index_name);
		void parsePath_2_tasks(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::string list_vector_index_name);
		void parsePath_3_labels(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, std::shared_ptr<kanban_markdown::KanbanTask> task, std::string task_vector_index_name);
		void parsePath_3_attachments(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, std::shared_ptr<kanban_markdown::KanbanTask> task, std::string task_vector_index_name);
		void parsePath_3_checklist(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, std::shared_ptr<kanban_markdown::KanbanTask> task, std::string task_vector_index_name);

		void parsePath_1_labels(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, std::string vector_index_name);

		void parsePath_1(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result)
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
				parsePath_1_list(kanban_tuple, split_result, vector_index_name);
				break;
			}
			case hash("labels"):
			{
				parsePath_1_labels(kanban_tuple, split_result, vector_index_name);
				break;
			}
			default:
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanBoard named "{}")", first));
			}
		}

		void parsePath_1_list(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, std::string vector_index_name)
		{
			auto it = std::find_if(kanban_tuple.kanban_board.list.begin(), kanban_tuple.kanban_board.list.end(), [&vector_index_name](const auto& x)
				{ return x->name == vector_index_name; });
			if (it == kanban_tuple.kanban_board.list.end())
			{
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanBoard.list named "{}")", vector_index_name));
			}
			if (split_result.size() == 1)
			{
				kanban_tuple.kanban_board.list.erase(it);
			}
			else
			{
				std::shared_ptr<kanban_markdown::KanbanList> kanban_list = *it;
				std::string second = split_result[1];
				switch (hash(second))
				{
				case hash("name"):
					throw std::runtime_error("Invalid path: KanbanList.name is a key and cannot be deleted");
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
						parsePath_2_tasks(kanban_tuple, split_result, kanban_list, list_vector_index_name);
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

		void parsePath_2_tasks(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::string list_vector_index_name)
		{
			auto it = std::find_if(kanban_list->tasks.begin(), kanban_list->tasks.end(), [&list_vector_index_name](const auto& x)
				{ return x->name == list_vector_index_name; });
			if (it == kanban_list->tasks.end())
			{
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanList.tasks named "{}")", list_vector_index_name));
			}
			if (split_result.size() == 2)
			{
				kanban_list->tasks.erase(it);
			}
			else
			{
				std::shared_ptr<kanban_markdown::KanbanTask> task = *it;
				std::string third = split_result[2];
				switch (hash(third))
				{
				case hash("name"):
					throw std::runtime_error("Invalid path: KanbanTask.name is a key and cannot be deleted");
				case hash("description"):
					throw std::runtime_error("Invalid path: KanbanTask.description is a key and cannot be deleted");
				case hash("checked"):
					throw std::runtime_error("Invalid path: KanbanTask.checked is a key and cannot be deleted");
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
						parsePath_3_labels(kanban_tuple, split_result, task, task_vector_index_name);
						break;
					}
					case hash("attachments"):
					{
						parsePath_3_attachments(kanban_tuple, split_result, task, task_vector_index_name);
						break;
					}
					case hash("checklist"):
					{
						parsePath_3_checklist(kanban_tuple, split_result, task, task_vector_index_name);
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

		void parsePath_3_labels(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, std::shared_ptr<kanban_markdown::KanbanTask> task, std::string task_vector_index_name)
		{
			auto it = std::find_if(task->labels.begin(), task->labels.end(), [&task_vector_index_name](const auto& x)
				{ return x->name == task_vector_index_name; });
			if (it == task->labels.end())
			{
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanTask.labels named "{}")", task_vector_index_name));
			}
			if (split_result.size() == 3)
			{
				task->labels.erase(it);
			}
			else
			{
				std::string fourth = split_result[3];
				switch (hash(fourth))
				{
				case hash("name"):
					throw std::runtime_error("Invalid path: KanbanLabel.name is a key and cannot be deleted");
				default:
					throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanLabel named "{}")", fourth));
				}
			}
		}

		void parsePath_3_attachments(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, std::shared_ptr<kanban_markdown::KanbanTask> task, std::string task_vector_index_name)
		{
			auto it = std::find_if(task->attachments.begin(), task->attachments.end(), [&task_vector_index_name](const auto& x)
				{ return x->name == task_vector_index_name; });
			if (it == task->attachments.end())
			{
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanTask.attachments named "{}")", task_vector_index_name));
			}
			if (split_result.size() == 3)
			{
				task->attachments.erase(it);
			}
			else
			{
				std::string fourth = split_result[3];
				switch (hash(fourth))
				{
				case hash("name"):
					throw std::runtime_error("Invalid path: KanbanAttachment.name is a key and cannot be deleted");
				case hash("url"):
					throw std::runtime_error("Invalid path: KanbanAttachment.url is a key and cannot be deleted");
				default:
					throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanAttachment named "{}")", fourth));
				}
			}
		}

		void parsePath_3_checklist(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, std::shared_ptr<kanban_markdown::KanbanTask> task, std::string task_vector_index_name)
		{
			auto it = std::find_if(task->checklist.begin(), task->checklist.end(), [&task_vector_index_name](const auto& x)
				{ return x->name == task_vector_index_name; });
			if (it == task->checklist.end())
			{
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanTask.checklist named "{}")", task_vector_index_name));
			}
			if (split_result.size() == 3)
			{
				task->checklist.erase(it);
			}
			else
			{
				std::string fourth = split_result[3];
				switch (hash(fourth))
				{
				case hash("name"):
					throw std::runtime_error("Invalid path: KanbanChecklistItem.name is a key and cannot be deleted");
				case hash("checked"):
					throw std::runtime_error("Invalid path: KanbanChecklistItem.checked is a key and cannot be deleted");
				default:
					throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanChecklistItem named "{}")", fourth));
				}
			}
		}

		void parsePath_1_labels(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, std::string vector_index_name)
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
				throw std::runtime_error("Invalid path: KanbanLabel.name is a key and cannot be deleted");
			default:
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanLabel named "{}")", second));
			}
		}
	}
	void command_delete(KanbanTuple& kanban_tuple, yyjson_val* command)
	{
		yyjson_val* path = yyjson_obj_get(command, "path");
		if (path == NULL)
		{
			throw std::runtime_error("Unable to find path");
		}
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
			throw std::runtime_error("Invalid path: KanbanBoard.name is a key and cannot be deleted");
		case hash("description"):
			throw std::runtime_error("Invalid path: KanbanBoard.description is a key and cannot be deleted");
		case hash("list"):
			throw std::runtime_error("Invalid path: KanbanBoard.list is a key and cannot be deleted");
		case hash("labels"):
			throw std::runtime_error("Invalid path: KanbanBoard.labels is a key and cannot be deleted");
		default:
			internal_delete::parsePath_1(kanban_tuple, split_result);
			break;
		}
	}
}