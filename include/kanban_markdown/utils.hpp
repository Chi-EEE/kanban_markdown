#pragma once

#include <tsl/robin_map.h>
#include <tsl/robin_set.h>

#include "kanban.hpp"

namespace kanban_markdown::utils {
	static inline unsigned int kanban_get_counter_with_name(std::string name_str, tsl::robin_map<std::string, TaskNameTracker>& task_name_tracker_map) {
		unsigned int counter = 1;
		if (task_name_tracker_map.contains(name_str)) {
			auto& task_name_tracker = task_name_tracker_map[name_str];
			do {
				counter = ++task_name_tracker.counter;
			} while (task_name_tracker.used_hash.contains(task_name_tracker.counter));
			task_name_tracker.used_hash.insert(task_name_tracker.counter);
		}
		else {
			auto& task_name_tracker = task_name_tracker_map[name_str];
			task_name_tracker.counter = counter;
			task_name_tracker.used_hash.insert(task_name_tracker.counter);
		}
		return counter;
	}

}