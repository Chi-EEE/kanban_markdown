#pragma once

#include <string>
#include <vector>
#include <memory>

#include <asap/asap.h>
#include <tsl/ordered_map.h>

#include "internal.hpp"

namespace kanban_markdown {
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
			if (this->name != other.name) {
				return false;
			}
			return true;
		}

		bool operator!=(const KanbanLabel& other) const {
			return !(*this == other);
		}

		std::string name;
		tsl::ordered_map<std::string, std::shared_ptr<KanbanTask>> tasks;
	};

	struct KanbanTask
	{
		bool operator==(const KanbanTask& other) const {
			if (this->checked != other.checked || this->name != other.name || this->description.size() != other.description.size() || this->labels.size() != other.labels.size() || this->attachments.size() != other.attachments.size() || this->checklist.size() != other.checklist.size()) {
				return false;
			}
			for (size_t i = 0; i < this->description.size(); i++) {
				if (this->description[i] != other.description[i]) {
					return false;
				}
			}
			for (auto& [label_name, label] : this->labels) {
				auto& otherLabel = other.labels.at(label_name);
				if (*label != *otherLabel) {
					return false;
				}
			}
			for (auto& [attachment_name, attachment] : this->attachments) {
				auto& otherAttachment = other.attachments.at(attachment_name);
				if (*attachment != *otherAttachment) {
					return false;
				}
			}
			for (auto& [checklist_item_name, checklist_item] : this->checklist) {
				auto& otherChecklistItem = other.checklist.at(checklist_item_name);
				if (*checklist_item != *otherChecklistItem) {
					return false;
				}
			}
			return true;
		}

		bool operator!=(const KanbanTask& other) const {
			return !(*this == other);
		}

		bool checked = false;
		std::string name;
		std::vector<std::string> description;
		tsl::ordered_map<std::string, std::shared_ptr<KanbanLabel>> labels;
		tsl::ordered_map<std::string, std::shared_ptr<KanbanAttachment>> attachments;
		tsl::ordered_map<std::string, std::shared_ptr<KanbanChecklistItem>> checklist;
	};

	struct KanbanList
	{
		bool operator==(const KanbanList& other) const {
			if (this->name != other.name || this->tasks.size() != other.tasks.size()) {
				return false;
			}
			// Go through both maps and compare the tasks
			for (auto& [task_name, task] : this->tasks) {
				auto& otherTask = other.tasks.at(task_name);
				if (*task != *otherTask) {
					return false;
				}
			}
			return true;
		}

		bool operator!=(const KanbanList& other) const {
			return !operator==(other);
		}

		std::string name;
		tsl::ordered_map<std::string, std::shared_ptr<KanbanTask>> tasks;
	};

	struct KanbanBoard
	{
		bool operator==(const KanbanBoard& other) const {
			if (this->created.timestamp() != other.created.timestamp() || this->last_modified.timestamp() != other.last_modified.timestamp() || this->name != other.name || this->description != other.description || this->labels.size() != other.labels.size() || this->list.size() != other.list.size()) {
				return false;
			}
			for (auto it = this->labels.begin(); it != this->labels.end(); ++it) {
				if (*it->second != *other.labels.at(it->first)) {
					return false;
				}
			}
			for (auto it = this->list.begin(); it != this->list.end(); ++it) {
				if (it->second != other.list.at(it->first)) {
					return false;
				}
			}
			return true;
		}

		bool operator!=(const KanbanBoard& other) const {
			return !(*this == other);
		}

		asap::datetime created;
		asap::datetime last_modified;
		unsigned int version;
		std::string checksum;

		std::string name;
		std::string description;
		tsl::ordered_map<std::string, std::shared_ptr<KanbanLabel>> labels;
		tsl::ordered_map<std::string, std::shared_ptr<KanbanList>> list;
	};
}