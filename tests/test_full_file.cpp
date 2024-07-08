#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>

#include <kanban_markdown/kanban_markdown.hpp>
using namespace kanban_markdown;

int main(int argc, char** argv) {
	std::filesystem::path exe_dir = std::filesystem::weakly_canonical(std::filesystem::path(argv[0])).parent_path();
	const std::string TODO_MD_PATH = exe_dir.string() + "/data/TODO.md";

	if (!std::filesystem::exists(TODO_MD_PATH))
	{
		std::cout << "Error: Could not find TODO.md in the exe dir\n";
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
		std::cout << "Error: " << maybe_kanban_board.error() << '\n';
		return 1;
	}
	KanbanBoard compareKanbanBoard;
	compareKanbanBoard.name = "Kanban Board";
	compareKanbanBoard.description = "This is a test!";

	std::shared_ptr<KanbanTask> compareKanbanTask1 = std::make_shared<KanbanTask>();
	compareKanbanTask1->checked = false;
	compareKanbanTask1->name = "Test Task 1";
	compareKanbanTask1->description.push_back("Test Description");
	compareKanbanTask1->attachments.push_back(KanbanAttachment{ "Test_Image", "test_image.txt" });
	compareKanbanTask1->checklist.push_back(KanbanChecklistItem{ true, "Test" });

	std::shared_ptr<KanbanLabel> compareKanbanLabel = std::make_shared<KanbanLabel>();
	compareKanbanLabel->name = "Test Label";

	compareKanbanLabel->tasks.push_back(compareKanbanTask1);
	compareKanbanTask1->labels.push_back(compareKanbanLabel);

	std::shared_ptr<KanbanTask> compareKanbanTask2 = std::make_shared<KanbanTask>();
	compareKanbanTask2->checked = true;
	compareKanbanTask2->name = "Test Task 2";
	compareKanbanTask2->description.push_back("Another Test Description");

	KanbanList compareKanbanList1;
	compareKanbanList1.name = "To do:";

	KanbanList compareKanbanList2;
	compareKanbanList2.name = "Doing:";

	KanbanList compareKanbanList3;
	compareKanbanList3.name = "On Hold:";

	KanbanList compareKanbanList4;
	compareKanbanList4.name = "Done:";

	compareKanbanList1.tasks.insert({ "Test Task 1", std::move(compareKanbanTask1) });
	compareKanbanList4.tasks.insert({ "Test Task 2", std::move(compareKanbanTask2) });

	compareKanbanBoard.list.insert({ "To do:", compareKanbanList1 });
	compareKanbanBoard.list.insert({ "Doing:", compareKanbanList2 });
	compareKanbanBoard.list.insert({ "On Hold:", compareKanbanList3 });
	compareKanbanBoard.list.insert({ "Done:", compareKanbanList4 });

	compareKanbanBoard.labels.insert({ "Test Label", std::move(compareKanbanLabel) });

	auto& kanban_board = maybe_kanban_board.value();

	if (kanban_board != compareKanbanBoard) {
		std::cout << "Error: Kanban board does not match\n";
		return 1;
	}

	std::cout << "Success: Parsed TODO.md\n";
	return 0;
}