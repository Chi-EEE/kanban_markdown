#pragma once

#include <string>
#include <vector>
#include <memory>

#include <asap/asap.h>
#include <cpp-dump/dump.hpp>

#include <tsl/robin_map.h>
#include <tsl/robin_set.h>

#include <kanban_markdown/internal.hpp>

namespace kanban_markdown {
	class DuplicateNameTracker {
	public:
		unsigned int getHash() {
			unsigned int counter;
			do {
				counter = ++this->counter;
			} while (this->used_hash.contains(this->counter));
			this->used_hash.insert(this->counter);
			return counter;
		}

		void removeHash(unsigned int counter) {
			if (counter == this->counter) {
				this->counter--;
			}
			this->used_hash.erase(counter);
		}

		unsigned int counter = 1;
		tsl::robin_set<unsigned int> used_hash;
	};

	struct KanbanAttachment
	{
		bool operator==(const KanbanAttachment& other) const {
			return this->name == other.name && this->url == other.url;
		}

		bool operator!=(const KanbanAttachment& other) const {
			return !(*this == other);
		}

		std::string name;
		std::string url;
	};

	struct KanbanChecklistItem
	{
		bool operator==(const KanbanChecklistItem& other) const {
			return this->checked == other.checked && this->name == other.name;
		}

		bool operator!=(const KanbanChecklistItem& other) const {
			return !(*this == other);
		}

		bool checked = false;
		std::string name;
	};

	struct KanbanTask;

	struct KanbanLabel
	{
		bool operator==(const KanbanLabel& other) const {
			if (this->name != other.name || this->color != other.color || this->tasks.size() != other.tasks.size()) {
				return false;
			}
			// Cannot compare tasks because they would compare recursively
			/*for (size_t i = 0; i < this->tasks.size(); i++) {
				if (this->tasks[i] != other.tasks[i]) {
					return false;
				}
			}*/
			return true;
		}

		bool operator!=(const KanbanLabel& other) const {
			return !(*this == other);
		}

		std::string color;
		std::string name;
		std::vector<std::shared_ptr<KanbanTask>> tasks;
	};

	struct KanbanTask
	{
		bool operator==(const KanbanTask& other) const {
			if (this->checked != other.checked || this->counter != other.counter || this->name != other.name || this->description.size() != other.description.size() || this->labels.size() != other.labels.size() || this->attachments.size() != other.attachments.size() || this->checklist.size() != other.checklist.size()) {
				return false;
			}
			for (size_t i = 0; i < this->description.size(); i++) {
				if (this->description[i] != other.description[i]) {
					return false;
				}
			}
			for (size_t i = 0; i < this->labels.size(); i++) {
				if (this->labels[i] != other.labels[i]) {
					return false;
				}
			}
			for (size_t i = 0; i < this->attachments.size(); i++) {
				if (this->attachments[i] != other.attachments[i]) {
					return false;
				}
			}
			for (size_t i = 0; i < this->checklist.size(); i++) {
				if (this->checklist[i] != other.checklist[i]) {
					return false;
				}
			}
			return true;
		}

		bool operator!=(const KanbanTask& other) const {
			return !(*this == other);
		}

		bool checked = false;
		unsigned int counter;
		std::string name;
		std::vector<std::string> description;
		std::vector<std::shared_ptr<KanbanLabel>> labels;
		std::vector<std::shared_ptr<KanbanAttachment>> attachments;
		std::vector<std::shared_ptr<KanbanChecklistItem>> checklist;
	};

	struct KanbanList
	{
		bool operator==(const KanbanList& other) const {
			if (this->counter != other.counter || this->name != other.name || this->tasks.size() != other.tasks.size()) {
				return false;
			}
			for (size_t i = 0; i < this->tasks.size(); i++) {
				if (this->tasks[i] != other.tasks[i]) {
					return false;
				}
			}
			return true;
		}

		bool operator!=(const KanbanList& other) const {
			return !operator==(other);
		}

		bool checked;
		unsigned int counter;
		std::string name;
		std::vector<std::shared_ptr<KanbanTask>> tasks;
	};

	struct KanbanBoard
	{
		bool operator==(const KanbanBoard& other) const {
			if (this->color != other.color || this->created.timestamp() != other.created.timestamp() || this->last_modified.timestamp() != other.last_modified.timestamp() || this->name != other.name || this->description != other.description || this->labels.size() != other.labels.size() || this->list.size() != other.list.size()) {
				return false;
			}
			for (size_t i = 0; i < this->description.size(); i++) {
				if (this->description[i] != other.description[i]) {
					return false;
				}
			}
			for (size_t i = 0; i < this->list.size(); i++) {
				if (this->list[i] != other.list[i]) {
					return false;
				}
			}
			return true;
		}

		bool operator!=(const KanbanBoard& other) const {
			return !(*this == other);
		}

		std::string color;
		asap::datetime created;
		asap::datetime last_modified;
		unsigned int version;
		std::string checksum;

		std::string name;
		std::string description;
		std::vector<std::shared_ptr<KanbanLabel>> labels;
		std::vector<std::shared_ptr<KanbanList>> list;
		tsl::robin_map<std::string, DuplicateNameTracker> list_name_tracker_map;
		tsl::robin_map<std::string, DuplicateNameTracker> task_name_tracker_map;
	};
}
CPP_DUMP_DEFINE_EXPORT_OBJECT(asap::datetime, when);
CPP_DUMP_DEFINE_EXPORT_OBJECT(kanban_markdown::KanbanAttachment, name, url);
CPP_DUMP_DEFINE_EXPORT_OBJECT(kanban_markdown::KanbanChecklistItem, checked, name);
CPP_DUMP_DEFINE_EXPORT_OBJECT(kanban_markdown::KanbanTask, checked, name, description, labels, attachments, checklist);
CPP_DUMP_DEFINE_EXPORT_OBJECT(kanban_markdown::KanbanLabel, name, tasks);
CPP_DUMP_DEFINE_EXPORT_OBJECT(kanban_markdown::KanbanList, checked, name, tasks);
CPP_DUMP_DEFINE_EXPORT_OBJECT(kanban_markdown::KanbanBoard, color, created, last_modified, version, checksum, name, description, labels, list);
