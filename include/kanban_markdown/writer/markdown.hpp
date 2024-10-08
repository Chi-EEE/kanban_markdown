#pragma once

#include <iostream>
#include <vector>
#include <sstream>
#include <string>

#include <picosha2.h>
#include <fmt/format.h>
#include <yaml-cpp/yaml.h>

#include <kanban_markdown/kanban_board.hpp>
#include <kanban_markdown/constants.hpp>
#include <kanban_markdown/internal.hpp>

namespace kanban_markdown::writer::markdown {
	struct Flags {
		bool github = true;
	};

	static inline std::string format_str(KanbanBoard kanban_board, Flags kanban_writer_flags = Flags()) {
		std::string markdown_file;
		markdown_file += "\r\n";
#pragma region Note
		markdown_file += "> [!NOTE]" + constants::END_OF_MARKDOWN_LINE;
		markdown_file += "> This file is generated by Kanban_MD." + constants::END_OF_MARKDOWN_LINE;
#pragma endregion
		markdown_file += "\r\n";
#pragma region Header and Description
		markdown_file += "# " + (!kanban_board.name.empty() ? kanban_board.name : constants::default_board_name) + constants::END_OF_MARKDOWN_LINE;
		markdown_file += (!kanban_board.description.empty() ? kanban_board.description : constants::default_description) + constants::END_OF_MARKDOWN_LINE;
#pragma endregion
		markdown_file += "\r\n";
#pragma region Labels:
		if (!kanban_board.labels.empty()) {
			markdown_file += "## Labels:" + constants::END_OF_MARKDOWN_LINE;
			for (auto& kanban_label : kanban_board.labels) {
				const std::string kanban_label_name = kanban_label->name;
				markdown_file += fmt::format(
					R"(- <span id="{kanban_md}-label-{id}" data-color="{color}">{name}</span>{eol})",
					fmt::arg("kanban_md", constants::kanban_md),
					fmt::arg("id", internal::string_to_id(kanban_label_name)),
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
						fmt::arg("id", internal::string_to_id(kanban_task_name)),
						fmt::arg("counter", kanban_task->counter),
						fmt::arg("name", kanban_task_name),
						fmt::arg("eol", constants::END_OF_MARKDOWN_LINE)
					);
				}
			}
			markdown_file += "\r\n";
		}
#pragma endregion
#pragma region Board
		if (!kanban_board.list.empty()) {
			markdown_file += "## Board:" + constants::END_OF_MARKDOWN_LINE;
			markdown_file += "\r\n";
			for (auto& kanban_list : kanban_board.list) {
				markdown_file += fmt::format(R"(### <span data-checked="{checked}" data-counter="{counter}">{name}</span>{eol})",
					fmt::arg("checked", kanban_list->checked),
					fmt::arg("counter", kanban_list->counter),
					fmt::arg("name", kanban_list->name),
					fmt::arg("eol", constants::END_OF_MARKDOWN_LINE)
				);
				for (auto& kanban_task : kanban_list->tasks) {
					markdown_file += fmt::format(R"(- [{checked}] <span id="{kanban_md}-task-{id}-{counter}" data-counter="{counter}">{name}</span>{eol})",
						fmt::arg("checked", kanban_task->checked ? 'x' : ' '),
						fmt::arg("kanban_md", constants::kanban_md),
						fmt::arg("id", internal::string_to_id(kanban_task->name)),
						fmt::arg("counter", kanban_task->counter),
						fmt::arg("name", kanban_task->name),
						fmt::arg("eol", constants::END_OF_MARKDOWN_LINE)
					);
					if (!kanban_task->description.empty()) {
						std::string description_string = "  - **Description**:  ";
						for (std::string description_line : kanban_task->description) {
							description_string += "\r\n  " + description_line + "  ";
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
								fmt::arg("id", internal::string_to_id(kanban_label_name)),
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
				markdown_file += "\r\n";
			}
			markdown_file += "\r\n";
		}
#pragma endregion
#pragma region Properties
		std::string properties_string;
		properties_string += "---\r\n";
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
		properties_string += oss.str() + "\r\n";
		properties_string += "---\r\n";
		markdown_file = properties_string + markdown_file;
#pragma endregion
		return markdown_file;
	}
}
