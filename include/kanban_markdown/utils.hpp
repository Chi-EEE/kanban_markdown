#pragma once

#include <tsl/robin_map.h>
#include <tsl/robin_set.h>

#include "kanban.hpp"

namespace kanban_markdown::utils {
	static inline unsigned int kanban_get_counter_with_name(std::string name_str, tsl::robin_map<std::string, DuplicateNameTracker>& duplicate_name_tracker_map) {
		unsigned int counter = 1;
		if (duplicate_name_tracker_map.contains(name_str)) {
			auto& duplicate_name_tracker = duplicate_name_tracker_map[name_str];
			counter = duplicate_name_tracker.getHash();
		}
		else {
			auto& duplicate_name_tracker = duplicate_name_tracker_map[name_str];
			duplicate_name_tracker.counter = counter;
			duplicate_name_tracker.used_hash.insert(duplicate_name_tracker.counter);
		}
		return counter;
	}
}