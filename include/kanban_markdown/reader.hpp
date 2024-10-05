#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <cctype>
#include <locale>

#include <fmt/format.h>
#include <tl/expected.hpp>

#include <md4c.h>
#include <pugixml.hpp>
#include <yaml-cpp/yaml.h>

#include <kanban_markdown/kanban_board.hpp>
#include <kanban_markdown/constants.hpp>

#include <kanban_markdown/internal.hpp>
#include <kanban_markdown/utils.hpp>

#include <kanban_markdown/reader/internal.hpp>
#include <kanban_markdown/reader/builder.hpp>

#include <kanban_markdown/reader/section/none.hpp>
#include <kanban_markdown/reader/section/task.hpp>
#include <kanban_markdown/reader/section/label.hpp>
#include <kanban_markdown/reader/section/properties.hpp>

namespace kanban_markdown::reader
{
	using namespace kanban_markdown::internal;

	namespace internal
	{
		static inline tl::expected<pugi::xml_document, std::variant<nullptr_t, std::string>> read_xml(KanbanReader *kanban_reader, std::string text_content)
		{
			auto end_of_current_html_tag = std::mismatch(constants::end_of_html_tag.begin(), constants::end_of_html_tag.end(), text_content.begin());
			if (end_of_current_html_tag.first != constants::end_of_html_tag.end())
			{
				// Start of the HTML tag
				kanban_reader->html_tags.push_back(text_content);
			}
			else
			{
				// End of the HTML tag
				std::string opening_html_tag = kanban_reader->html_tags.back();
				kanban_reader->html_tags.pop_back();
				std::string complete_html_tag = opening_html_tag + text_content;
				pugi::xml_document xml;
				pugi::xml_parse_result result = xml.load_buffer(complete_html_tag.c_str(), complete_html_tag.size());
				if (!result)
				{
					return tl::make_unexpected(result.description());
				}
				return xml;
			}
			return tl::make_unexpected(nullptr);
		}

		static int enter_block_callback(MD_BLOCKTYPE type, void *detail, void *userdata)
		{
			KanbanReader *kanban_reader = static_cast<KanbanReader *>(userdata);
			switch (type)
			{
			case MD_BLOCK_H:
			{
				MD_BLOCK_H_DETAIL *header_detail = static_cast<MD_BLOCK_H_DETAIL *>(detail);
				kanban_reader->previous_headers.push_back(kanban_reader->header_level);
				kanban_reader->header_level = header_detail->level;
				break;
			}
			case MD_BLOCK_LI:
			{
				if (kanban_reader->list_item_level == 1)
				{
					kanban_reader->sub_list_item_count++;
				}
				kanban_reader->list_item_level++;
				break;
			}
			default:
			{
				break;
			}
			}
			return 0;
		}

		static int leave_block_callback(MD_BLOCKTYPE type, void *detail, void *userdata)
		{
			KanbanReader *kanban_reader = static_cast<KanbanReader *>(userdata);
			switch (type)
			{
			case MD_BLOCK_H:
			{
				MD_BLOCK_H_DETAIL *header_detail = static_cast<MD_BLOCK_H_DETAIL *>(detail);
				kanban_reader->header_level = kanban_reader->previous_headers.back();
				kanban_reader->previous_headers.pop_back();
				break;
			}
			case MD_BLOCK_UL:
			{
				if (kanban_reader->list_item_level == 0)
				{
					kanban_reader->state = KanbanState::Board;
				}
				break;
			}
			case MD_BLOCK_LI:
			{
				if (kanban_reader->list_item_level == 1)
				{
					kanban_reader->sub_list_item_count = 0;
				}
				kanban_reader->list_item_level--;
				break;
			}
			default:
			{
				break;
			}
			}
			return 0;
		}

		static int enter_span_callback(MD_SPANTYPE type, void *detail, void *userdata)
		{
			KanbanReader *kanban_reader = static_cast<KanbanReader *>(userdata);
			switch (type)
			{
			case MD_SPAN_A:
			{
				kanban_reader->currently_reading_link = true;
				MD_SPAN_A_DETAIL *a_detail = static_cast<MD_SPAN_A_DETAIL *>(detail);
				if (kanban_reader->state == KanbanState::Board)
				{
					if (kanban_reader->content_section.task_read_state == TaskReadState::Attachments)
					{
						ListSection *current_list = kanban_reader->content_section.current_list;
						if (current_list == nullptr)
						{
							std::cerr << "Current board is null." << '\n';
							return 0;
						}
						CurrentTask &current_task = current_list->current_task;
						const std::string current_task_index = fmt::format("{}-{}", current_task.name, current_task.counter);
						TaskDetail &current_task_detail = current_list->task_details[current_task_index];
						KanbanAttachment attachment;
						attachment.url = std::string(a_detail->href.text, a_detail->href.size);
						current_task_detail.attachments.push_back(attachment);
						current_list->current_attachment = &current_task_detail.attachments.back();
					}
				}
				break;
			}
			default:
			{
				break;
			}
			}
			return 0;
		}

		static int leave_span_callback(MD_SPANTYPE type, void *detail, void *userdata)
		{
			KanbanReader *kanban_reader = static_cast<KanbanReader *>(userdata);
			switch (type)
			{
			case MD_SPAN_A:
			{
				kanban_reader->currently_reading_link = false;
				break;
			}
			default:
			{
				break;
			}
			}
			return 0;
		}

		static void parseSection(KanbanReader *kanban_reader, const std::string &text_content)
		{
			// Check if it is not a main header
			if (kanban_reader->header_level == 1 || kanban_reader->header_level == 2)
			{
				section::none::read_text(kanban_reader, text_content);
				return;
			}
			switch (kanban_reader->state)
			{
			case KanbanState::None:
				section::none::read_text(kanban_reader, text_content);
				break;
			case KanbanState::Labels:
				section::label::read_text(kanban_reader, text_content);
				break;
			case KanbanState::Board:
				section::task::read_text(kanban_reader, text_content);
				break;
			default:
			{
				break;
			}
			}
		}

		static int text_callback(MD_TEXTTYPE type, const MD_CHAR *text, MD_SIZE size, void *userdata)
		{
			KanbanReader *kanban_reader = static_cast<KanbanReader *>(userdata);
			std::string text_content(text, size);
			switch (type)
			{
			case MD_TEXT_NORMAL:
			{
				parseSection(kanban_reader, text_content);
				break;
			}
			case MD_TEXT_HTML:
			{
				auto maybe_xml = internal::read_xml(kanban_reader, text_content);

				if (!maybe_xml)
				{
					if (std::holds_alternative<std::string>(maybe_xml.error()))
					{
						std::string error = std::get<std::string>(maybe_xml.error());
						// TODO: error here
					}
					break;
				}

				const pugi::xml_document &xml = maybe_xml.value();

				switch (kanban_reader->state)
				{
				case KanbanState::Board:
					section::task::read_html(kanban_reader, xml);
					break;
				case KanbanState::Labels:
					section::label::read_html(kanban_reader, xml);
					break;
				}
				break;
			}
			default:
			{
				break;
			}
			}
			return 0;
		}

		static void debug(const char *msg, void *userdata)
		{
			KanbanBoard *kanban_board = static_cast<KanbanBoard *>(userdata);
			std::cout << msg << '\n';
		}

	}

	static inline tl::expected<KanbanBoard, std::string> parse(std::string md_string)
	{
		internal::KanbanReader kanban_reader;

		auto process_properties = [&](const std::string &delimiter) -> tl::expected<nullptr_t, std::string>
		{
			std::size_t end_of_properties = md_string.find(delimiter, delimiter.length());
			if (end_of_properties == std::string::npos)
				return tl::make_unexpected("Invalid Markdown file. Properties are not closed.");

			const std::string properties = md_string.substr(delimiter.length(), end_of_properties - delimiter.length());
			tl::expected<nullptr_t, std::string> properties_read_result = section::properties::read(kanban_reader, properties);

			if (!properties_read_result.has_value())
				return tl::make_unexpected(properties_read_result.error());

			md_string = md_string.substr(end_of_properties + delimiter.length());
			kanban_reader.read_properties = true;

			return nullptr;
		};

		if (md_string.substr(0, 5) == "---\r\n")
		{
			tl::expected<nullptr_t, std::string> result = process_properties("---\r\n");
			if (!result.has_value())
				return tl::make_unexpected(result.error());
		}
		else if (md_string.substr(0, 4) == "---\n")
		{
			tl::expected<nullptr_t, std::string> result = process_properties("---\n");
			if (!result.has_value())
				return tl::make_unexpected(result.error());
		}
		else
		{
			auto now = now_utc();
			kanban_reader.color = constants::default_color;
			kanban_reader.created = now;
			kanban_reader.last_modified = now;
			kanban_reader.version = 0;
		}

		MD_PARSER parser;
		parser.abi_version = 0;
		parser.enter_block = internal::enter_block_callback;
		parser.leave_block = internal::leave_block_callback;
		parser.enter_span = internal::enter_span_callback;
		parser.leave_span = internal::leave_span_callback;
		parser.text = internal::text_callback;
		parser.flags = 0;
		parser.syntax = nullptr;
		parser.debug_log = internal::debug;

		int result = md_parse(md_string.c_str(), md_string.size(), &parser, &kanban_reader);
		if (result != 0)
		{
			std::cerr << "Error parsing Markdown text." << std::endl;
		}

		KanbanBoard kanban_board = builder::create(kanban_reader);
		return kanban_board;
	}
}