#pragma once

#include <iostream>
#include <vector>
#include <fmt/format.h>
#include <tl/expected.hpp>
#include <algorithm> 
#include <cctype>
#include <locale>

#include <unordered_map>

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

	struct LabelSection {
		std::vector<std::string> list_items;
	};

	struct TaskDetail {
		std::string name;
		std::vector<std::string> description;
		std::vector<std::string> labels;
		std::vector<std::string> attachments;
		std::vector<std::string> checklist;
	};

	struct BoardSection {
		BoardSection() {
			/*magic_enum::enum_for_each<TaskReadState>([](auto val) {
				constexpr TaskReadState task_read_state = val;
				task_detail[task_read_state] = TaskDetail();
				});*/
		}
		std::string name;
		std::string current_task_name;
		std::unordered_map<std::string, TaskDetail> task_details;
	};

	struct BoardListSection {
		std::vector<BoardSection> boards;
		BoardSection* current_board = nullptr;
		TaskReadState task_read_state = TaskReadState::None;
		bool reading_task_detail = false;
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
		unsigned int sub_list_item_count = 0;
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
			if (kanban_parser->state == KanbanState::Board) {
				kanban_parser->board_section.reading_task_detail = false;
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
		case MD_SPAN_EM:
		{
			//std::cout << "[Emphasis]\n";
			break;
		}
		case MD_SPAN_STRONG:
		{
			//std::cout << "[Strong]\n";
			break;
		}
		case MD_SPAN_A:
		{
			//std::cout << "[Link]\n";
			break;
		}
		case MD_SPAN_IMG:
		{
			//std::cout << "[Image]\n";
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
		//std::cout << "Leaving span type: " << type << std::endl;
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
			break;
		}
		case 2:
		{
			kanban_parser->label_section.list_items.push_back(text_content);
			break;
		}
		}
	}

	void parseTaskDetails(KanbanParser* kanban_parser, const std::string& text_content) {
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
			current_board->task_details[kanban_parser->board_section.current_board->current_task_name].description.push_back(text_content);
		}
	}

	void parseTaskDetailList(KanbanParser* kanban_parser, const std::string& text_content) {
		switch (kanban_parser->board_section.task_read_state) {
		case TaskReadState::Labels:
		{
			std::cout << "Labels: " << text_content << '\n';
			kanban_parser->board_section.current_board->task_details[kanban_parser->board_section.current_board->current_task_name].labels.push_back(text_content);
			break;
		}
		case TaskReadState::Attachments:
		{
			std::cout << "Attachments: " << text_content << '\n';
			kanban_parser->board_section.current_board->task_details[kanban_parser->board_section.current_board->current_task_name].attachments.push_back(text_content);
			break;
		}
		case TaskReadState::Checklist:
		{
			std::cout << "Checklist: " << text_content << '\n';
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
			kanban_parser->board_section.current_board->task_details[kanban_parser->board_section.current_board->current_task_name].checklist.push_back(text_content.substr(4));
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
		for (const std::string& label : kanban_parser.label_section.list_items) {
			std::shared_ptr<KanbanLabel> kanban_label = std::make_shared<KanbanLabel>();
			kanban_label->name = label;
			kanban_board.labels.insert({ label, kanban_label });
		}
		for (BoardSection board_section : kanban_parser.board_section.boards) {
			KanbanList kanban_list;
			kanban_list.name = board_section.name;
			for (auto& [task_name, task_detail] : board_section.task_details) {
				std::shared_ptr<KanbanTask> kanban_task = std::make_shared<KanbanTask>();
				kanban_task->name = task_name;
				kanban_task->description = task_detail.description;
				for (const std::string& label : task_detail.labels) {
					kanban_task->labels.push_back(kanban_board.labels[label]);
				}
				//kanban_task.attachments = task_detail.attachments;
				//kanban_task.checklist = task_detail.checklist;
				kanban_list.tasks.insert({ task_name, kanban_task });
			}
			kanban_board.list.insert({ board_section.name , kanban_list });
		}
		for (const std::string& label : kanban_parser.label_section.list_items) {
			std::shared_ptr<KanbanLabel> kanban_label = kanban_board.labels[label];
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