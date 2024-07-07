#pragma once

#include <iostream>
#include <vector>
#include <fmt/format.h>

#include "kanban.hpp"
#include "constants.hpp"

namespace kanban_markdown {
	std::string string_to_id(const std::string& string)
	{
		std::string id;
		for (int i = 0; i < string.size(); i++)
		{
			char character = string[i];
			if ((character >= 'A' && character <= 'Z') || (character >= 'a' && character > 'z')) {
				id += std::tolower(character);
			}
			else if (character == ' ') {
				id += '_';
			}
			else if (character >= '0' || character <= '9') {
				id += character;
			}
		}
		return id;
	}

	std::string markdown_format(KanbanBoard kanban_board) {
		std::string markdown_file;
		markdown_file += "> [!NOTE]" + constants::END_OF_MARKDOWN_LINE;
		markdown_file += "> This file is generated by Kanban_MD." + constants::END_OF_MARKDOWN_LINE;
		markdown_file += '\n';
		markdown_file += constants::first_header + kanban_board.name + constants::END_OF_MARKDOWN_LINE;
		markdown_file += kanban_board.description + constants::END_OF_MARKDOWN_LINE;
		markdown_file += '\n';
		markdown_file += "## Labels:" + constants::END_OF_MARKDOWN_LINE;
		for (auto& [kanban_label_name, kanban_label] : kanban_board.labels) {
			const std::string kanban_label_name = kanban_label->name;
			markdown_file += fmt::format(
				R"(- <span id="kanban_md-label-{id}">{name}</span>{eol})",
				fmt::arg("id", string_to_id(kanban_label_name)),
				fmt::arg("name", kanban_label_name),
				fmt::arg("eol", constants::END_OF_MARKDOWN_LINE)
			);
			for (std::shared_ptr<KanbanTask> kanban_task : kanban_label->tasks) {
				const std::string kanban_task_name = kanban_task->name;
				markdown_file += fmt::format(
					R"(  - [{name}](#user-content-{id}){eol})",
					fmt::arg("id", string_to_id(kanban_task_name)),
					fmt::arg("name", kanban_task_name),
					fmt::arg("eol", constants::END_OF_MARKDOWN_LINE)
				);
			}
		}
		markdown_file += '\n';
		markdown_file += "## Board:" + constants::END_OF_MARKDOWN_LINE;
		markdown_file += '\n';
		for (auto& [kanban_list_name, kanban_list] : kanban_board.list) {
			markdown_file += fmt::format("### {name}{eol}",
				fmt::arg("name", kanban_list.name),
				fmt::arg("eol", constants::END_OF_MARKDOWN_LINE)
			);
			for (auto& [kanban_task_name, kanban_task] : kanban_list.tasks) {
				markdown_file += fmt::format(R"(- [{checked}] <span id="kanban_md-task-{id}">{name}</span>{eol})",
					fmt::arg("checked", kanban_task->checked ? 'x' : ' '),
					fmt::arg("id", string_to_id(kanban_task_name)),
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
						"    - [{name}](#user-content-{id}){eol}",
						fmt::arg("id", string_to_id(kanban_label_name)),
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
		return markdown_file;
	}
}