#pragma once

#include <string>

#include <yyjson.h>

#include <kanban_markdown/kanban_board.hpp>
#include <kanban_markdown/constants.hpp>
#include <kanban_markdown/internal.hpp>

namespace kanban_markdown::writer::json {
	inline void format(KanbanBoard kanban_board, yyjson_mut_doc* doc, yyjson_mut_val* root) {
		if (kanban_board.name.empty()) {
			yyjson_mut_obj_add_strncpy(doc, root, "name", constants::default_board_name.c_str(), constants::default_board_name.length());
		}
		else {
			yyjson_mut_obj_add_strncpy(doc, root, "name", kanban_board.name.c_str(), kanban_board.name.length());
		}
		if (kanban_board.description.empty()) {
			yyjson_mut_obj_add_strncpy(doc, root, "description", constants::default_description.c_str(), constants::default_description.length());
		} else {
			yyjson_mut_obj_add_strncpy(doc, root, "description", kanban_board.description.c_str(), kanban_board.description.length());
		}

		// Properties
		yyjson_mut_val* properties_obj = yyjson_mut_obj(doc);
		yyjson_mut_obj_add_strncpy(doc, properties_obj, "color", kanban_board.color.c_str(), kanban_board.color.length());
		yyjson_mut_obj_add_uint(doc, properties_obj, "version", kanban_board.version);
		yyjson_mut_obj_add_uint(doc, properties_obj, "created", kanban_board.created.timestamp());
		yyjson_mut_obj_add_uint(doc, properties_obj, "last_modified", kanban_board.last_modified.timestamp());
		yyjson_mut_obj_add_strncpy(doc, properties_obj, "checksum", kanban_board.checksum.c_str(), kanban_board.checksum.length());

		yyjson_mut_obj_add_val(doc, root, "properties", properties_obj);

		yyjson_mut_val* task_name_tracker_map_obj = yyjson_mut_obj(doc);
		for (const auto& [name, task_name_tracker] : kanban_board.task_name_tracker_map) {
			yyjson_mut_val* tracker_obj = yyjson_mut_obj(doc);
			yyjson_mut_obj_add_uint(doc, tracker_obj, "counter", task_name_tracker.counter);

			yyjson_mut_val* used_hash_arr = yyjson_mut_arr(doc);
			for (const auto& hash : task_name_tracker.used_hash) {
				yyjson_mut_arr_add_uint(doc, used_hash_arr, hash);
			}
			yyjson_mut_obj_add_val(doc, tracker_obj, "used_hash", used_hash_arr);

			yyjson_mut_val* name_val = yyjson_mut_strncpy(doc, name.c_str(), name.length());
			yyjson_mut_obj_add(task_name_tracker_map_obj, name_val, tracker_obj);
		}
		yyjson_mut_obj_add_val(doc, root, "task_name_tracker_map", task_name_tracker_map_obj);

		yyjson_mut_val* list_name_tracker_map_obj = yyjson_mut_obj(doc);
		for (const auto& [name, list_name_tracker] : kanban_board.list_name_tracker_map) {
			yyjson_mut_val* tracker_obj = yyjson_mut_obj(doc);
			yyjson_mut_obj_add_uint(doc, tracker_obj, "counter", list_name_tracker.counter);

			yyjson_mut_val* used_hash_arr = yyjson_mut_arr(doc);
			for (const auto& hash : list_name_tracker.used_hash) {
				yyjson_mut_arr_add_uint(doc, used_hash_arr, hash);
			}
			yyjson_mut_obj_add_val(doc, tracker_obj, "used_hash", used_hash_arr);

			yyjson_mut_val* name_val = yyjson_mut_strncpy(doc, name.c_str(), name.length());
			yyjson_mut_obj_add(list_name_tracker_map_obj, name_val, tracker_obj);
		}
		yyjson_mut_obj_add_val(doc, root, "list_name_tracker_map", list_name_tracker_map_obj);

		yyjson_mut_val* labels_arr = yyjson_mut_arr(doc);
		for (const auto& kanban_label : kanban_board.labels) {
			yyjson_mut_val* label_obj = yyjson_mut_obj(doc);
			yyjson_mut_obj_add_strncpy(doc, label_obj, "name", kanban_label->name.c_str(), kanban_label->name.length());
			yyjson_mut_obj_add_strncpy(doc, label_obj, "color", kanban_label->color.c_str(), kanban_label->color.length());

			yyjson_mut_val* tasks_arr = yyjson_mut_arr(doc);
			for (const auto& kanban_task : kanban_label->tasks) {
				yyjson_mut_val* task_obj = yyjson_mut_obj(doc);
				yyjson_mut_obj_add_strncpy(doc, task_obj, "name", kanban_task->name.c_str(), kanban_task->name.length());

				yyjson_mut_arr_add_val(tasks_arr, task_obj);
			}
			yyjson_mut_obj_add_val(doc, label_obj, "tasks", tasks_arr);
			yyjson_mut_arr_add_val(labels_arr, label_obj);
		}
		yyjson_mut_obj_add_val(doc, root, "labels", labels_arr);

		yyjson_mut_val* lists_arr = yyjson_mut_arr(doc);
		for (const auto& kanban_list : kanban_board.list) {
			yyjson_mut_val* list_obj = yyjson_mut_obj(doc);
			yyjson_mut_obj_add_strncpy(doc, list_obj, "name", kanban_list->name.c_str(), kanban_list->name.length());
			yyjson_mut_obj_add_uint(doc, list_obj, "counter", kanban_list->counter);

			yyjson_mut_val* tasks_arr = yyjson_mut_arr(doc);
			for (const auto& kanban_task : kanban_list->tasks) {
				yyjson_mut_val* task_obj = yyjson_mut_obj(doc);
				yyjson_mut_obj_add_strncpy(doc, task_obj, "name", kanban_task->name.c_str(), kanban_task->name.length());
				yyjson_mut_obj_add_bool(doc, task_obj, "checked", kanban_task->checked);
				yyjson_mut_obj_add_uint(doc, task_obj, "counter", kanban_task->counter);

				yyjson_mut_val* desc_arr = yyjson_mut_arr(doc);
				for (const auto& desc_line : kanban_task->description) {
					yyjson_mut_arr_add_strncpy(doc, desc_arr, desc_line.c_str(), desc_line.length());
				}
				yyjson_mut_obj_add_val(doc, task_obj, "description", desc_arr);

				yyjson_mut_val* task_labels_arr = yyjson_mut_arr(doc);
				for (const auto& label : kanban_task->labels) {
					yyjson_mut_val* task_label_obj = yyjson_mut_obj(doc);
					yyjson_mut_obj_add_strncpy(doc, task_label_obj, "name", label->name.c_str(), label->name.length());
					yyjson_mut_obj_add_strncpy(doc, task_label_obj, "color", label->color.c_str(), label->color.length());
					yyjson_mut_arr_add_val(task_labels_arr, task_label_obj);
				}
				yyjson_mut_obj_add_val(doc, task_obj, "labels", task_labels_arr);

				yyjson_mut_val* attachments_arr = yyjson_mut_arr(doc);
				for (const auto& attachment : kanban_task->attachments) {
					yyjson_mut_val* attachment_obj = yyjson_mut_obj(doc);
					yyjson_mut_obj_add_strncpy(doc, attachment_obj, "name", attachment->name.c_str(), attachment->name.length());
					yyjson_mut_obj_add_strncpy(doc, attachment_obj, "url", attachment->url.c_str(), attachment->url.length());
					yyjson_mut_arr_add_val(attachments_arr, attachment_obj);
				}
				yyjson_mut_obj_add_val(doc, task_obj, "attachments", attachments_arr);

				if (!kanban_task->checklist.empty())
				{
					yyjson_mut_val* checklist_arr = yyjson_mut_arr(doc);
					for (const auto& item : kanban_task->checklist) {
						yyjson_mut_val* checklist_item_obj = yyjson_mut_obj(doc);
						yyjson_mut_obj_add_strncpy(doc, checklist_item_obj, "name", item->name.c_str(), item->name.length());
						yyjson_mut_obj_add_bool(doc, checklist_item_obj, "checked", item->checked);
						yyjson_mut_arr_add_val(checklist_arr, checklist_item_obj);
					}
					yyjson_mut_obj_add_val(doc, task_obj, "checklist", checklist_arr);
				}

				yyjson_mut_arr_add_val(tasks_arr, task_obj);
			}
			yyjson_mut_obj_add_val(doc, list_obj, "tasks", tasks_arr);
			yyjson_mut_arr_add_val(lists_arr, list_obj);
		}
		yyjson_mut_obj_add_val(doc, root, "lists", lists_arr);
	}

	inline std::string format_str(KanbanBoard kanban_board) {
		yyjson_mut_doc* doc = yyjson_mut_doc_new(nullptr);
		yyjson_mut_val* root = yyjson_mut_obj(doc);
		yyjson_mut_doc_set_root(doc, root);

		format(kanban_board, doc, root);

		const char* json = yyjson_mut_write(doc, 0, nullptr);
		std::string result(json);
		yyjson_mut_doc_free(doc);
		return result;
	}
}
