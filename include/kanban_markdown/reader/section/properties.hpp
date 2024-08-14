#pragma once

#include <asap/asap.h>
#include <tl/expected.hpp>
#include <yaml-cpp/yaml.h>

#include <kanban_markdown/constants.hpp>
#include <kanban_markdown/reader/internal.hpp>

namespace kanban_markdown::reader::section::properties {
	using namespace kanban_markdown::reader::internal;

	static inline tl::expected<nullptr_t, std::string> read(KanbanReader& kanban_parser, const std::string &properties) {
		YAML::Node config = YAML::Load(properties);
		const std::string color = config["Color"].as<std::string>();
		if (color.empty()) {
			return tl::make_unexpected("Invalid Markdown file. [Color] property is empty.");
		}

		const std::string created = config["Created"].as<std::string>();
		if (created.empty()) {
			return tl::make_unexpected("Invalid Markdown file. [Created] property is empty.");
		}
		const int version = config["Version"].as<unsigned int>();

		const std::string last_modified = config["Last Modified"].as<std::string>();
		if (last_modified.empty()) {
			return tl::make_unexpected("Invalid Markdown file. [Last Modified] property is empty.");
		}

		const std::string checksum = config["Checksum"].as<std::string>();
		if (checksum.empty()) {
			return tl::make_unexpected("Invalid Markdown file. [Checksum] property is empty.");
		}

		asap::datetime created_datetime(created, constants::time_format);
		if (created_datetime.timestamp() == 0) {
			return tl::make_unexpected("Invalid Markdown file. [Created] property has invalid seconds.");
		}

		asap::datetime last_modified_datetime(last_modified, constants::time_format);
		if (last_modified_datetime.timestamp() == 0) {
			return tl::make_unexpected("Invalid Markdown file. [Last Modified] property has invalid seconds.");
		}

		kanban_parser.color = color;
		kanban_parser.created = created_datetime;
		kanban_parser.last_modified = last_modified_datetime;

		kanban_parser.version = version;

		kanban_parser.checksum = checksum;

		return nullptr;
	}
}