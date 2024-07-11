#pragma once

#include <string>
#include <vector>
#include <memory>

#include <asap/asap.h>
#include <tsl/ordered_map.h>

namespace kanban_markdown {
	struct KanbanAttachment
	{
		bool operator==(const KanbanAttachment& other) const {
			return name == other.name && url == other.url;
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
			return checked == other.checked && name == other.name;
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
			if (name != other.name) {
				return false;
			}
			return true;
		}

		bool operator!=(const KanbanLabel& other) const {
			return !(*this == other);
		}

		std::string name;
		std::vector<std::shared_ptr<KanbanTask>> tasks;
	};

	struct KanbanTask
	{
		bool operator==(const KanbanTask& other) const {
			if (checked != other.checked || name != other.name || description.size() != other.description.size() || labels.size() != other.labels.size() || attachments.size() != other.attachments.size() || checklist.size() != other.checklist.size()) {
				return false;
			}
			for (size_t i = 0; i < description.size(); i++) {
				if (description[i] != other.description[i]) {
					return false;
				}
			}
			for (size_t i = 0; i < labels.size(); i++) {
				if (*labels[i] != *other.labels[i]) {
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
		std::vector<std::shared_ptr<KanbanLabel>> labels;
		std::vector<KanbanAttachment> attachments;
		std::vector<KanbanChecklistItem> checklist;
	};

	struct KanbanList
	{
		bool operator==(const KanbanList& other) const {
			if (name != other.name || tasks.size() != other.tasks.size()) {
				return false;
			}
			// Go through both maps and compare the tasks
			for (auto& [task_name, task] : tasks) {
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
			if (created.timestamp() != other.created.timestamp() || last_updated.timestamp() != other.last_updated.timestamp() || name != other.name || description != other.description || labels.size() != other.labels.size() || list.size() != other.list.size()) {
				return false;
			}
			for (auto it = labels.begin(); it != labels.end(); ++it) {
				if (*it->second != *other.labels.at(it->first)) {
					return false;
				}
			}
			for (auto it = list.begin(); it != list.end(); ++it) {
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
		asap::datetime last_updated;
		std::string name;
		std::string description;
		tsl::ordered_map<std::string, std::shared_ptr<KanbanLabel>> labels;
		tsl::ordered_map<std::string, KanbanList> list;
	};
}