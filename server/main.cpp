#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <optional>

#include <yyjson.h>

#include <kanban_markdown/kanban_markdown.hpp>

static constexpr inline uint32_t hash(const std::string_view s) noexcept
{
	uint32_t hash = 5381;

	for (const char* c = s.data(); c < s.data() + s.size(); ++c)
		hash = ((hash << 5) + hash) + (unsigned char)*c;

	return hash;
}

static inline std::string yyjson_get_string_object(yyjson_val* val) {
	const char* val_data = yyjson_get_str(val);
	int val_size = yyjson_get_len(val);
	return std::string(val_data, val_size);
}

struct KanbanTuple {
	std::string file_path;
	kanban_markdown::KanbanBoard kanban_board;
};

static void setKanbanBoardProperty(kanban_markdown::KanbanBoard& kanban_board, yyjson_val* root) {
	yyjson_val* property = yyjson_obj_get(root, "property");
	if (property == NULL) {
		throw std::runtime_error("Unable to find property");
	}

	yyjson_val* value = yyjson_obj_get(root, "value");
	if (value == NULL) {
		throw std::runtime_error("Unable to find value");
	}

	std::string property_str = yyjson_get_string_object(property);
	std::string value_str = yyjson_get_string_object(value);

	switch (hash(property_str)) {
	case hash("name"): {
		kanban_board.name = value_str;
		break;
	}
	case hash("description"): {
		kanban_board.description = value_str;
		break;
	}
	}
}

static tl::expected<KanbanTuple, std::string> parse(yyjson_val* root) {
	yyjson_val* file = yyjson_obj_get(root, "file");
	if (file == NULL) {
		throw std::runtime_error("Unable to find file");
	}
	std::string file_path = yyjson_get_string_object(file);

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
		return tl::make_unexpected(maybe_kanban_board.error());
	}

	KanbanTuple kanban_tuple;
	kanban_tuple.file_path = file_path;
	kanban_tuple.kanban_board = maybe_kanban_board.value();
	return kanban_tuple;
}

inline static void with_kanban_tuple(KanbanTuple& kanban_tuple, yyjson_val* root, std::string type_str) {
	switch (hash(type_str))
	{
	case hash("get"): {
		std::cout << kanban_markdown::json_format(kanban_tuple.kanban_board) << '\n';
		break;
	}
	case hash("set"): {
		setKanbanBoardProperty(kanban_tuple.kanban_board, root);
		break;
	}
	case hash("add_label"): {
		yyjson_val* name = yyjson_obj_get(root, "name");
		if (name == NULL) {
			throw std::runtime_error("Unable to find name");
		}
		std::string name_str = yyjson_get_string_object(name);
		std::shared_ptr<kanban_markdown::KanbanLabel> label = std::make_shared<kanban_markdown::KanbanLabel>();
		label->name = name_str;
		kanban_tuple.kanban_board.labels.insert({ name_str, label });
		break;
	}
	case hash("add_list"): {
		yyjson_val* name = yyjson_obj_get(root, "name");
		if (name == NULL) {
			throw std::runtime_error("Unable to find name");
		}
		std::string name_str = yyjson_get_string_object(name);
		kanban_markdown::KanbanList list;
		list.name = name_str;
		kanban_tuple.kanban_board.list.insert({ name_str, list });
		break;
	}
	case hash("add_task_to_list"): {
		yyjson_val* list = yyjson_obj_get(root, "list");
		if (list == NULL) {
			throw std::runtime_error("Unable to find list");
		}

		yyjson_val* name = yyjson_obj_get(root, "name");
		if (name == NULL) {
			throw std::runtime_error("Unable to find name");
		}

		yyjson_val* checked = yyjson_obj_get(root, "checked");
		if (checked == NULL) {
			throw std::runtime_error("Unable to find checked");
		}
		std::string list_str = yyjson_get_string_object(list);
		if (!kanban_tuple.kanban_board.list.contains(list_str)) {
			throw std::runtime_error(fmt::format("'[{}]' list does not exist", list_str));
		}

		std::string name_str = yyjson_get_string_object(name);

		std::shared_ptr<kanban_markdown::KanbanTask> task = std::make_shared<kanban_markdown::KanbanTask>();
		task->checked = yyjson_get_bool(checked);
		task->name = name_str;

		kanban_tuple.kanban_board.list[list_str].tasks.insert({ name_str, task });
		break;
	}
	case hash("add_attachment_to_task"): {
		yyjson_val* task = yyjson_obj_get(root, "task");
		if (task == NULL) {
			throw std::runtime_error("Unable to find task");
		}

		yyjson_val* name = yyjson_obj_get(root, "name");
		if (name == NULL) {
			throw std::runtime_error("Unable to find name");
		}

		yyjson_val* url = yyjson_obj_get(root, "url");
		if (url == NULL) {
			throw std::runtime_error("Unable to find url");
		}

		std::string task_str = yyjson_get_string_object(task);
		std::string name_str = yyjson_get_string_object(name);
		std::string url_str = yyjson_get_string_object(url);

		if (!kanban_tuple.kanban_board.list.contains(task_str)) {
			throw std::runtime_error(fmt::format("'[{}]' task does not exist", task_str));
		}

		kanban_tuple.kanban_board.list[task_str].tasks[name_str]->attachments.push_back(kanban_markdown::KanbanAttachment{ name_str, url_str });
		break;
	}
	case hash("add_checklist_item_to_task"): {
		yyjson_val* task = yyjson_obj_get(root, "task");
		if (task == NULL) {
			throw std::runtime_error("Unable to find task");
		}

		yyjson_val* checked = yyjson_obj_get(root, "checked");
		if (checked == NULL) {
			throw std::runtime_error("Unable to find checked");
		}

		yyjson_val* name = yyjson_obj_get(root, "name");
		if (name == NULL) {
			throw std::runtime_error("Unable to find name");
		}

		std::string task_str = yyjson_get_string_object(task);
		bool checked_bool = yyjson_get_bool(checked);
		std::string name_str = yyjson_get_string_object(name);

		if (!kanban_tuple.kanban_board.list.contains(task_str)) {
			throw std::runtime_error(fmt::format("'[{}]' task does not exist", task_str));
		}

		kanban_tuple.kanban_board.list[task_str].tasks[name_str]->checklist.push_back(kanban_markdown::KanbanChecklistItem{ checked_bool, name_str });
		break;
	}
	case hash("add_label_to_task"): {
		yyjson_val* task = yyjson_obj_get(root, "task");
		if (task == NULL) {
			throw std::runtime_error("Unable to find task");
		}

		yyjson_val* label = yyjson_obj_get(root, "label");
		if (label == NULL) {
			throw std::runtime_error("Unable to find label");
		}

		std::string task_str = yyjson_get_string_object(task);
		std::string label_str = yyjson_get_string_object(label);

		if (!kanban_tuple.kanban_board.list.contains(task_str)) {
			throw std::runtime_error(fmt::format("'[{}]' task does not exist", task_str));
		}

		if (!kanban_tuple.kanban_board.labels.contains(label_str)) {
			throw std::runtime_error(fmt::format("'[{}]' label does not exist", label_str));
		}

		kanban_tuple.kanban_board.list[task_str].tasks[task_str]->labels.push_back(kanban_tuple.kanban_board.labels[label_str]);
		kanban_tuple.kanban_board.labels[label_str]->tasks.push_back(kanban_tuple.kanban_board.list[task_str].tasks[task_str]);
		break;
	}
	case hash("move_task_to_list"): {
		yyjson_val* task = yyjson_obj_get(root, "task");
		if (task == NULL) {
			throw std::runtime_error("Unable to find task");
		}

		yyjson_val* list = yyjson_obj_get(root, "list");
		if (list == NULL) {
			throw std::runtime_error("Unable to find list");
		}

		std::string task_str = yyjson_get_string_object(task);
		std::string list_str = yyjson_get_string_object(list);

		if (!kanban_tuple.kanban_board.list.contains(task_str)) {
			throw std::runtime_error(fmt::format("'[{}]' task does not exist", task_str));
		}

		if (!kanban_tuple.kanban_board.list.contains(list_str)) {
			throw std::runtime_error(fmt::format("'[{}]' list does not exist", list_str));
		}

		kanban_tuple.kanban_board.list[list_str].tasks.insert({ task_str, kanban_tuple.kanban_board.list[task_str].tasks[task_str] });
		kanban_tuple.kanban_board.list[task_str].tasks.erase(task_str);
		break;
	}
	case hash("remove_label"): {
		yyjson_val* name = yyjson_obj_get(root, "name");
		if (name == NULL) {
			throw std::runtime_error("Unable to find name");
		}
		std::string name_str = yyjson_get_string_object(name);
		if (!kanban_tuple.kanban_board.labels.contains(name_str)) {
			throw std::runtime_error(fmt::format("'[{}]' label does not exist", name_str));
		}
		for (std::shared_ptr<kanban_markdown::KanbanTask> task : kanban_tuple.kanban_board.labels[name_str]->tasks) {
			task->labels.erase(std::remove(task->labels.begin(), task->labels.end(), kanban_tuple.kanban_board.labels[name_str]), task->labels.end());
		}
		kanban_tuple.kanban_board.labels.erase(name_str);
		break;
	}
	case hash("remove_list"): {
		yyjson_val* name = yyjson_obj_get(root, "name");
		if (name == NULL) {
			throw std::runtime_error("Unable to find name");
		}
		std::string name_str = yyjson_get_string_object(name);
		if (!kanban_tuple.kanban_board.list.contains(name_str)) {
			throw std::runtime_error(fmt::format("'[{}]' list does not exist", name_str));
		}
		for (auto& [task_name, task] : kanban_tuple.kanban_board.list[name_str].tasks) {
			for (std::shared_ptr<kanban_markdown::KanbanLabel> label : task->labels) {
				label->tasks.erase(std::remove(label->tasks.begin(), label->tasks.end(), task), label->tasks.end());
			}
		}
		kanban_tuple.kanban_board.list.erase(name_str);
		break;
	}
	case hash("remove_task"): {
		yyjson_val* list = yyjson_obj_get(root, "list");
		if (list == NULL) {
			throw std::runtime_error("Unable to find list");
		}

		yyjson_val* task = yyjson_obj_get(root, "task");
		if (task == NULL) {
			throw std::runtime_error("Unable to find task");
		}
		std::string task_str = yyjson_get_string_object(task);
		std::string list_str = yyjson_get_string_object(list);
		for (auto& [label_name, label] : kanban_tuple.kanban_board.labels) {
			label->tasks.erase(std::remove(label->tasks.begin(), label->tasks.end(), kanban_tuple.kanban_board.list[task_str].tasks[task_str]), label->tasks.end());
		}
		break;
	}
	default:
	{
		break;
	}
	}
}

int main() {
	yyjson_doc* doc = NULL;
	std::string input;
	std::optional<KanbanTuple> kanban_tuple;
	while (true)
	{
		std::getline(std::cin, input);
		try {
			doc = yyjson_read(input.c_str(), input.size(), 0);
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
				tl::expected<KanbanTuple, std::string> maybe_kanban_tuple = parse(root);
				if (!maybe_kanban_tuple.has_value()) {
					throw std::runtime_error(maybe_kanban_tuple.error());
				}
				else {
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
				if (!kanban_tuple.has_value()) {
					throw std::runtime_error("No kanban board loaded");
				}
				else {
					KanbanTuple& kanban_tuple_ = kanban_tuple.value();
					with_kanban_tuple(kanban_tuple_, root, type_str);
					kanban_tuple_.kanban_board.last_updated = kanban_markdown::internal::now_utc();
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
		if (doc != NULL) {
			yyjson_doc_free(doc);
		}
	}

	return 0;
}