#pragma once

#include "../internal.hpp"

#include <fmt/format.h>
#include <re2/re2.h>

namespace server::commands
{
	namespace internal_move
	{
		struct MoveValue {
			unsigned int index;
			std::string destination;
		};

		void parsePath_1(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, internal_move::MoveValue move_value);

		void parsePath_1_list(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, internal_move::MoveValue move_value, std::string vector_index_name);
		void parsePath_2_tasks(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, internal_move::MoveValue move_value, std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::string list_vector_index_name, unsigned int list_vector_index_counter);
		void parsePath_3_checklist(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, internal_move::MoveValue move_value, std::shared_ptr<kanban_markdown::KanbanTask> task, std::string task_vector_index_name);

		void parsePath_1(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, internal_move::MoveValue move_value)
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
				parsePath_1_list(kanban_tuple, split_result, move_value, vector_index_name);
				break;
			}
			case hash("labels"):
				throw std::runtime_error("Unsupported: Moving labels is not supported");
			default:
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanBoard named "{}")", first));
			}
		}

		void parsePath_1_list(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, internal_move::MoveValue move_value, std::string vector_index_name)
		{
			auto it = std::find_if(kanban_tuple.kanban_board.list.begin(), kanban_tuple.kanban_board.list.end(), [&vector_index_name](const auto& x)
				{ return x->name == vector_index_name; });
			if (it == kanban_tuple.kanban_board.list.end())
			{
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanBoard.list named "{}")", vector_index_name));
			}
			if (split_result.size() == 1)
			{
				std::shared_ptr<kanban_markdown::KanbanList> kanban_list = *it;
				it = kanban_tuple.kanban_board.list.erase(it);
				kanban_tuple.kanban_board.list.insert(kanban_tuple.kanban_board.list.begin() + move_value.index, kanban_list);
			}
			else
			{
				std::shared_ptr<kanban_markdown::KanbanList> kanban_list = *it;
				std::string second = split_result[1];
				switch (hash(second))
				{
				case hash("name"):
					throw std::runtime_error("Invalid path: KanbanList.name is a key and cannot be moved");
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
						parsePath_2_tasks(kanban_tuple, split_result, move_value, kanban_list, list_vector_index_name, list_vector_index_counter);
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

		void parsePath_2_tasks(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, internal_move::MoveValue move_value, std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::string list_vector_index_name, unsigned int list_vector_index_counter)
		{
			auto it = std::find_if(kanban_list->tasks.begin(), kanban_list->tasks.end(), [&list_vector_index_name, &list_vector_index_counter](const std::shared_ptr<kanban_markdown::KanbanTask>& x)
				{ return x->name == list_vector_index_name && x->counter == list_vector_index_counter; });
			if (it == kanban_list->tasks.end())
			{
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanList.tasks named "{}" and with counter "{}")", list_vector_index_name, list_vector_index_counter));
			}
			if (split_result.size() == 2)
			{
				if (move_value.destination.empty()) {
					std::shared_ptr<kanban_markdown::KanbanTask> kanban_task = *it;
					it = kanban_list->tasks.erase(it);
					kanban_list->tasks.insert(kanban_list->tasks.begin() + move_value.index, kanban_task);
				}
				else {
					static re2::RE2 destination_pattern(R"(\w+\[(.+)\].tasks)");
					std::string destination_list;
					if (!RE2::PartialMatch(move_value.destination, destination_pattern, &destination_list))
					{
						throw std::runtime_error("Invalid path: The destination must be a list name");
					}

					auto parent_it = std::find_if(kanban_tuple.kanban_board.list.begin(), kanban_tuple.kanban_board.list.end(), [&destination_list](const auto& x)
						{ return x->name == destination_list; });

					if (parent_it == kanban_tuple.kanban_board.list.end())
					{
						throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanBoard.list named "{}")", destination_list));
					}
					std::shared_ptr<kanban_markdown::KanbanList> parent_list = *parent_it;
					int old_index = std::distance(kanban_list->tasks.begin(), it);

					std::shared_ptr<kanban_markdown::KanbanTask> task = kanban_list->tasks[old_index];
					task->checked = parent_list->checked;

					parent_list->tasks.insert(parent_list->tasks.begin() + move_value.index, task);
					kanban_list->tasks.erase(kanban_list->tasks.begin() + old_index);
				}
			}
			else
			{
				std::shared_ptr<kanban_markdown::KanbanTask> task = *it;
				std::string third = split_result[2];
				switch (hash(third))
				{
				case hash("name"):
					throw std::runtime_error("Invalid path: KanbanTask.name is a key and cannot be moved");
				case hash("description"):
					throw std::runtime_error("Invalid path: KanbanTask.description is a key and cannot be moved");
				case hash("checked"):
					throw std::runtime_error("Invalid path: KanbanTask.checked is a key and cannot be moved");
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
						throw std::runtime_error("Unsupported: Moving labels is not supported");
					case hash("attachments"):
						throw std::runtime_error("Unsupported: Moving labels is not supported");
					case hash("checklist"):
					{
						parsePath_3_checklist(kanban_tuple, split_result, move_value, task, task_vector_index_name);
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

		void parsePath_3_checklist(KanbanTuple& kanban_tuple, std::vector<std::string>& split_result, internal_move::MoveValue move_value, std::shared_ptr<kanban_markdown::KanbanTask> task, std::string task_vector_index_name)
		{
			auto it = std::find_if(task->checklist.begin(), task->checklist.end(), [&task_vector_index_name](const auto& x)
				{ return x->name == task_vector_index_name; });
			if (it == task->checklist.end())
			{
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanTask.checklist named "{}")", task_vector_index_name));
			}
			if (split_result.size() == 3)
			{
				static re2::RE2 destination_pattern(R"(\w+\[(.+)\].tasks\[(.+)\].checklist)");
				std::string destination_list;
				if (!RE2::PartialMatch(move_value.destination, destination_pattern, &destination_list))
				{
					throw std::runtime_error("Invalid path: The destination must be a list name");
				}
				// TODO:
				throw std::runtime_error("Unsupported: Moving checklist items is not supported yet");
			}
			else
			{
				std::string fourth = split_result[3];
				switch (hash(fourth))
				{
				case hash("name"):
					throw std::runtime_error("Invalid path: KanbanChecklistItem.name is a key and cannot be moved");
				case hash("checked"):
					throw std::runtime_error("Invalid path: KanbanChecklistItem.checked is a key and cannot be moved");
				default:
					throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanChecklistItem named "{}")", fourth));
				}
			}
		}
	}

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

		internal_move::MoveValue move_value;
		move_value.destination = destination_str;
		move_value.index = index_int;

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
			throw std::runtime_error("Invalid path: KanbanBoard.name is a key and cannot be moved");
		case hash("description"):
			throw std::runtime_error("Invalid path: KanbanBoard.description is a key and cannot be moved");
		case hash("list"):
			throw std::runtime_error("Invalid path: KanbanBoard.list is a key and cannot be moved");
		case hash("labels"):
			throw std::runtime_error("Invalid path: KanbanBoard.labels is a key and cannot be moved");
		default:
			internal_move::parsePath_1(kanban_tuple, split_result, move_value);
			break;
		}
	}
}