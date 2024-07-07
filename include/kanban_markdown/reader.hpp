#pragma once

#include <iostream>
#include <vector>
#include <fmt/format.h>
#include <tl/expected.hpp>
#include <algorithm> 
#include <cctype>
#include <locale>

#include <tsl/ordered_map.h>

#include <re2/re2.h>
#include <md4c.h>
#include <magic_enum_all.hpp>

#include "kanban.hpp"
#include "constants.hpp"

namespace kanban_markdown {
	static constexpr uint32_t hash(const std::string_view s) noexcept
	{
		uint32_t hash = 5381;

		for (const char* c = s.data(); c < s.data() + s.size(); ++c)
			hash = ((hash << 5) + hash) + (unsigned char)*c;

		return hash;
	}

	enum class KanbanState {
		None = 0,
		Labels,
		Board,
	};

	enum class TaskReadState {
		None = 0,
		Description,
		Labels,
		Attachments,
		Checklist,
	};

	struct LabelDetail {
		std::string name;
		std::vector<std::string> list_items;
	};

	struct LabelSection {
		std::string current_label_name;
		tsl::ordered_map<std::string, LabelDetail> label_details;
	};

	struct Attachment {
		std::string name;
		std::string url;
	};

	struct TaskDetail {
		std::string name;
		std::vector<std::string> description;
		std::vector<std::string> labels;
		std::vector<Attachment> attachments;
		std::vector<KanbanChecklistItem> checklist;

		Attachment* currentAttachment;
	};

	struct BoardSection {
		std::string name;
		std::string current_task_name;
		tsl::ordered_map<std::string, TaskDetail> task_details;
	};

	struct BoardListSection {
		std::vector<BoardSection> boards;
		BoardSection* current_board = nullptr;
		TaskReadState task_read_state = TaskReadState::None;
	};

	struct KanbanParser {
		unsigned int header_level = 0;
		KanbanState state = KanbanState::None;

		std::string kanban_board_name;
		bool read_kanban_board_name = false;

		std::string kanban_board_description;

		LabelSection label_section;
		BoardListSection board_section;

		unsigned int list_item_level = 0;
		unsigned int sub_list_item_count = 0; //

		bool currently_reading_link = false;
	};

	const char* ws = " \t\n\r\f\v";

	// trim from end of string (right)
	inline std::string& rtrim(std::string& s, const char* t = ws)
	{
		s.erase(s.find_last_not_of(t) + 1);
		return s;
	}

	// trim from beginning of string (left)
	inline std::string& ltrim(std::string& s, const char* t = ws)
	{
		s.erase(0, s.find_first_not_of(t));
		return s;
	}

	// trim from both ends of string (right then left)
	inline std::string& trim(std::string& s, const char* t = ws)
	{
		return ltrim(rtrim(s, t), t);
	}

	int enter_block_callback(MD_BLOCKTYPE type, void* detail, void* userdata) {
		KanbanParser* kanban_parser = static_cast<KanbanParser*>(userdata);
		switch (type) {
		case MD_BLOCK_DOC:
		{
			//std::cout << "[Document Start]\n";
			break;
		}
		case MD_BLOCK_H:
		{
			MD_BLOCK_H_DETAIL* header_detail = static_cast<MD_BLOCK_H_DETAIL*>(detail);
			kanban_parser->header_level = header_detail->level;
			std::cout << "[Header - Open] " << header_detail->level << "\n";
			break;
		}
		case MD_BLOCK_P:
		{
			//std::cout << "[Paragraph]\n";
			break;
		}
		case MD_BLOCK_UL:
		{
			MD_BLOCK_UL_DETAIL* ul_detail = static_cast<MD_BLOCK_UL_DETAIL*>(detail);
			//std::cout << "[Unordered List]\n";
			break;
		}
		case MD_BLOCK_OL:
		{
			//std::cout << "[Ordered List]\n";
			break;
		}
		case MD_BLOCK_LI:
		{
			//std::cout << "[List Item - Open]\n";
			if (kanban_parser->list_item_level == 1) {
				kanban_parser->sub_list_item_count++;
			}
			kanban_parser->list_item_level++;
			break;
		}
		default:
		{
			//std::cout << "Unknown Type: " << type << "\n";
			break;
		}
		}
		return 0;
	}


	// Callback for leaving block elements
	int leave_block_callback(MD_BLOCKTYPE type, void* detail, void* userdata) {
		KanbanParser* kanban_parser = static_cast<KanbanParser*>(userdata);
		switch (type) {
		case MD_BLOCK_DOC:
		{
			//std::cout << "[Document End]\n";
			break;
		}
		case MD_BLOCK_H:
		{
			MD_BLOCK_H_DETAIL* header_detail = static_cast<MD_BLOCK_H_DETAIL*>(detail);
			kanban_parser->header_level = 0;
			break;
		}
		case MD_BLOCK_P:
		{
			//std::cout << "[Paragraph]\n";
			break;
		}
		case MD_BLOCK_UL:
		{
			//std::cout << "[Unordered List]\n";
			//std::cout << "[List Item - Close]\n";
			if (kanban_parser->list_item_level == 0) {
				kanban_parser->state = KanbanState::None;
			}
			break;
		}
		case MD_BLOCK_OL:
		{
			//std::cout << "[Ordered List]\n";
			break;
		}
		case MD_BLOCK_LI:
		{
			//std::cout << "[List Item - Close]\n";
			if (kanban_parser->list_item_level == 1) {
				kanban_parser->sub_list_item_count = 0;
			}
			kanban_parser->list_item_level--;
			break;
		}
		default:
		{
			//std::cout << "Unknown Type: " << type << "\n";
			break;
		}
		}
		return 0;
	}

	// Callback for processing span elements
	int enter_span_callback(MD_SPANTYPE type, void* detail, void* userdata) {
		KanbanParser* kanban_parser = static_cast<KanbanParser*>(userdata);
		switch (type) {
		case MD_SPAN_A:
		{
			kanban_parser->currently_reading_link = true;
			MD_SPAN_A_DETAIL* a_detail = static_cast<MD_SPAN_A_DETAIL*>(detail);

			if (kanban_parser->state == KanbanState::Board) {
				if (kanban_parser->board_section.task_read_state == TaskReadState::Attachments) {
					TaskDetail& task_detail = kanban_parser->board_section.current_board->task_details[kanban_parser->board_section.current_board->current_task_name];
					Attachment attachment;
					attachment.url = std::string(a_detail->href.text, a_detail->href.size);
					task_detail.attachments.push_back(attachment);
					task_detail.currentAttachment = &task_detail.attachments.back();
				}
			}
			//std::cout << "[Link]\n";
			break;
		}
		default:
		{
			break;
		}
		}
		return 0;
	}

	// Callback for leaving span elements
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

	void parseHeader(KanbanParser* kanban_parser, const std::string& text_content) {
		switch (kanban_parser->header_level) {
		case 0:
		{
			if (kanban_parser->read_kanban_board_name && kanban_parser->kanban_board_description.empty())
			{
				kanban_parser->kanban_board_description = text_content;
			}
			break;
		}
		case 1:
		{
			if (!kanban_parser->read_kanban_board_name)
			{
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
		switch (hash(text_content)) {
		case hash("Description"):
			kanban_parser->board_section.task_read_state = TaskReadState::Description;
			is_task_property = true;
			break;
		case hash("Labels"):
			kanban_parser->board_section.task_read_state = TaskReadState::Labels;
			is_task_property = true;
			break;
		case hash("Attachments"):
			kanban_parser->board_section.task_read_state = TaskReadState::Attachments;
			is_task_property = true;
			break;
		case hash("Checklist"):
			kanban_parser->board_section.task_read_state = TaskReadState::Checklist;
			is_task_property = true;
			break;
		}
		if (!is_task_property && kanban_parser->board_section.task_read_state == TaskReadState::Description) {
			BoardSection* current_board = kanban_parser->board_section.current_board;
			TaskDetail& tail_detail = current_board->task_details[kanban_parser->board_section.current_board->current_task_name];
			if (tail_detail.description.empty()) {
				text_content = ltrim(text_content);
				if (text_content.find(":") == 0)
				{
					text_content = text_content.substr(1);
				}
				text_content = trim(text_content);
				if (!text_content.empty()) {
					tail_detail.description.push_back(text_content);
				}
			}
			else {
				text_content = trim(text_content);
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
			kanban_parser->board_section.current_board->task_details[kanban_parser->board_section.current_board->current_task_name].labels.push_back(text_content);
			break;
		}
		case TaskReadState::Attachments:
		{
			if (kanban_parser->currently_reading_link) {
				auto& current_board = kanban_parser->board_section.current_board;
				Attachment* current_attachment = current_board->task_details[current_board->current_task_name].currentAttachment;
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
			kanban_parser->board_section.current_board->task_details[kanban_parser->board_section.current_board->current_task_name].checklist.push_back(checkbox);
			break;
		}
		}
	}

	void parseBoardSection(KanbanParser* kanban_parser, const std::string& text_content) {
		if (text_content == "[ ] " || text_content == "[x] ") {
			return;
		}
		switch (kanban_parser->list_item_level) {
		case 0:
		{
			BoardSection board_section;
			board_section.name = text_content;
			kanban_parser->board_section.boards.push_back(board_section);
			kanban_parser->board_section.current_board = &kanban_parser->board_section.boards.back();
			break;
		}
		case 1:
		{
			kanban_parser->board_section.current_board->current_task_name = text_content;
			kanban_parser->board_section.current_board->task_details[text_content] = TaskDetail();
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

	// Callback for processing text
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

	tl::expected<KanbanBoard, std::string> parse(std::string md_string) {
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

		KanbanParser kanban_parser;

		int result = md_parse(md_string.c_str(), md_string.size(), &parser, &kanban_parser);
		if (result != 0) {
			std::cerr << "Error parsing Markdown text." << std::endl;
		}
		KanbanBoard kanban_board;
		kanban_board.name = kanban_parser.kanban_board_name;
		kanban_board.description = kanban_parser.kanban_board_description;
		for (auto& [label_name, label_detail] : kanban_parser.label_section.label_details) {
			std::shared_ptr<KanbanLabel> kanban_label = std::make_shared<KanbanLabel>();
			kanban_label->name = label_name;
			kanban_board.labels.insert({ label_name, kanban_label });
		}
		for (BoardSection board_section : kanban_parser.board_section.boards) {
			KanbanList kanban_list;
			kanban_list.name = board_section.name;
			for (auto& [task_name, task_detail] : board_section.task_details) {
				std::shared_ptr<KanbanTask> kanban_task = std::make_shared<KanbanTask>();
				kanban_task->name = task_name;
				kanban_task->description = task_detail.description;
				for (const std::string& label : task_detail.labels) {
					auto& kanban_label_pair = kanban_board.labels.find(label);
					if (kanban_label_pair != kanban_board.labels.end()) {
						kanban_task->labels.push_back(kanban_label_pair->second);
					}
					else {
						auto kanban_label = std::make_shared<KanbanLabel>();
						kanban_label->name = label;
						kanban_label->tasks.push_back(kanban_task);
						kanban_board.labels[label] = kanban_label;
					}
				}
				for (const Attachment& attachment : task_detail.attachments) {
					kanban_task->attachments.push_back({ attachment.name, attachment.url });
				}
				for (const KanbanChecklistItem& checkbox : task_detail.checklist) {
					kanban_task->checklist.push_back(checkbox);
				}
				kanban_list.tasks.insert({ task_name, kanban_task });
			}
			kanban_board.list.insert({ board_section.name , kanban_list });
		}
		for (auto& [label_name, label_detail] : kanban_parser.label_section.label_details) {
			std::shared_ptr<KanbanLabel> kanban_label = kanban_board.labels[label_name];
			for (BoardSection board_section : kanban_parser.board_section.boards) {
				for (auto& [task_name, task_detail] : board_section.task_details) {
					for (const std::string& label : task_detail.labels) {
						kanban_label->tasks.push_back(kanban_board.list[board_section.name].tasks[task_name]);
					}
				}
			}
		}
		return kanban_board;
	}
}