#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <optional>

#include <kanban_markdown/kanban_markdown.hpp>

#include "internal.hpp"

#include "commands/update.hpp"

namespace server
{
	class KanbanServer
	{
	public:
		KanbanServer() = default;
		void start()
		{
			yyjson_doc* doc = NULL;
			std::string input;
			std::optional<KanbanTuple> kanban_tuple;

			while (true)
			{
				std::getline(std::cin, input);
				try
				{
					doc = yyjson_read(input.c_str(), input.size(), 0);
					if (doc == NULL)
					{
						throw std::runtime_error("Invalid JSON");
					}
					yyjson_val* root = yyjson_doc_get_root(doc);
					if (root == NULL)
					{
						throw std::runtime_error("Invalid JSON");
					}
					yyjson_val* type = yyjson_obj_get(root, "type");
					if (type == NULL)
					{
						throw std::runtime_error("Unable to find type");
					}
					std::string type_str = yyjson_get_string_object(type);
					switch (hash(type_str))
					{
					case hash("parse"):
					{
						tl::expected<KanbanTuple, std::string> maybe_kanban_tuple = parse(root);
						if (!maybe_kanban_tuple.has_value())
						{
							throw std::runtime_error(maybe_kanban_tuple.error());
						}
						else
						{
							kanban_tuple = maybe_kanban_tuple.value();
							yyjson_mut_doc* doc = yyjson_mut_doc_new(NULL);
							yyjson_mut_val* root = yyjson_mut_obj(doc);
							yyjson_mut_doc_set_root(doc, root);
							yyjson_mut_obj_add_bool(doc, root, "success", true);
							const char* json = yyjson_mut_write(doc, 0, NULL);
							printf("%s\n", json);
							free((void*)json);
						}
						break;
					}
					default:
					{
						if (!kanban_tuple.has_value())
						{
							throw std::runtime_error("No kanban board loaded");
						}
						else
						{
							KanbanTuple& kanban_tuple_ = kanban_tuple.value();
							//with_kanban_tuple(kanban_tuple_, root, type_str);
							kanban_tuple_.kanban_board.version += 1;
							kanban_tuple_.kanban_board.last_modified = kanban_markdown::internal::now_utc();
							std::ofstream md_file;
							md_file.open(kanban_tuple_.file_path);
							md_file << kanban_markdown::markdown_format(kanban_tuple_.kanban_board);
							md_file.close();
							yyjson_mut_doc* doc = yyjson_mut_doc_new(NULL);
							yyjson_mut_val* root = yyjson_mut_obj(doc);
							yyjson_mut_doc_set_root(doc, root);
							yyjson_mut_obj_add_bool(doc, root, "success", true);
							const char* json = yyjson_mut_write(doc, 0, NULL);
							printf("%s\n", json);
							free((void*)json);
						}
						break;
					}
					}
				}
				catch (const std::exception& e)
				{
					yyjson_mut_doc* doc = yyjson_mut_doc_new(NULL);
					yyjson_mut_val* root = yyjson_mut_obj(doc);
					yyjson_mut_doc_set_root(doc, root);
					yyjson_mut_obj_add_bool(doc, root, "success", false);
					yyjson_mut_obj_add_str(doc, root, "error", e.what());
					const char* json = yyjson_mut_write(doc, 0, NULL);
					printf("%s\n", json);
					free((void*)json);
				}
				if (doc != NULL)
				{
					yyjson_doc_free(doc);
				}
			}
		}

		static tl::expected<KanbanTuple, std::string> parse(yyjson_val* root)
		{
			yyjson_val* file = yyjson_obj_get(root, "file");
			if (file == NULL)
			{
				throw std::runtime_error("Unable to find file");
			}
			std::string file_path = yyjson_get_string_object(file);

			if (!std::filesystem::exists(file_path))
			{
				throw std::runtime_error("File does not exist");
			}

			std::ifstream file_stream(file_path);
			std::stringstream buffer;
			buffer << file_stream.rdbuf();
			const std::string input_file_string = buffer.str();
			file_stream.close();
			buffer.clear();

			auto& maybe_kanban_board = kanban_markdown::parse(input_file_string);
			if (!maybe_kanban_board.has_value())
			{
				return tl::make_unexpected(maybe_kanban_board.error());
			}

			KanbanTuple kanban_tuple;
			kanban_tuple.file_path = file_path;
			kanban_tuple.kanban_board = maybe_kanban_board.value();
			return kanban_tuple;
		}
	};
}