#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <re2/re2.h>

#include "internal.hpp"

namespace server
{
	class KanbanPathVisitor
	{
#pragma region Public
	public:
		KanbanPathVisitor(kanban_markdown::KanbanBoard* kanban_board, std::string path, void* userdata) {
			this->kanban_board = kanban_board;
			this->path_split = parsePathString(urlDecode(path));
			this->userdata = userdata;
		}

		~KanbanPathVisitor()
		{
			this->kanban_board = nullptr;
			this->userdata = nullptr;
		}

		void run()
		{
			if (this->path_split.size() == 1 && this->path_split.back().empty()) {
				this->visitBoard();
			}
			else {
				this->internal_visitBoard();
			}
		}
#pragma endregion

#pragma region Use These
	protected:
		kanban_markdown::KanbanBoard* kanban_board;
		std::vector<std::string> path_split;
		void* userdata;
#pragma endregion

#pragma region Override These
	private:
		// KanbanBoard
		virtual void visitBoard() = 0;

		// KanbanBoard.name
		virtual void editBoardName() = 0;
		// KanbanBoard.description
		virtual void editBoardDescription() = 0;
		// KanbanBoard.color
		virtual void editBoardColor() = 0;
		// KanbanBoard.list
		virtual void editBoardList() = 0;
		// KanbanBoard.labels
		virtual void editBoardLabels() = 0;

		// KanbanList
		virtual void visitList(std::vector<std::shared_ptr<kanban_markdown::KanbanList>>::iterator kanban_list_iterator) = 0;

		// KanbanList.name
		virtual void editListName(std::shared_ptr<kanban_markdown::KanbanList> kanban_list) = 0;
		// KanbanList.checked
		virtual void editListChecked(std::shared_ptr<kanban_markdown::KanbanList> kanban_list) = 0;
		// KanbanList.tasks
		virtual void editListTasks(std::shared_ptr<kanban_markdown::KanbanList> kanban_list) = 0;

		// KanbanTask
		virtual void visitTask(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::vector<std::shared_ptr<kanban_markdown::KanbanTask>>::iterator kanban_task_iterator) = 0;

		// KanbanTask.name
		virtual void editTaskName(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) = 0;
		// KanbanTask.description
		virtual void editTaskDescription(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) = 0;
		// KanbanTask.checked
		virtual void editTaskChecked(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) = 0;
		// KanbanTask.labels
		virtual void editTaskLabels(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) = 0;
		// KanbanTask.attachments
		virtual void editTaskAttachments(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) = 0;
		// KanbanTask.checklist
		virtual void editTaskChecklist(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task) = 0;

		// KanbanLabel
		virtual void visitTaskLabel(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::vector<std::shared_ptr<kanban_markdown::KanbanLabel>>::iterator kanban_label_iterator) = 0;

		// KanbanLabel.name
		virtual void editTaskLabelName(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label) = 0;
		// KanbanLabel.color
		virtual void editTaskLabelColor(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label) = 0;

		// KanbanAttachment
		virtual void visitTaskAttachment(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::vector<std::shared_ptr<kanban_markdown::KanbanAttachment>>::iterator kanban_attachment_iterator) = 0;

		// KanbanAttachment.name
		virtual void editTaskAttachmentName(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanAttachment> kanban_attachment) = 0;
		// KanbanAttachment.url
		virtual void editTaskAttachmentUrl(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanAttachment> kanban_attachment) = 0;

		// KanbanChecklistItem
		virtual void visitTaskChecklist(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::vector<std::shared_ptr<kanban_markdown::KanbanChecklistItem>>::iterator kanban_checklist_item_iterator) = 0;

		// KanbanChecklistItem.name
		virtual void editTaskChecklistName(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanChecklistItem> kanban_checklist_item) = 0;
		// KanbanChecklistItem.checked
		virtual void editTaskChecklistChecked(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::shared_ptr<kanban_markdown::KanbanChecklistItem> kanban_checklist_item) = 0;

		// KanbanLabel
		virtual void visitLabel(std::vector<std::shared_ptr<kanban_markdown::KanbanLabel>>::iterator kanban_label_iterator) = 0;

		// KanbanLabel.name
		virtual void editLabelName(std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label) = 0;
		// KanbanLabel.color
		virtual void editLabelColor(std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label) = 0;
#pragma endregion

#pragma region Private
	private:
		std::string urlDecode(std::string text)
		{
			std::string escaped;

			for (auto i = text.begin(), nd = text.end(); i < nd; ++i)
			{
				auto c = (*i);

				switch (c)
				{
				case '%':
					if (i[1] && i[2]) {
						char hs[]{ i[1], i[2] };
						escaped += static_cast<char>(strtol(hs, nullptr, 16));
						i += 2;
					}
					break;
				case '+':
					escaped += ' ';
					break;
				default:
					escaped += c;
				}
			}

			return escaped;
		}

		std::vector<std::string> parsePathString(const std::string& s, char delimiter = '.')
		{
			std::vector<std::string> splitted;
			splitted.push_back("");
			bool ignore = false;
			bool flag = false;
			for (int i = 0; i < s.size(); ++i)
			{
				if (s[i] == '\\')
				{
					ignore = true;
				}
				else if (ignore)
				{
					ignore = false;
				}
				else
				{
					if (s[i] == '\"')
					{
						flag = !flag;
						continue;
					}
				}

				if (s[i] == delimiter && !flag)
				{
					splitted.push_back("");
				}
				else
				{
					splitted.back() += s[i];
				}
			}
			return splitted;
		}

		void internal_visitBoard()
		{
			std::string board_item = this->path_split[0];
			switch (hash(board_item))
			{
			case hash("name"):
				this->editBoardName();
				break;
			case hash("description"):
				this->editBoardDescription();
				break;
			case hash("color"):
				this->editBoardColor();
				break;
			case hash("list"):
				this->editBoardList();
				break;
			case hash("labels"):
				this->editBoardLabels();
				break;
			default:
			{
				static re2::RE2 path_pattern(R"((\w+)\[\"(.+)\"\])");
				std::string board_item_name;
				std::string board_item_index_name;

				if (!RE2::PartialMatch(board_item, path_pattern, &board_item_name, &board_item_index_name))
				{
					throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanBoard named "{}")", board_item));
				}

				switch (hash(board_item_name))
				{
				case hash("list"):
				{
					internal_visitList(board_item_index_name);
					break;
				}
				case hash("labels"):
				{
					internal_visitLabels(board_item_index_name);
					break;
				}
				default:
					throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanBoard named "{}")", board_item_name));
				}
				break;
			}
			}
		}

		void internal_visitList(std::string board_item_index_name)
		{
			auto it = std::find_if(this->kanban_board->list.begin(), this->kanban_board->list.end(), [&board_item_index_name](const auto& x)
				{ return x->name == board_item_index_name; });
			if (it == this->kanban_board->list.end())
			{
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanBoard.list named "{}")", board_item_index_name));
			}
			if (this->path_split.size() == 1)
			{
				this->visitList(it);
			}
			else
			{
				std::shared_ptr<kanban_markdown::KanbanList> kanban_list = *it;
				std::string second = this->path_split[1];
				switch (hash(second))
				{
				case hash("name"):
					this->editListName(kanban_list);
					break;
				case hash("checked"):
					this->editListChecked(kanban_list);
					break;
				case hash("tasks"):
					this->editListTasks(kanban_list);
					break;
				default:
				{
					static re2::RE2 path_pattern(R"((\w+)\[\"(.+)\"\]\[(.+)\])");
					std::string list_item_name;
					std::string task_index_name;
					std::string task_index_counter_str;
					if (!RE2::PartialMatch(second, path_pattern, &list_item_name, &task_index_name, &task_index_counter_str))
					{
						throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanList named "{}")", second));
					}

					unsigned int task_index_counter = std::stoi(task_index_counter_str);

					switch (hash(list_item_name))
					{
					case hash("tasks"):
					{
						this->internal_visitTasks(kanban_list, task_index_name, task_index_counter);
						break;
					}
					default:
						throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanList named "{}")", second));
					}
					break;
				}
				}
			}
		}

		void internal_visitTasks(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::string task_index_name, unsigned int task_index_counter)
		{

			auto it = std::find_if(kanban_list->tasks.begin(), kanban_list->tasks.end(), [&task_index_name, &task_index_counter](const std::shared_ptr<kanban_markdown::KanbanTask>& x)
				{ return x->name == task_index_name && x->counter == task_index_counter; });
			if (it == kanban_list->tasks.end())
			{
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanList.tasks named "{}")", task_index_name));
			}
			if (this->path_split.size() == 2)
			{
				this->visitTask(kanban_list, it);
			}
			else
			{
				std::shared_ptr<kanban_markdown::KanbanTask>& kanban_task = *it;
				std::string third = this->path_split[2];
				switch (hash(third))
				{
				case hash("name"):
					this->editTaskName(kanban_list, kanban_task);
					break;
				case hash("checked"):
					this->editTaskChecked(kanban_list, kanban_task);
					break;
				case hash("description"):
					this->editTaskDescription(kanban_list, kanban_task);
					break;
				case hash("labels"):
					this->editTaskLabels(kanban_list, kanban_task);
					break;
				case hash("attachments"):
					this->editTaskAttachments(kanban_list, kanban_task);
					break;
				case hash("checklist"):
					this->editTaskChecklist(kanban_list, kanban_task);
					break;
				default:
				{
					static re2::RE2 path_pattern(R"((\w+)\[\"(.+)\"\])");
					std::string task_item_name;
					std::string task_item_index_name;
					if (!RE2::PartialMatch(third, path_pattern, &task_item_name, &task_item_index_name))
					{
						throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanTask named "{}")", third));
					}

					switch (hash(task_item_name))
					{
					case hash("labels"):
						this->internal_visitTaskLabels(kanban_list, kanban_task, task_item_index_name);
						break;
					case hash("attachments"):
						this->internal_visitTaskAttachments(kanban_list, kanban_task, task_item_index_name);
						break;
					case hash("checklist"):
						this->internal_visitTaskChecklist(kanban_list, kanban_task, task_item_index_name);
						break;
					default:
						throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanTask named "{}")", third));
					}
					break;
				}
				}
			}
		}

		void internal_visitTaskLabels(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::string task_item_index_name)
		{
			auto it = std::find_if(kanban_task->labels.begin(), kanban_task->labels.end(), [&task_item_index_name](const auto& x)
				{ return x->name == task_item_index_name; });
			if (it == kanban_task->labels.end())
			{
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanTask.labels named "{}")", task_item_index_name));
			}
			if (this->path_split.size() == 3)
			{
				this->visitTaskLabel(kanban_list, kanban_task, it);
			}
			else
			{
				std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label = *it;
				std::string fourth = this->path_split[3];
				switch (hash(fourth))
				{
				case hash("name"):
					this->editTaskLabelName(kanban_list, kanban_task, kanban_label);
					break;
				case hash("color"):
					this->editTaskLabelColor(kanban_list, kanban_task, kanban_label);
					break;
				case hash("tasks"):
					throw std::runtime_error("Error: Editing Tasks through labels is not supported. Please edit tasks directly within their respective lists.");
				default:
					throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanLabel named "{}")", fourth));
				}
			}
		}

		void internal_visitTaskAttachments(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::string task_item_index_name)
		{
			auto it = std::find_if(kanban_task->attachments.begin(), kanban_task->attachments.end(), [&task_item_index_name](const auto& x)
				{ return x->name == task_item_index_name; });
			if (it == kanban_task->attachments.end())
			{
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanTask.attachments named "{}")", task_item_index_name));
			}
			if (this->path_split.size() == 3)
			{
				this->visitTaskAttachment(kanban_list, kanban_task, it);
			}
			else
			{
				std::shared_ptr<kanban_markdown::KanbanAttachment> kanban_attachment = *it;
				std::string fourth = this->path_split[3];
				switch (hash(fourth))
				{
				case hash("name"):
					this->editTaskAttachmentName(kanban_list, kanban_task, kanban_attachment);
					break;
				case hash("url"):
					this->editTaskAttachmentUrl(kanban_list, kanban_task, kanban_attachment);
					break;
				default:
					throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanAttachment named "{}")", fourth));
				}
			}
		}

		void internal_visitTaskChecklist(std::shared_ptr<kanban_markdown::KanbanList> kanban_list, std::shared_ptr<kanban_markdown::KanbanTask> kanban_task, std::string task_item_index_name)
		{
			auto it = std::find_if(kanban_task->checklist.begin(), kanban_task->checklist.end(), [&task_item_index_name](const auto& x)
				{ return x->name == task_item_index_name; });
			if (it == kanban_task->checklist.end())
			{
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanTask.checklist named "{}")", task_item_index_name));
			}
			if (this->path_split.size() == 3)
			{
				this->visitTaskChecklist(kanban_list, kanban_task, it);
			}
			else
			{
				std::shared_ptr<kanban_markdown::KanbanChecklistItem> kanban_checklist_item = *it;
				std::string fourth = this->path_split[3];
				switch (hash(fourth))
				{
				case hash("name"):
					this->editTaskChecklistName(kanban_list, kanban_task, kanban_checklist_item);
					break;
				case hash("checked"):
					this->editTaskChecklistChecked(kanban_list, kanban_task, kanban_checklist_item);
					break;
				default:
					throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanChecklistItem named "{}")", fourth));
				}
			}
		}

		void internal_visitLabels(std::string board_item_index_name)
		{
			auto it = std::find_if(this->kanban_board->labels.begin(), this->kanban_board->labels.end(), [&board_item_index_name](const auto& x)
				{ return x->name == board_item_index_name; });
			if (it == this->kanban_board->labels.end())
			{
				throw std::runtime_error(fmt::format(R"(Invalid path: There are no keys inside KanbanBoard.labels named "{}")", board_item_index_name));
			}
			if (this->path_split.size() == 1)
			{
				this->visitLabel(it);
			}
			else
			{
				std::shared_ptr<kanban_markdown::KanbanLabel> kanban_label = *it;
				std::string second = this->path_split[1];
				switch (hash(second))
				{
				case hash("name"):
					this->editLabelName(kanban_label);
					break;
				case hash("color"):
					this->editLabelColor(kanban_label);
					break;
				case hash("tasks"):
					throw std::runtime_error("Error: Editing Tasks through labels is not supported. Please edit tasks directly within their respective lists.");
				default:
					throw std::runtime_error(fmt::format(R"(Invalid path: There are no fields inside KanbanLabel named "{}")", second));
				}
			}
		}
#pragma endregion	
	};
}