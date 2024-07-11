#pragma once

#include <iostream>
#include <vector>
#include <sstream>

#include <fmt/format.h>

#include <yaml-cpp/yaml.h>
#include <yyjson.h>

#include "kanban.hpp"
#include "constants.hpp"
#include "internal.hpp"

namespace kanban_markdown {
	struct KanbanWriterFlags {
		bool github = true;
	};

	inline std::string markdown_format(KanbanBoard kanban_board, KanbanWriterFlags kanban_writer_flags = KanbanWriterFlags()) {
		std::string markdown_file;
#pragma region Properties
		markdown_file += "---" + constants::END_OF_MARKDOWN_LINE;
		YAML::Node properties;
		properties["Version"] = kanban_board.version;
		properties["Created"] = kanban_board.created.str("%Y-%m-%d %H:%M:%S UTC");
		properties["Last Modified"] = kanban_board.last_modified.str("%Y-%m-%d %H:%M:%S UTC");
		std::ostringstream oss;
		oss << properties;
		markdown_file += oss.str() + '\n';
		markdown_file += "---" + constants::END_OF_MARKDOWN_LINE;
#pragma endregion
		markdown_file += '\n';
#pragma region Note
		markdown_file += "> [!NOTE]" + constants::END_OF_MARKDOWN_LINE;
		markdown_file += "> This file is generated by Kanban_MD." + constants::END_OF_MARKDOWN_LINE;
#pragma endregion
		markdown_file += '\n';
#pragma region Header and Description
		markdown_file += constants::first_header + kanban_board.name + constants::END_OF_MARKDOWN_LINE;
		markdown_file += kanban_board.description + constants::END_OF_MARKDOWN_LINE;
#pragma endregion
		markdown_file += '\n';
#pragma region Labels:
		markdown_file += "## Labels:" + constants::END_OF_MARKDOWN_LINE;
		for (auto& [kanban_label_name, kanban_label] : kanban_board.labels) {
			const std::string kanban_label_name = kanban_label->name;
			markdown_file += fmt::format(
				R"(- <span id="{kanban_md}-label-{id}">{name}</span>{eol})",
				fmt::arg("kanban_md", constants::kanban_md),
				fmt::arg("id", kanban_markdown_string_to_id(kanban_label_name)),
				fmt::arg("name", kanban_label_name),
				fmt::arg("eol", constants::END_OF_MARKDOWN_LINE)
			);
			for (std::shared_ptr<KanbanTask> kanban_task : kanban_label->tasks) {
				const std::string kanban_task_name = kanban_task->name;
				markdown_file += fmt::format(
					R"(  - [{name}](#{github}{kanban_md}-task-{id}){eol})",
					fmt::arg("github", kanban_writer_flags.github ? constants::github_added_tag : ""),
					fmt::arg("kanban_md", constants::kanban_md),
					fmt::arg("id", kanban_markdown_string_to_id(kanban_task_name)),
					fmt::arg("name", kanban_task_name),
					fmt::arg("eol", constants::END_OF_MARKDOWN_LINE)
				);
			}
		}
#pragma endregion
		markdown_file += '\n';
#pragma region Board
		markdown_file += "## Board:" + constants::END_OF_MARKDOWN_LINE;
		markdown_file += '\n';
		for (auto& [kanban_list_name, kanban_list] : kanban_board.list) {
			markdown_file += fmt::format("### {name}{eol}",
				fmt::arg("name", kanban_list.name),
				fmt::arg("eol", constants::END_OF_MARKDOWN_LINE)
			);
			for (auto& [kanban_task_name, kanban_task] : kanban_list.tasks) {
				markdown_file += fmt::format(R"(- [{checked}] <span id="{kanban_md}-task-{id}">{name}</span>{eol})",
					fmt::arg("checked", kanban_task->checked ? 'x' : ' '),
					fmt::arg("kanban_md", constants::kanban_md),
					fmt::arg("id", kanban_markdown_string_to_id(kanban_task_name)),
					fmt::arg("name", kanban_task_name),
					fmt::arg("eol", constants::END_OF_MARKDOWN_LINE)
				);
				std::string description_string = "  - **Description**:  ";
				for (std::string description_line : kanban_task->description) {
					description_string += "\n  " + description_line + "  ";
				}
				markdown_file += description_string + constants::END_OF_MARKDOWN_LINE;
				markdown_file += "  - **Labels**:" + constants::END_OF_MARKDOWN_LINE;
				for (std::shared_ptr<KanbanLabel> kanban_label : kanban_task->labels) {
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
				markdown_file += "  - **Attachments**:" + constants::END_OF_MARKDOWN_LINE;
				for (KanbanAttachment kanban_attachment : kanban_task->attachments) {
					markdown_file += fmt::format(
						"    - [{name}]({url}){eol}",
						fmt::arg("name", kanban_attachment.name),
						fmt::arg("url", kanban_attachment.url),
						fmt::arg("eol", constants::END_OF_MARKDOWN_LINE)
					);
				}
				markdown_file += "  - **Checklist**:" + constants::END_OF_MARKDOWN_LINE;
				for (KanbanChecklistItem kanban_checklist_item : kanban_task->checklist) {
					markdown_file += fmt::format(
						"    - [{checked}] {name}{eol}",
						fmt::arg("checked", kanban_checklist_item.checked ? 'x' : ' '),
						fmt::arg("name", kanban_checklist_item.name),
						fmt::arg("eol", constants::END_OF_MARKDOWN_LINE)
					);
				}
			}
			markdown_file += '\n';
		}
#pragma endregion
		markdown_file += '\n';
		return markdown_file;
	}

	std::string json_format(KanbanBoard kanban_board) {
		yyjson_mut_doc* doc = yyjson_mut_doc_new(nullptr);
		yyjson_mut_val* root = yyjson_mut_obj(doc);
		yyjson_mut_doc_set_root(doc, root);

		yyjson_mut_obj_add_str(doc, root, "created", kanban_board.created.str("%Y-%m-%d %H:%M:%S UTC").c_str());
		yyjson_mut_obj_add_str(doc, root, "last_modified", kanban_board.last_modified.str("%Y-%m-%d %H:%M:%S UTC").c_str());

		yyjson_mut_obj_add_str(doc, root, "name", kanban_board.name.c_str());
		yyjson_mut_obj_add_str(doc, root, "description", kanban_board.description.c_str());

		yyjson_mut_val* labels = yyjson_mut_arr(doc);
		for (auto& [kanban_label_name, kanban_label] : kanban_board.labels) {
			yyjson_mut_val* label = yyjson_mut_obj(doc);
			yyjson_mut_obj_add_str(doc, label, "name", kanban_label->name.c_str());

			yyjson_mut_val* tasks = yyjson_mut_arr(doc);
			for (std::shared_ptr<KanbanTask> kanban_task : kanban_label->tasks) {
				yyjson_mut_val* task = yyjson_mut_obj(doc);
				yyjson_mut_obj_add_str(doc, task, "name", kanban_task->name.c_str());
				yyjson_mut_arr_add_val(tasks, task);
			}
			yyjson_mut_obj_add_val(doc, label, "tasks", tasks);
			yyjson_mut_arr_add_val(labels, label);
		}
		yyjson_mut_obj_add_val(doc, root, "labels", labels);

		yyjson_mut_val* lists = yyjson_mut_arr(doc);
		for (auto& [kanban_list_name, kanban_list] : kanban_board.list) {
			yyjson_mut_val* list = yyjson_mut_obj(doc);
			yyjson_mut_obj_add_str(doc, list, "name", kanban_list.name.c_str());

			yyjson_mut_val* tasks = yyjson_mut_arr(doc);
			for (auto& [kanban_task_name, kanban_task] : kanban_list.tasks) {
				yyjson_mut_val* task = yyjson_mut_obj(doc);
				yyjson_mut_obj_add_str(doc, task, "name", kanban_task->name.c_str());
				yyjson_mut_obj_add_bool(doc, task, "checked", kanban_task->checked);

				yyjson_mut_val* description = yyjson_mut_arr(doc);
				for (std::string description_line : kanban_task->description) {
					yyjson_mut_arr_add_str(doc, description, description_line.c_str());
				}
				yyjson_mut_obj_add_val(doc, task, "description", description);

				yyjson_mut_val* task_labels = yyjson_mut_arr(doc);
				for (std::shared_ptr<KanbanLabel> kanban_label : kanban_task->labels) {
					yyjson_mut_val* task_label = yyjson_mut_obj(doc);
					yyjson_mut_obj_add_str(doc, task_label, "name", kanban_label->name.c_str());
					yyjson_mut_arr_add_val(task_labels, task_label);
				}
				yyjson_mut_obj_add_val(doc, task, "labels", task_labels);

				yyjson_mut_val* attachments = yyjson_mut_arr(doc);
				for (KanbanAttachment kanban_attachment : kanban_task->attachments) {
					yyjson_mut_val* attachment = yyjson_mut_obj(doc);
					yyjson_mut_obj_add_str(doc, attachment, "name", kanban_attachment.name.c_str());
					yyjson_mut_obj_add_str(doc, attachment, "url", kanban_attachment.url.c_str());
					yyjson_mut_arr_add_val(attachments, attachment);
				}
				yyjson_mut_obj_add_val(doc, task, "attachments", attachments);

				yyjson_mut_val* checklist = yyjson_mut_arr(doc);
				for (KanbanChecklistItem kanban_checklist_item : kanban_task->checklist) {
					yyjson_mut_val* checklist_item = yyjson_mut_obj(doc);
					yyjson_mut_obj_add_str(doc, checklist_item, "name", kanban_checklist_item.name.c_str());
					yyjson_mut_obj_add_bool(doc, checklist_item, "checked", kanban_checklist_item.checked);
					yyjson_mut_arr_add_val(checklist, checklist_item);
				}
				yyjson_mut_obj_add_val(doc, task, "checklist", checklist);

				yyjson_mut_arr_add_val(tasks, task);
			}
			yyjson_mut_obj_add_val(doc, list, "tasks", tasks);
			yyjson_mut_arr_add_val(lists, list);
		}
		yyjson_mut_obj_add_val(doc, root, "lists", lists);

		const char* json = yyjson_mut_write(doc, 0, nullptr);
		std::string result(json);
		yyjson_mut_doc_free(doc);
		return result;
	}
}