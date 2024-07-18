#pragma once

#include <iostream>
#include <vector>
#include <algorithm> 
#include <cctype>
#include <locale>

#include <fmt/format.h>
#include <tl/expected.hpp>

#include <chrono>
#include <asap/asap.h>

#include <tsl/ordered_map.h>

#include <md4c.h>
#include <yaml-cpp/yaml.h>

#include "kanban.hpp"
#include "constants.hpp"

#include "reader/internal.hpp"
#include "internal.hpp"

namespace kanban_markdown {
	namespace internal {
		struct KanbanParser {
			std::vector<unsigned int> previous_headers;
			unsigned int header_level = 0;
			KanbanState state = KanbanState::None;

			bool read_properties = false;

			std::string color;
			asap::datetime created;
			asap::datetime last_modified;
			unsigned int version = 0;
			std::string checksum;

			std::string kanban_board_name;
			bool read_kanban_board_name = false;

			std::string kanban_board_description;
			bool read_kanban_board_description = false;

			LabelSection label_section;
			BoardListSection board_section;

			unsigned int list_item_level = 0;
			unsigned int sub_list_item_count = 0;

			bool currently_reading_link = false;
		};

		int enter_block_callback(MD_BLOCKTYPE type, void* detail, void* userdata) {
			KanbanParser* kanban_parser = static_cast<KanbanParser*>(userdata);
			switch (type) {
			case MD_BLOCK_H:
			{
				MD_BLOCK_H_DETAIL* header_detail = static_cast<MD_BLOCK_H_DETAIL*>(detail);
				kanban_parser->previous_headers.push_back(kanban_parser->header_level);
				kanban_parser->header_level = header_detail->level;
				break;
			}
			case MD_BLOCK_LI:
			{
				if (kanban_parser->list_item_level == 1) {
					kanban_parser->sub_list_item_count++;
				}
				kanban_parser->list_item_level++;
				break;
			}
			default:
			{
				break;
			}
			}
			return 0;
		}

		int leave_block_callback(MD_BLOCKTYPE type, void* detail, void* userdata) {
			KanbanParser* kanban_parser = static_cast<KanbanParser*>(userdata);
			switch (type) {
			case MD_BLOCK_H:
			{
				MD_BLOCK_H_DETAIL* header_detail = static_cast<MD_BLOCK_H_DETAIL*>(detail);
				kanban_parser->header_level = kanban_parser->previous_headers.back();
				kanban_parser->previous_headers.pop_back();
				break;
			}
			case MD_BLOCK_UL:
			{
				if (kanban_parser->list_item_level == 0) {
					kanban_parser->state = KanbanState::Board;
				}
				break;
			}
			case MD_BLOCK_LI:
			{
				if (kanban_parser->list_item_level == 1) {
					kanban_parser->sub_list_item_count = 0;
				}
				kanban_parser->list_item_level--;
				break;
			}
			default:
			{
				break;
			}
			}
			return 0;
		}

		int enter_span_callback(MD_SPANTYPE type, void* detail, void* userdata) {
			KanbanParser* kanban_parser = static_cast<KanbanParser*>(userdata);
			switch (type) {
			case MD_SPAN_A:
			{
				kanban_parser->currently_reading_link = true;
				MD_SPAN_A_DETAIL* a_detail = static_cast<MD_SPAN_A_DETAIL*>(detail);

				if (kanban_parser->state == KanbanState::Board) {
					if (kanban_parser->board_section.task_read_state == TaskReadState::Attachments) {
						BoardSection* current_board = kanban_parser->board_section.current_board;
						if (current_board == nullptr) {
							std::cerr << "Current board is null." << '\n';
							return 0;
						}
						TaskDetail& task_detail = current_board->task_details[current_board->current_task_name];
						KanbanAttachment attachment;
						attachment.url = std::string(a_detail->href.text, a_detail->href.size);
						task_detail.attachments.push_back(attachment);
						task_detail.currentAttachment = &task_detail.attachments.back();
					}
				}
				break;
			}
			default:
			{
				break;
			}
			}
			return 0;
		}

		int leave_span_callback(MD_SPANTYPE type, void* detail, void* userdata) {
			KanbanParser* kanban_parser = static_cast<KanbanParser*>(userdata);
			switch (type) {
			case MD_SPAN_A:
			{
				kanban_parser->currently_reading_link = false;
				break;
			}
			default:
			{
				break;
			}
			}
			return 0;
		}

		void parseHeader(KanbanParser* kanban_parser, std::string text_content) {
			switch (kanban_parser->header_level) {
			case 0:
			{
				if (!kanban_parser->read_kanban_board_description && kanban_parser->read_kanban_board_name && kanban_parser->kanban_board_description.empty())
				{
					if (text_content == constants::default_description) {
						text_content = "";
					}
					kanban_parser->kanban_board_description = text_content;
					kanban_parser->read_kanban_board_description = true;
				}
				break;
			}
			case 1:
			{
				if (!kanban_parser->read_kanban_board_name)
				{
					if (text_content == constants::default_board_name) {
						text_content = "";
					}
					kanban_parser->kanban_board_name = text_content;
					kanban_parser->read_kanban_board_name = true;
				}
				break;
			}
			case 2:
			{
				if (text_content == "Labels:") {
					kanban_parser->state = KanbanState::Labels;
				}
				else if (text_content == "Board:") {
					kanban_parser->state = KanbanState::Board;
				}
				break;
			}
			default:
			{
				break;
			}
			}
		}

		void parseLabelSection(KanbanParser* kanban_parser, const std::string& text_content) {
			switch (kanban_parser->list_item_level) {
			case 1:
			{
				kanban_parser->label_section.label_details[text_content] = LabelDetail();
				kanban_parser->label_section.current_label_name = text_content;
				break;
			}
			case 2:
			{
				kanban_parser->label_section.label_details[kanban_parser->label_section.current_label_name].list_items.push_back(text_content);
				break;
			}
			}
		}

		void parseTaskDetails(KanbanParser* kanban_parser, std::string text_content) {
			bool is_task_property = false;
			switch (kanban_markdown_hash(text_content)) {
			case kanban_markdown_hash("Description"):
				kanban_parser->board_section.task_read_state = TaskReadState::Description;
				is_task_property = true;
				break;
			case kanban_markdown_hash("Labels"):
				kanban_parser->board_section.task_read_state = TaskReadState::Labels;
				is_task_property = true;
				break;
			case kanban_markdown_hash("Attachments"):
				kanban_parser->board_section.task_read_state = TaskReadState::Attachments;
				is_task_property = true;
				break;
			case kanban_markdown_hash("Checklist"):
				kanban_parser->board_section.task_read_state = TaskReadState::Checklist;
				is_task_property = true;
				break;
			}
			if (!is_task_property && kanban_parser->board_section.task_read_state == TaskReadState::Description) {
				BoardSection* current_board = kanban_parser->board_section.current_board;
				if (current_board == nullptr) {
					std::cerr << "Current board is null." << '\n';
					return;
				}
				TaskDetail& tail_detail = current_board->task_details[current_board->current_task_name];
				if (tail_detail.description.empty()) {
					text_content = kanban_markdown_ltrim(text_content);
					if (text_content.find(":") == 0) // TODO: Fix this
					{
						text_content = text_content.substr(1);
					}
					text_content = kanban_markdown_trim(text_content);
					if (!text_content.empty()) {
						tail_detail.description.push_back(text_content);
					}
				}
				else {
					text_content = kanban_markdown_trim(text_content);
					if (!text_content.empty()) {
						tail_detail.description.push_back(text_content);
					}
				}
			}
		}

		void parseTaskDetailList(KanbanParser* kanban_parser, const std::string& text_content) {
			switch (kanban_parser->board_section.task_read_state) {
			case TaskReadState::Labels:
			{
				BoardSection* current_board = kanban_parser->board_section.current_board;
				if (current_board == nullptr) {
					std::cerr << "Current board is null." << '\n';
					return;
				}
				current_board->task_details[current_board->current_task_name].labels.push_back(text_content);
				break;
			}
			case TaskReadState::Attachments:
			{
				if (kanban_parser->currently_reading_link) {
					BoardSection* current_board = kanban_parser->board_section.current_board;
					if (current_board == nullptr) {
						std::cerr << "Current board is null." << '\n';
						return;
					}
					KanbanAttachment* current_attachment = current_board->task_details[current_board->current_task_name].currentAttachment;
					current_attachment->name = text_content;
				}
				break;
			}
			case TaskReadState::Checklist:
			{
				bool is_checked = false;
				std::string checkbox_characters = text_content.substr(0, 4);
				if (checkbox_characters == "[ ] ") {
					is_checked = false;
				}
				else if (checkbox_characters == "[x] ") {
					is_checked = true;
				}
				else {
					std::cerr << "Invalid checklist item: " << text_content << '\n';
				}
				KanbanChecklistItem checkbox;
				checkbox.checked = is_checked;
				checkbox.name = text_content.substr(4);
				BoardSection* current_board = kanban_parser->board_section.current_board;
				if (current_board == nullptr) {
					std::cerr << "Current board is null." << '\n';
					return;
				}
				current_board->task_details[current_board->current_task_name].checklist.push_back(checkbox);
				break;
			}
			}
		}

		void parseBoardSection(KanbanParser* kanban_parser, const std::string& text_content) {
			if (text_content == "[ ] " || text_content == "[x] ") {
				bool is_checked = false;
				std::string checkbox_characters = text_content.substr(0, 4);
				if (checkbox_characters == "[ ] ") {
					is_checked = false;
				}
				else if (checkbox_characters == "[x] ") {
					is_checked = true;
				}
				else {
					std::cerr << "Invalid checklist item: " << text_content << '\n';
				}
				BoardSection* current_board = kanban_parser->board_section.current_board;
				if (current_board == nullptr) {
					std::cerr << "Current board is null." << '\n';
					return;
				}
				current_board->current_stored_checked = is_checked;
				return;
			}
			switch (kanban_parser->list_item_level) {
			case 0:
			{
				// Check if it is not a main header
				if (kanban_parser->header_level <= 2) {
					return;
				}
				BoardSection board_section;
				board_section.name = text_content;
				kanban_parser->board_section.boards.push_back(board_section);
				kanban_parser->board_section.current_board = &kanban_parser->board_section.boards.back();
				break;
			}
			case 1:
			{
				BoardSection* current_board = kanban_parser->board_section.current_board;
				if (current_board == nullptr) {
					std::cerr << "Current board is null." << '\n';
					return;
				}
				current_board->current_task_name = text_content;
				TaskDetail task_detail;
				task_detail.name = text_content;
				task_detail.checked = current_board->current_stored_checked;
				current_board->task_details[current_board->current_task_name] = task_detail;
				break;
			}
			case 2:
			{
				parseTaskDetails(kanban_parser, text_content);
				break;
			}
			case 3:
			{
				parseTaskDetailList(kanban_parser, text_content);
				break;
			}
			}
		}

		void parseSection(KanbanParser* kanban_parser, const std::string& text_content) {
			// Check if it is not a main header
			if (kanban_parser->header_level == 1 || kanban_parser->header_level == 2) {
				parseHeader(kanban_parser, text_content);
				return;
			}
			switch (kanban_parser->state) {
			case KanbanState::None:
			{
				parseHeader(kanban_parser, text_content);
				break;
			}
			case KanbanState::Labels:
			{
				parseLabelSection(kanban_parser, text_content);
				break;
			}
			case KanbanState::Board:
			{
				parseBoardSection(kanban_parser, text_content);
				break;
			}
			default:
			{
				break;
			}
			}
		}

		int text_callback(MD_TEXTTYPE type, const MD_CHAR* text, MD_SIZE size, void* userdata) {
			KanbanParser* kanban_parser = static_cast<KanbanParser*>(userdata);
			std::string text_content(text, size);
			switch (type) {
			case MD_TEXT_NORMAL:
			{
				parseSection(kanban_parser, text_content);
				break;
			}
			default:
			{
				break;
			}
			}
			return 0;
		}

		void debug(const char* msg, void* userdata) {
			KanbanBoard* kanban_board = static_cast<KanbanBoard*>(userdata);
			std::cout << msg << '\n';
		}
	}
	using namespace internal;

	inline KanbanBoard createKanbanBoard(KanbanParser& kanban_parser) {
		KanbanBoard kanban_board;
		kanban_board.color = kanban_parser.color;
		kanban_board.created = kanban_parser.created;
		kanban_board.last_modified = kanban_parser.last_modified;
		kanban_board.version = kanban_parser.version;
		kanban_board.checksum = kanban_parser.checksum;

		kanban_board.name = kanban_parser.kanban_board_name;
		kanban_board.description = kanban_parser.kanban_board_description;

		for (auto& [label_name, label_detail] : kanban_parser.label_section.label_details) {
			std::shared_ptr<KanbanLabel> kanban_label = std::make_shared<KanbanLabel>();
			kanban_label->name = label_name;
			kanban_board.labels.push_back(kanban_label);
		}

		for (BoardSection board_section : kanban_parser.board_section.boards) {
			std::shared_ptr<KanbanList> kanban_list = std::make_shared<KanbanList>();
			kanban_list->name = board_section.name;
			for (auto& [task_name, task_detail] : board_section.task_details) {
				std::shared_ptr<KanbanTask> kanban_task = std::make_shared<KanbanTask>();
				kanban_task->checked = task_detail.checked;
				kanban_task->name = task_name;
				kanban_task->description = task_detail.description;
				for (const std::string& label : task_detail.labels) {
					auto it = std::find_if(kanban_board.labels.begin(), kanban_board.labels.end(), [&label](const std::shared_ptr<KanbanLabel>& x) { return x->name == label; });
					std::shared_ptr<KanbanLabel> kanban_label;
					if (it == kanban_board.labels.end()) {
						kanban_label = std::make_shared<KanbanLabel>();
						kanban_label->name = label;
						kanban_board.labels.push_back(kanban_label);
					}
					else {
						kanban_label = *it;
					}
					kanban_label->tasks.push_back(kanban_task);
					kanban_task->labels.push_back(kanban_label);
				}
				for (const KanbanAttachment& attachment : task_detail.attachments) {
					kanban_task->attachments.push_back(std::make_shared<KanbanAttachment>(attachment));
				}
				for (const KanbanChecklistItem& checkbox : task_detail.checklist) {
					kanban_task->checklist.push_back(std::make_shared<KanbanChecklistItem>(checkbox));
				}
				kanban_list->tasks.push_back(kanban_task);
			}
			kanban_board.list.push_back(kanban_list);
		}
		return kanban_board;
	}

	inline tl::expected<KanbanBoard, std::string> parse(std::string md_string) {
		KanbanParser kanban_parser;

		bool has_properties = md_string.substr(0, 4) == "---\n";
		if (has_properties) {
			std::size_t end_of_properties = md_string.find("---\n", 4);
			if (end_of_properties == std::string::npos) {
				return tl::make_unexpected("Invalid Markdown file. Properties are not closed.");
			}
			std::string properties = md_string.substr(3, end_of_properties - 4);
			YAML::Node config = YAML::Load(properties);
			const std::string color = config["Color"].as<std::string>();
			if (color.empty()) {
				return tl::make_unexpected("Invalid Markdown file. [Color] property is empty.");
			}

			const std::string created = config["Created"].as<std::string>();
			if (created.empty()) {
				return tl::make_unexpected("Invalid Markdown file. [Created] property is empty.");
			}
			const int version = config["Version"].as<unsigned int>();

			const std::string last_modified = config["Last Modified"].as<std::string>();
			if (last_modified.empty()) {
				return tl::make_unexpected("Invalid Markdown file. [Last Modified] property is empty.");
			}

			const std::string checksum = config["Checksum"].as<std::string>();
			if (checksum.empty()) {
				return tl::make_unexpected("Invalid Markdown file. [Checksum] property is empty.");
			}

			asap::datetime created_datetime(created, "%Y-%m-%d %H:%M:%S UTC");
			if (created_datetime.timestamp() == 0) {
				return tl::make_unexpected("Invalid Markdown file. [Created] property has invalid seconds.");
			}

			asap::datetime last_modified_datetime(last_modified, "%Y-%m-%d %H:%M:%S UTC");
			if (last_modified_datetime.timestamp() == 0) {
				return tl::make_unexpected("Invalid Markdown file. [Last Modified] property has invalid seconds.");
			}

			kanban_parser.color = color;
			kanban_parser.created = created_datetime;
			kanban_parser.last_modified = last_modified_datetime;

			kanban_parser.version = version;

			kanban_parser.checksum = checksum;

			md_string = md_string.substr(end_of_properties + 3);
		}
		else {
			auto now = kanban_markdown::internal::now_utc();
			kanban_parser.color = constants::default_color;
			kanban_parser.created = now;
			kanban_parser.last_modified = now;
			kanban_parser.version = 0;
		}
		kanban_parser.read_properties = true;

		MD_PARSER parser;
		parser.abi_version = 0;
		parser.enter_block = enter_block_callback;
		parser.leave_block = leave_block_callback;
		parser.enter_span = enter_span_callback;
		parser.leave_span = leave_span_callback;
		parser.text = text_callback;
		parser.flags = 0;
		parser.syntax = nullptr;
		parser.debug_log = debug;

		int result = md_parse(md_string.c_str(), md_string.size(), &parser, &kanban_parser);
		if (result != 0) {
			std::cerr << "Error parsing Markdown text." << std::endl;
		}

		KanbanBoard kanban_board = createKanbanBoard(kanban_parser);
		return kanban_board;
	}
}