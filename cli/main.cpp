#include <iostream>
#include <fstream>
#include <filesystem>

#include <kanban_markdown/kanban_markdown.hpp>
#include <argparse/argparse.hpp>

int main(int argc, char* argv[]) {
	argparse::ArgumentParser program("kanban_md");

	program.add_argument("-i", "--input")
		.help("Input file")
		.required();

	program.add_argument("-o", "--output")
		.help("Output file")
		.required();

	program.add_argument("-g", "--github")
		.help("Use GitHub markdown")
		.default_value(false)
		.implicit_value(true);

	std::string input_file;
	std::string output_file;
	bool github;

	try {
		program.parse_args(argc, argv);
	}
	catch (const std::runtime_error& err) {
		std::cout << err.what() << std::endl;
		std::cout << program;
		return 1;
	}

	input_file = program.get<std::string>("--input");
	output_file = program.get<std::string>("--output");

	if (!std::filesystem::exists(input_file)) {
		std::cout << "Error: Input file does not exist." << std::endl;
		return 1;
	}

	if (std::filesystem::exists(output_file)) {
		std::cout << "Error: Output file already exists." << std::endl;
		return 1;
	}

	github = program.get<bool>("--github");

	auto& maybe_kanban_board = kanban_markdown::parse(input_file);
	if (maybe_kanban_board.has_value()) {
		auto& kanban_board = maybe_kanban_board.value();
		kanban_markdown::KanbanWriterFlags kanban_writer_flags;
		kanban_writer_flags.github = github;

		auto markdown = kanban_markdown::markdown_format(kanban_board, kanban_writer_flags);

		std::ofstream output_stream(output_file);
		output_stream << markdown;
		output_stream.close();
	}
	else {
		std::cout << "Error: " << maybe_kanban_board.error() << std::endl;
	}
}