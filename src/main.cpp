#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>

#include "kanban_markdown/kanban_markdown.hpp"

int main(int argc, char* argv[]) {

	//kanban_markdown::KanbanBoard board;
	//kanban_markdown::KanbanLabel kanban_label;
	//kanban_markdown::KanbanTask kanban_task;
	//kanban_task.title = "Huge Bug";
	//kanban_label.title = "Bug";
	//kanban_label.tasks.push_back(std::make_shared<kanban_markdown::KanbanTask>(kanban_task));
	//board.labels.push_back(kanban_label);
	//std::cout << kanban_markdown::format(board);
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

	auto& maybe_kanban_board = kanban_markdown::parse(todo_string);
	if (!maybe_kanban_board.has_value()) {
		std::cout << maybe_kanban_board.error() << '\n';
		return 1;
	}

	std::ofstream out(exe_dir.string() + "/aaa.txt");
	std::cout << kanban_markdown::markdown_format(maybe_kanban_board.value());
	out << kanban_markdown::markdown_format(maybe_kanban_board.value());
	out.close();

	return 0;
}
