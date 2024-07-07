#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>

#include "kanban_markdown/kanban_markdown.hpp"

int main(int argc, char* argv[]) {
	std::filesystem::path exe_dir = std::filesystem::weakly_canonical(std::filesystem::path(argv[0])).parent_path();
	const std::string TODO_MD_PATH = exe_dir.string() + "/aaa.md";

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

	std::ofstream out(exe_dir.string() + "/aaa.md");
	std::cout << kanban_markdown::markdown_format(maybe_kanban_board.value());
	out << kanban_markdown::markdown_format(maybe_kanban_board.value());
	out.close();

	return 0;
}
