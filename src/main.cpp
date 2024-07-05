#include <filesystem>
#include <iostream>
#include <md4c.h>
#include <fstream>
#include <sstream>

#include "writer.hpp"

struct KanBan {
	std::string title = "dsa";
	std::string description = "dsa";
};

// Callback for processing block elements
int enter_block_callback(MD_BLOCKTYPE type, void* detail, void* userdata) {
	KanBan* kanban = static_cast<KanBan*>(userdata);
	switch (type) {
	case MD_BLOCK_DOC:
		std::cout << "[Document Start]\n";
		break;
	case MD_BLOCK_H:
		std::cout << "[Header]\n";
		break;
	case MD_BLOCK_P:
		std::cout << "[Paragraph]\n";
		break;
	case MD_BLOCK_UL:
		std::cout << "[Unordered List]\n";
		break;
	case MD_BLOCK_OL:
		std::cout << "[Ordered List]\n";
		break;
	default:
		std::cout << "Unknown Type: " << type << "\n";
		break;
	}
	return 0;
}

// Callback for leaving block elements
int leave_block_callback(MD_BLOCKTYPE type, void* detail, void* userdata) {
	KanBan* kanban = static_cast<KanBan*>(userdata);
	switch (type) {
	case MD_BLOCK_DOC:
		std::cout << "[Document End]\n";
		break;
	case MD_BLOCK_H:
		std::cout << "[Header]\n";
		break;
	case MD_BLOCK_P:
		std::cout << "[Paragraph]\n";
		break;
	case MD_BLOCK_UL:
		std::cout << "[Unordered List]\n";
		break;
	case MD_BLOCK_OL:
		std::cout << "[Ordered List]\n";
		break;
	default:
		std::cout << "Unknown Type: " << type << "\n";
		break;
	}
	return 0;
}

// Callback for processing span elements
int enter_span_callback(MD_SPANTYPE type, void* detail, void* userdata) {
	KanBan* kanban = static_cast<KanBan*>(userdata);
	switch (type) {
	case MD_SPAN_EM:
		std::cout << "[Emphasis]\n";
		break;
	case MD_SPAN_STRONG:
		std::cout << "[Strong]\n";
		break;
	case MD_SPAN_A:
		std::cout << "[Link]\n";
		break;
	case MD_SPAN_IMG:
		std::cout << "[Image]\n";
		break;
	default:
		break;
	}
	return 0;
}

// Callback for leaving span elements
int leave_span_callback(MD_SPANTYPE type, void* detail, void* userdata) {
	KanBan* kanban = static_cast<KanBan*>(userdata);
	std::cout << "Leaving span type: " << type << std::endl;
	return 0;
}

// Callback for processing text
int text_callback(MD_TEXTTYPE type, const MD_CHAR* text, MD_SIZE size, void* userdata) {
	KanBan* kanban = static_cast<KanBan*>(userdata);
	std::string text_content(text, size);
	switch (type) {
	case MD_TEXT_NORMAL:
		std::cout << "Text: " << text_content << "\n";
		break;
	case MD_TEXT_CODE:
		std::cout << "Code: " << text_content << "\n";
		break;
	case MD_TEXT_HTML:
		std::cout << "HTML: " << text_content << "\n";
		break;
	default:
		break;
	}
	return 0;
}


void debug(const char* msg, void* userdata) {
	KanBan* kanban = static_cast<KanBan*>(userdata);
	std::cout << msg << '\n';
}


int main(int argc, char* argv[]) {
	KanbanBoard board;
	KanbanLabel kanban_label;
	KanbanTask kanban_task;
	kanban_task.title = "Huge Bug";
	kanban_label.title = "Bug";
	kanban_label.tasks.push_back(std::make_shared<KanbanTask>(kanban_task));
	board.labels.push_back(kanban_label);
	std::cout << kanban_markdown::format(board);
	std::filesystem::path exe_dir = std::filesystem::weakly_canonical(std::filesystem::path(argv[0])).parent_path();
	const std::string TODO_MD_PATH = exe_dir.string() + "/TODO.md";

	if (!std::filesystem::exists(TODO_MD_PATH))
	{
		std::cout << "Could not find TODO.md in the exe dir\n";
		return 1;
	}

	std::ifstream todo_md_file(TODO_MD_PATH);
	std::stringstream buffer;
	buffer << todo_md_file.rdbuf();
	const std::string todo_string = buffer.str();
	todo_md_file.close();
	buffer.clear();

	return 1;
	// Define the parser
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

	KanBan kanban;
	// Parse the Markdown text
	int result = md_parse(todo_string.c_str(), todo_string.size(), &parser, &kanban);

	if (result != 0) {
		std::cerr << "Error parsing Markdown text." << std::endl;
	}

	return 0;
}
