#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include <sstream>

#include <yyjson.h>

#include <kanban_markdown/kanban_markdown.hpp>

static constexpr inline uint32_t hash(const std::string_view s) noexcept
{
	uint32_t hash = 5381;

	for (const char* c = s.data(); c < s.data() + s.size(); ++c)
		hash = ((hash << 5) + hash) + (unsigned char)*c;

	return hash;
}

int main() {
	using namespace kanban_markdown;
	std::vector<KanbanBoard> boards;

	std::string input;
	while (true)
	{
		std::getline(std::cin, input);
		try {
			yyjson_doc* doc = yyjson_read(input.c_str(), input.size(), 0);
			if (doc == NULL) {
				throw std::runtime_error("Invalid JSON");
			}
			yyjson_val* root = yyjson_doc_get_root(doc);
			if (root == NULL) {
				throw std::runtime_error("Invalid JSON");
			}
			yyjson_val* type = yyjson_obj_get(root, "type");
			if (type == NULL) {
				throw std::runtime_error("Unable to find type");
			}
			const char* type_data = yyjson_get_str(type);
			int type_size = yyjson_get_len(type);

			std::string type_str(type_data, type_size);
			switch (hash(type_str)) {
			case hash("parse"): {
				yyjson_val* file = yyjson_obj_get(root, "file");
				if (file == NULL) {
					throw std::runtime_error("Unable to find file");
				}
				const char* file_path_data = yyjson_get_str(file);
				int file_path_size = yyjson_get_len(file);

				std::string file_path(file_path_data, file_path_size);
				if (!std::filesystem::exists(file_path)) {
					throw std::runtime_error("File does not exist");
				}

				std::ifstream file_stream(file_path);
				std::stringstream buffer;
				buffer << file_stream.rdbuf();
				const std::string input_file_string = buffer.str();
				file_stream.close();
				buffer.clear();

				auto& maybe_kanban_board = kanban_markdown::parse(input_file_string);
				if (!maybe_kanban_board.has_value()) {
					throw std::runtime_error(maybe_kanban_board.error());
				}
				else {
					boards.push_back(maybe_kanban_board.value());
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
		catch (const std::exception& e) {
			yyjson_mut_doc* doc = yyjson_mut_doc_new(NULL);
			yyjson_mut_val* root = yyjson_mut_obj(doc);
			yyjson_mut_doc_set_root(doc, root);
			yyjson_mut_obj_add_bool(doc, root, "success", false);
			yyjson_mut_obj_add_str(doc, root, "error", e.what());
			const char* json = yyjson_mut_write(doc, 0, NULL);
			printf("%s\n", json);
			free((void*)json);
		}
	}

	return 0;
}