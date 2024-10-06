#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <optional>

#include <kanban_markdown/kanban_markdown.hpp>

#include <tobiaslocker_base64/base64.hpp>

#include <gzip/compress.hpp>
#include <gzip/decompress.hpp>

#include "constants.hpp"
#include "internal.hpp"

#include "commands/create.hpp"
#include "commands/update.hpp"
#include "commands/delete.hpp"
#include "commands/move.hpp"

namespace server
{
	struct KanbanCommandResult
	{
		bool modified = false;
		std::string message;
	};

	class KanbanServer
	{
	public:
		KanbanServer() = default;
		void start()
		{
			yyjson_doc *doc = NULL;
			std::string input;
			std::optional<KanbanTuple> kanban_tuple;

			while (true)
			{
				std::getline(std::cin, input);
				if (input.empty())
				{
					continue;
				}
				try
				{
					doc = yyjson_read(input.c_str(), input.size(), 0);
					if (doc == NULL)
					{
						throw std::runtime_error("The request made to the server is invalid; it must be in JSON format.");
					}
					yyjson_val *root = yyjson_doc_get_root(doc);
					if (root == NULL)
					{
						throw std::runtime_error("The request made to the server is invalid; it must be in JSON format.");
					}
					yyjson_val *id = yyjson_obj_get(root, "id");
					if (id == NULL)
					{
						throw std::runtime_error("All requests made to the server require an ID to track the server's response.");
					}
					yyjson_val *type = yyjson_obj_get(root, "type");
					if (type == NULL)
					{
						throw std::runtime_error("All requests made to the server require a type to determine the action to be taken.");
					}
					std::string id_str = server::yyjson_get_string_object(id);
					std::string type_str = server::yyjson_get_string_object(type);
					switch (kanban_markdown::internal::hash(type_str))
					{
					case kanban_markdown::internal::hash("parseFile"):
					{
						tl::expected<KanbanTuple, std::string> maybe_kanban_tuple = parseFile(root);
						if (!maybe_kanban_tuple.has_value())
						{
							throw std::runtime_error(maybe_kanban_tuple.error());
						}
						else
						{
							kanban_tuple = maybe_kanban_tuple.value();
							yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
							yyjson_mut_val *root = yyjson_mut_obj(doc);
							yyjson_mut_doc_set_root(doc, root);
							yyjson_mut_obj_add_str(doc, root, "id", id_str.c_str());
							yyjson_mut_obj_add_bool(doc, root, "success", true);
							std::string json = std::string(yyjson_mut_write(doc, 0, NULL));
							std::cout << json << std::endl;
						}
						break;
					}
					case kanban_markdown::internal::hash("parseFileWithContent"):
					{
						tl::expected<KanbanTuple, std::string> maybe_kanban_tuple = parseFileWithContent(root);
						if (!maybe_kanban_tuple.has_value())
						{
							throw std::runtime_error(maybe_kanban_tuple.error());
						}
						else
						{
							kanban_tuple = maybe_kanban_tuple.value();
							yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
							yyjson_mut_val *root = yyjson_mut_obj(doc);
							yyjson_mut_doc_set_root(doc, root);
							yyjson_mut_obj_add_str(doc, root, "id", id_str.c_str());
							yyjson_mut_obj_add_bool(doc, root, "success", true);
							std::string json = std::string(yyjson_mut_write(doc, 0, NULL));
							std::cout << json << std::endl;
						}
						break;
					}
					default:
					{
						if (!kanban_tuple.has_value())
						{
							throw std::runtime_error("No kanban board has been parsed yet.");
						}
						else
						{
							KanbanTuple &kanban_tuple_ = kanban_tuple.value();
							KanbanCommandResult kanban_command_result = KanbanServer::withKanbanBoard(kanban_tuple_.kanban_board, root, id_str, type_str);
							if (kanban_command_result.modified)
							{
								kanban_tuple_.kanban_board.version += 1;
								kanban_tuple_.kanban_board.last_modified = kanban_markdown::internal::now_utc();
							}
							std::cout << kanban_command_result.message << std::endl;
						}
						break;
					}
					}
				}
				catch (const std::exception &e)
				{
					yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
					yyjson_mut_val *root = yyjson_mut_obj(doc);
					yyjson_mut_doc_set_root(doc, root);
					yyjson_mut_obj_add_bool(doc, root, "success", false);
					yyjson_mut_obj_add_str(doc, root, "error", e.what());
					std::string json = std::string(yyjson_mut_write(doc, 0, NULL));
					std::cout << json << std::endl;
				}
				if (doc != NULL)
				{
					yyjson_doc_free(doc);
				}
			}
		}

		static KanbanCommandResult withKanbanBoard(kanban_markdown::KanbanBoard &kanban_board, yyjson_val *root, std::string id_str, std::string type_str)
		{
			switch (kanban_markdown::internal::hash(type_str))
			{
			case kanban_markdown::internal::hash("commands"):
				return KanbanServer::commands(kanban_board, root, id_str);
			case kanban_markdown::internal::hash("get"):
				return KanbanServer::get(kanban_board, root, id_str);
			default:
				return KanbanCommandResult{
					false,
					fmt::format(R"(Error: Unknown command type "{}".)", type_str),
				};
			}
		}

		static KanbanCommandResult get(kanban_markdown::KanbanBoard &kanban_board, yyjson_val *root, std::string id_str)
		{
			yyjson_val *format = yyjson_obj_get(root, "format");
			if (format == NULL)
			{
				return KanbanCommandResult{
					false,
					"Error: Missing required 'format' field in root object.",
				};
			}

			std::string format_str = yyjson_get_string_object(format);

			std::string message = [&]()
			{
				switch (kanban_markdown::internal::hash(format_str))
				{
				case kanban_markdown::internal::hash("json"):
					return KanbanServer::get_json(kanban_board, id_str);
				case kanban_markdown::internal::hash("markdown"):
					return KanbanServer::get_markdown(kanban_board, id_str);
				default:
					return fmt::format(R"(Error: Unknown format type "{}".)", format_str);
				}
			}();
			return KanbanCommandResult{
				false,
				message,
			};
		}

		static std::string get_json(kanban_markdown::KanbanBoard &kanban_board, std::string id_str)
		{
			yyjson_mut_doc *new_doc = yyjson_mut_doc_new(nullptr);
			yyjson_mut_val *new_root = yyjson_mut_obj(new_doc);
			yyjson_mut_doc_set_root(new_doc, new_root);
			yyjson_mut_obj_add_str(new_doc, new_root, "id", id_str.c_str());
			yyjson_mut_val *kanban_board_object = yyjson_mut_obj(new_doc);
			yyjson_mut_obj_add_val(new_doc, new_root, "json", kanban_board_object);
			kanban_markdown::writer::json::format(kanban_board, new_doc, kanban_board_object);
			std::string json = std::string(yyjson_mut_write(new_doc, 0, NULL));
			yyjson_mut_doc_free(new_doc);
			return json;
		}

		static std::string get_markdown(kanban_markdown::KanbanBoard &kanban_board, std::string id_str)
		{
			yyjson_mut_doc *new_doc = yyjson_mut_doc_new(nullptr);
			yyjson_mut_val *new_root = yyjson_mut_obj(new_doc);
			yyjson_mut_doc_set_root(new_doc, new_root);
			yyjson_mut_obj_add_str(new_doc, new_root, "id", id_str.c_str());
			const std::string md_string = kanban_markdown::writer::markdown::format_str(kanban_board);
			const std::string compressed_md_string = gzip::compress(md_string.data(), md_string.size(), Z_BEST_COMPRESSION);
			const std::string md_base64_string = base64::to_base64(compressed_md_string);
			yyjson_mut_obj_add_str(new_doc, new_root, "markdown", md_base64_string.c_str());
			std::string json = std::string(yyjson_mut_write(new_doc, 0, NULL));
			yyjson_mut_doc_free(new_doc);
			return json;
		}

		static KanbanCommandResult commands(kanban_markdown::KanbanBoard &kanban_board, yyjson_val *root, std::string id_str)
		{
			yyjson_val *commands = yyjson_obj_get(root, "commands");
			if (!commands || !yyjson_is_arr(commands))
			{
				yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
				yyjson_mut_val *root = yyjson_mut_obj(doc);
				yyjson_mut_doc_set_root(doc, root);
				yyjson_mut_obj_add_str(doc, root, "id", id_str.c_str());
				yyjson_mut_obj_add_bool(doc, root, "success", false);
				std::string json = std::string(yyjson_mut_write(doc, 0, NULL));
				yyjson_mut_doc_free(doc);
				return KanbanCommandResult{
					false,
					json,
				};
			}
			yyjson_mut_doc *new_doc = yyjson_mut_doc_new(NULL);
			yyjson_mut_val *new_root = yyjson_mut_obj(new_doc);
			yyjson_mut_doc_set_root(new_doc, new_root);
			yyjson_mut_obj_add_str(new_doc, new_root, "id", id_str.c_str());
			yyjson_mut_val *commands_array = yyjson_mut_arr(new_doc);
			yyjson_mut_obj_add_val(new_doc, new_root, "commands", commands_array);

			bool modified = false;

			yyjson_val *command;
			size_t idx, max;
			yyjson_arr_foreach(commands, idx, max, command)
			{
				if (!yyjson_is_obj(command))
					continue;
				yyjson_mut_val *command_obj = yyjson_mut_obj(new_doc);

				yyjson_val *action = yyjson_obj_get(command, "action");
				if (action == NULL)
				{
					throw std::runtime_error(fmt::format(R"(Error: Missing required 'action' field in command object at index "{}")", idx));
				}
				std::string action_str = server::yyjson_get_string_object(action);
				bool success = false;
				try
				{
					switch (kanban_markdown::internal::hash(action_str))
					{
					case kanban_markdown::internal::hash("create"):
					{
						commands::command_create(kanban_board, command);
						modified = true;
						success = true;
						break;
					}
					case kanban_markdown::internal::hash("update"):
					{
						commands::command_update(kanban_board, command);
						modified = true;
						success = true;
						break;
					}
					case kanban_markdown::internal::hash("delete"):
					{
						commands::command_delete(kanban_board, command);
						modified = true;
						success = true;
						break;
					}
					case kanban_markdown::internal::hash("move"):
					{
						commands::command_move(kanban_board, command);
						modified = true;
						success = true;
						break;
					}
					default:
						break;
					}
				}
				catch (const std::exception &e)
				{
					yyjson_mut_obj_add_str(new_doc, command_obj, "error", e.what());
				}
				yyjson_mut_obj_add_bool(new_doc, command_obj, "success", success);
				yyjson_mut_arr_append(commands_array, command_obj);
			}

			std::string json = std::string(yyjson_mut_write(new_doc, 0, NULL));
			yyjson_mut_doc_free(new_doc);

			return KanbanCommandResult{
				modified,
				json,
			};
		}

		static tl::expected<KanbanTuple, std::string> parseFile(yyjson_val *root)
		{
			yyjson_val *file = yyjson_obj_get(root, "file");
			if (file == NULL)
				return tl::make_unexpected("Error: Missing required 'file' field in root object.");
			std::string file_path = server::yyjson_get_string_object(file);

			if (!std::filesystem::exists(file_path))
				return tl::make_unexpected(fmt::format(R"(Error: File path "{}" does not exist.)", file_path));

			std::ifstream file_stream(file_path, std::ios::binary);
			std::stringstream buffer;
			buffer << file_stream.rdbuf();
			const std::string content_str = buffer.str();
			file_stream.close();
			buffer.clear();

			tl::expected<kanban_markdown::KanbanBoard, std::string> maybe_kanban_board = kanban_markdown::reader::parse(content_str);
			if (!maybe_kanban_board.has_value())
				return tl::make_unexpected(maybe_kanban_board.error());

			KanbanTuple kanban_tuple;
			kanban_tuple.file_path = file_path;
			kanban_tuple.kanban_board = maybe_kanban_board.value();
			return kanban_tuple;
		}

		static tl::expected<KanbanTuple, std::string> parseFileWithContent(yyjson_val *root)
		{
			yyjson_val *file = yyjson_obj_get(root, "file");
			if (file == NULL)
				return tl::make_unexpected("Error: Missing required 'file' field in root object.");
			yyjson_val *content = yyjson_obj_get(root, "content");
			if (content == NULL)
				return tl::make_unexpected("Error: Missing required 'content' field in root object.");
			std::string file_path = server::yyjson_get_string_object(file);

			if (!std::filesystem::exists(file_path))
				return tl::make_unexpected(fmt::format(R"(Error: File path "{}" does not exist.)", file_path));

			const std::string content_b64_str = server::yyjson_get_string_object(content);
			const std::string content_compressed_str = base64::from_base64(content_b64_str);

			const std::string content_str = gzip::decompress(content_compressed_str.c_str(), content_compressed_str.size());

			tl::expected<kanban_markdown::KanbanBoard, std::string> maybe_kanban_board = kanban_markdown::reader::parse(content_str);
			if (!maybe_kanban_board.has_value())
				return tl::make_unexpected(maybe_kanban_board.error());

			KanbanTuple kanban_tuple;
			kanban_tuple.file_path = file_path;
			kanban_tuple.kanban_board = maybe_kanban_board.value();
			return kanban_tuple;
		}
	};
}