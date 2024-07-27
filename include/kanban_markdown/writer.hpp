#pragma once

#include <iostream>
#include <vector>
#include <sstream>

#include <fmt/format.h>

#include <yaml-cpp/yaml.h>
#include <yyjson.h>

#include <picosha2.h>

#include "kanban.hpp"
#include "constants.hpp"
#include "internal.hpp"

namespace kanban_markdown {
	struct KanbanWriterFlags {
		bool github = true;
	};

	inline std::string markdown_format(KanbanBoard kanban_board, KanbanWriterFlags kanban_writer_flags = KanbanWriterFlags()) {
		std::string markdown_file;
		markdown_file += '\n';
#pragma region Note
		markdown_file += "> [!NOTE]" + constants::END_OF_MARKDOWN_LINE;
		markdown_file += "> This file is generated by Kanban_MD." + constants::END_OF_MARKDOWN_LINE;
#pragma endregion
		markdown_file += '\n';
#pragma region Header and Description
		markdown_file += "# " + (!kanban_board.name.empty() ? kanban_board.name : constants::default_board_name) + constants::END_OF_MARKDOWN_LINE;
		markdown_file += (!kanban_board.description.empty() ? kanban_board.description : constants::default_description) + constants::END_OF_MARKDOWN_LINE;
#pragma endregion
		markdown_file += '\n';
#pragma region Labels:
		if (!kanban_board.labels.empty()) {
			markdown_file += "## Labels:" + constants::END_OF_MARKDOWN_LINE;
			for (auto& kanban_label : kanban_board.labels) {
				const std::string kanban_label_name = kanban_label->name;
				markdown_file += fmt::format(
					R"(- <span id="{kanban_md}-label-{id}" data-color="{color}">{name}</span>{eol})",
					fmt::arg("kanban_md", constants::kanban_md),
					fmt::arg("id", kanban_markdown_string_to_id(kanban_label_name)),
					fmt::arg("color", kanban_label->color),
					fmt::arg("name", kanban_label_name),
					fmt::arg("eol", constants::END_OF_MARKDOWN_LINE)
				);
				for (auto& kanban_task : kanban_label->tasks) {
					const std::string kanban_task_name = kanban_task->name;
					markdown_file += fmt::format(
						R"(  - [{name}](#{github}{kanban_md}-task-{id}-{counter}){eol})",
						fmt::arg("github", kanban_writer_flags.github ? constants::github_added_tag : ""),
						fmt::arg("kanban_md", constants::kanban_md),
						fmt::arg("id", kanban_markdown_string_to_id(kanban_task_name)),
						fmt::arg("counter", kanban_task->counter),
						fmt::arg("name", kanban_task_name),
						fmt::arg("eol", constants::END_OF_MARKDOWN_LINE)
					);
				}
			}
			markdown_file += '\n';
		}
#pragma endregion
#pragma region Board
		if (!kanban_board.list.empty()) {
			markdown_file += "## Board:" + constants::END_OF_MARKDOWN_LINE;
			markdown_file += '\n';
			for (auto& kanban_list : kanban_board.list) {
				markdown_file += fmt::format(R"(### <span data-checked="{checked}">{name}</span>{eol})",
					fmt::arg("checked", kanban_list->checked),
					fmt::arg("name", kanban_list->name),
					fmt::arg("eol", constants::END_OF_MARKDOWN_LINE)
				);
				for (auto& kanban_task : kanban_list->tasks) {
					markdown_file += fmt::format(R"(- [{checked}] <span id="{kanban_md}-task-{id}-{counter}" data-counter="{counter}">{name}</span>{eol})",
						fmt::arg("checked", kanban_task->checked ? 'x' : ' '),
						fmt::arg("kanban_md", constants::kanban_md),
						fmt::arg("id", kanban_markdown_string_to_id(kanban_task->name)),
						fmt::arg("counter", kanban_task->counter),
						fmt::arg("name", kanban_task->name),
						fmt::arg("eol", constants::END_OF_MARKDOWN_LINE)
					);
					if (!kanban_task->description.empty()) {
						std::string description_string = "  - **Description**:  ";
						for (std::string description_line : kanban_task->description) {
							description_string += "\n  " + description_line + "  ";
						}
						markdown_file += description_string + constants::END_OF_MARKDOWN_LINE;
					}
					if (!kanban_task->labels.empty()) {
						markdown_file += "  - **Labels**:" + constants::END_OF_MARKDOWN_LINE;
						for (auto& kanban_label : kanban_task->labels) {
							const std::string kanban_label_name = kanban_label->name;
							markdown_file += fmt::format(
								"    - [{name}](#{github}{kanban_md}-label-{id}){eol}",
								fmt::arg("github", kanban_writer_flags.github ? constants::github_added_tag : ""),
								fmt::arg("kanban_md", constants::kanban_md),
								fmt::arg("id", kanban_markdown_string_to_id(kanban_label_name)),
								fmt::arg("name", kanban_label_name),
								fmt::arg("eol", constants::END_OF_MARKDOWN_LINE)
							);
						}
					}
					if (!kanban_task->attachments.empty()) {
						markdown_file += "  - **Attachments**:" + constants::END_OF_MARKDOWN_LINE;
						for (auto& kanban_attachment : kanban_task->attachments) {
							markdown_file += fmt::format(
								"    - [{name}]({url}){eol}",
								fmt::arg("name", kanban_attachment->name),
								fmt::arg("url", kanban_attachment->url),
								fmt::arg("eol", constants::END_OF_MARKDOWN_LINE)
							);
						}
					}
					if (!kanban_task->checklist.empty()) {
						markdown_file += "  - **Checklist**:" + constants::END_OF_MARKDOWN_LINE;
						for (auto& kanban_checklist_item : kanban_task->checklist) {
							markdown_file += fmt::format(
								"    - [{checked}] {name}{eol}",
								fmt::arg("checked", kanban_checklist_item->checked ? 'x' : ' '),
								fmt::arg("name", kanban_checklist_item->name),
								fmt::arg("eol", constants::END_OF_MARKDOWN_LINE)
							);
						}
					}
				}
				markdown_file += '\n';
			}
			markdown_file += '\n';
		}
#pragma endregion
#pragma region Properties
		std::string properties_string;
		properties_string += "---\n";
		YAML::Node properties;
		properties["Color"] = kanban_board.color;
		properties["Version"] = kanban_board.version;
		properties["Created"] = kanban_board.created.str(constants::time_format);
		properties["Last Modified"] = kanban_board.last_modified.str(constants::time_format);

		// Get the checksum of the file without the properties
		std::vector<unsigned char> hash(picosha2::k_digest_size);
		picosha2::hash256(markdown_file.begin(), markdown_file.end(), hash.begin(), hash.end());
		std::string hex_str = picosha2::bytes_to_hex_string(hash.begin(), hash.end());
		properties["Checksum"] = hex_str;

		std::ostringstream oss;
		oss << properties;
		properties_string += oss.str() + '\n';
		properties_string += "---\n";
		markdown_file = properties_string + markdown_file;
#pragma endregion
		return markdown_file;
	}

	inline void json(KanbanBoard kanban_board, yyjson_mut_doc* doc, yyjson_mut_val* root) {
		if (kanban_board.name.empty()) {
			yyjson_mut_obj_add_strncpy(doc, root, "name", constants::default_board_name.c_str(), constants::default_board_name.length());
		}
		else {
			yyjson_mut_obj_add_strncpy(doc, root, "name", kanban_board.name.c_str(), kanban_board.name.length());
		}
		yyjson_mut_obj_add_str(doc, root, "description", kanban_board.description.empty() ? constants::default_description.c_str() : kanban_board.description.c_str());

		// Properties
		yyjson_mut_val* properties_obj = yyjson_mut_obj(doc);
		yyjson_mut_obj_add_strncpy(doc, properties_obj, "color", kanban_board.color.c_str(), kanban_board.color.length());
		yyjson_mut_obj_add_uint(doc, properties_obj, "version", kanban_board.version);
		yyjson_mut_obj_add_uint(doc, properties_obj, "created", kanban_board.created.timestamp());
		yyjson_mut_obj_add_uint(doc, properties_obj, "last_modified", kanban_board.last_modified.timestamp());
		yyjson_mut_obj_add_strncpy(doc, properties_obj, "checksum", kanban_board.checksum.c_str(), kanban_board.checksum.length());

		yyjson_mut_obj_add_val(doc, root, "properties", properties_obj);

		yyjson_mut_val* labels_arr = yyjson_mut_arr(doc);
		for (const auto& kanban_label : kanban_board.labels) {
			yyjson_mut_val* label_obj = yyjson_mut_obj(doc);
			yyjson_mut_obj_add_strncpy(doc, label_obj, "name", kanban_label->name.c_str(), kanban_label->name.length());
			yyjson_mut_obj_add_strncpy(doc, label_obj, "color", kanban_label->color.c_str(), kanban_label->color.length());

			yyjson_mut_val* tasks_arr = yyjson_mut_arr(doc);
			for (const auto& kanban_task : kanban_label->tasks) {
				yyjson_mut_val* task_obj = yyjson_mut_obj(doc);
				yyjson_mut_obj_add_strncpy(doc, task_obj, "name", kanban_task->name.c_str(), kanban_task->name.length());

				yyjson_mut_arr_add_val(tasks_arr, task_obj);
			}
			yyjson_mut_obj_add_val(doc, label_obj, "tasks", tasks_arr);
			yyjson_mut_arr_add_val(labels_arr, label_obj);
		}
		yyjson_mut_obj_add_val(doc, root, "labels", labels_arr);

		yyjson_mut_val* lists_arr = yyjson_mut_arr(doc);
		for (const auto& kanban_list : kanban_board.list) {
			yyjson_mut_val* list_obj = yyjson_mut_obj(doc);
			yyjson_mut_obj_add_strncpy(doc, list_obj, "name", kanban_list->name.c_str(), kanban_list->name.length());

			yyjson_mut_val* tasks_arr = yyjson_mut_arr(doc);
			for (const auto& kanban_task : kanban_list->tasks) {
				yyjson_mut_val* task_obj = yyjson_mut_obj(doc);
				yyjson_mut_obj_add_strncpy(doc, task_obj, "name", kanban_task->name.c_str(), kanban_task->name.length());
				yyjson_mut_obj_add_bool(doc, task_obj, "checked", kanban_task->checked);
				yyjson_mut_obj_add_uint(doc, task_obj, "counter", kanban_task->counter);

				yyjson_mut_val* desc_arr = yyjson_mut_arr(doc);
				for (const auto& desc_line : kanban_task->description) {
					yyjson_mut_arr_add_strncpy(doc, desc_arr, desc_line.c_str(), desc_line.length());
				}
				yyjson_mut_obj_add_val(doc, task_obj, "description", desc_arr);

				yyjson_mut_val* task_labels_arr = yyjson_mut_arr(doc);
				for (const auto& label : kanban_task->labels) {
					yyjson_mut_val* task_label_obj = yyjson_mut_obj(doc);
					yyjson_mut_obj_add_strncpy(doc, task_label_obj, "name", label->name.c_str(), label->name.length());
					yyjson_mut_obj_add_strncpy(doc, task_label_obj, "color", label->color.c_str(), label->color.length());
					yyjson_mut_arr_add_val(task_labels_arr, task_label_obj);
				}
				yyjson_mut_obj_add_val(doc, task_obj, "labels", task_labels_arr);

				yyjson_mut_val* attachments_arr = yyjson_mut_arr(doc);
				for (const auto& attachment : kanban_task->attachments) {
					yyjson_mut_val* attachment_obj = yyjson_mut_obj(doc);
					yyjson_mut_obj_add_strncpy(doc, attachment_obj, "name", attachment->name.c_str(), attachment->name.length());
					yyjson_mut_obj_add_strncpy(doc, attachment_obj, "url", attachment->url.c_str(), attachment->url.length());
					yyjson_mut_arr_add_val(attachments_arr, attachment_obj);
				}
				yyjson_mut_obj_add_val(doc, task_obj, "attachments", attachments_arr);

				if (!kanban_task->checklist.empty())
				{
					yyjson_mut_val* checklist_arr = yyjson_mut_arr(doc);
					for (const auto& item : kanban_task->checklist) {
						yyjson_mut_val* checklist_item_obj = yyjson_mut_obj(doc);
						yyjson_mut_obj_add_strncpy(doc, checklist_item_obj, "name", item->name.c_str(), item->name.length());
						yyjson_mut_obj_add_bool(doc, checklist_item_obj, "checked", item->checked);
						yyjson_mut_arr_add_val(checklist_arr, checklist_item_obj);
					}
					yyjson_mut_obj_add_val(doc, task_obj, "checklist", checklist_arr);
				}

				yyjson_mut_arr_add_val(tasks_arr, task_obj);
			}
			yyjson_mut_obj_add_val(doc, list_obj, "tasks", tasks_arr);
			yyjson_mut_arr_add_val(lists_arr, list_obj);
		}
		yyjson_mut_obj_add_val(doc, root, "lists", lists_arr);
	}

	inline std::string json_format(KanbanBoard kanban_board) {
		yyjson_mut_doc* doc = yyjson_mut_doc_new(nullptr);
		yyjson_mut_val* root = yyjson_mut_obj(doc);
		yyjson_mut_doc_set_root(doc, root);

		kanban_markdown::json(kanban_board, doc, root);

		const char* json = yyjson_mut_write(doc, 0, nullptr);
		std::string result(json);
		yyjson_mut_doc_free(doc);
		return result;
	}
}