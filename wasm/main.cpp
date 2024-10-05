#include <emscripten/bind.h>
#include <kanban_markdown/kanban_markdown.hpp>
#include <server.hpp>

template <typename T>
struct Ok
{
    const T value;
    const bool ok = true;
};

struct Err
{
    const std::string error;
    const bool ok = false;
};

emscripten::val parse(std::string md_string)
{
    auto maybe_kanban_board = kanban_markdown::reader::parse(md_string);
    if (!maybe_kanban_board)
        return emscripten::val(Err{maybe_kanban_board.error()});
    return emscripten::val(Ok<kanban_markdown::KanbanBoard>{maybe_kanban_board.value()});
}

emscripten::val update(kanban_markdown::KanbanBoard kanban_board, std::string input)
{
    yyjson_doc *doc = NULL;
    try
    {
        doc = yyjson_read(input.c_str(), input.size(), 0);
        if (doc == NULL)
        {
            return emscripten::val(Err{"The input is invalid; it must be in JSON format."});
        }
        yyjson_val *root = yyjson_doc_get_root(doc);
        if (root == NULL)
        {
            yyjson_doc_free(doc);
            return emscripten::val(Err{"The input is invalid; it must be in JSON format."});
        }
        server::KanbanCommandResult kanban_command_result = server::KanbanServer::commands(kanban_board, root, "");
        if (kanban_command_result.modified)
        {
            kanban_board.version += 1;
            kanban_board.last_modified = kanban_markdown::internal::now_utc();
        }
        yyjson_doc_free(doc);
        return emscripten::val(Ok<kanban_markdown::KanbanBoard>{kanban_board});
    }
    catch (const std::exception &e)
    {
        if (doc != NULL)
        {
            yyjson_doc_free(doc);
        }
        return emscripten::val(Err{e.what()});
    }
}

// Binding code
EMSCRIPTEN_BINDINGS(my_class_example)
{
    emscripten::class_<asap::datetime>("asap::datetime")
        .property("when", &asap::datetime::when);

    emscripten::class_<std::tm>("std::tm")
        .property("tm_sec", &std::tm::tm_sec)
        .property("tm_min", &std::tm::tm_min)
        .property("tm_hour", &std::tm::tm_hour)
        .property("tm_mday", &std::tm::tm_mday)
        .property("tm_mon", &std::tm::tm_mon)
        .property("tm_year", &std::tm::tm_year)
        .property("tm_wday", &std::tm::tm_wday)
        .property("tm_yday", &std::tm::tm_yday)
        .property("tm_isdst", &std::tm::tm_isdst)
        .property("tm_gmtoff", &std::tm::tm_gmtoff);

    emscripten::class_<kanban_markdown::KanbanBoard>("KanbanBoard")
        .property("color", &kanban_markdown::KanbanBoard::color)
        .property("created", &kanban_markdown::KanbanBoard::created)
        .property("last_modified", &kanban_markdown::KanbanBoard::last_modified)
        .property("version", &kanban_markdown::KanbanBoard::version)
        .property("checksum", &kanban_markdown::KanbanBoard::checksum)
        .property("name", &kanban_markdown::KanbanBoard::name)
        .property("description", &kanban_markdown::KanbanBoard::description)
        .property("labels", &kanban_markdown::KanbanBoard::labels)
        .property("list", &kanban_markdown::KanbanBoard::list);

    emscripten::class_<kanban_markdown::KanbanList>("KanbanList")
        .smart_ptr<std::shared_ptr<kanban_markdown::KanbanList>>("KanbanList")
        .property("checked", &kanban_markdown::KanbanList::checked)
        .property("counter", &kanban_markdown::KanbanList::counter)
        .property("name", &kanban_markdown::KanbanList::name)
        .property("tasks", &kanban_markdown::KanbanList::tasks);
    emscripten::register_vector<std::shared_ptr<kanban_markdown::KanbanList>>("KanbanList.List");

    emscripten::class_<kanban_markdown::KanbanLabel>("KanbanLabel")
        .smart_ptr<std::shared_ptr<kanban_markdown::KanbanLabel>>("KanbanLabel")
        .property("color", &kanban_markdown::KanbanLabel::color)
        .property("name", &kanban_markdown::KanbanLabel::name)
        .property("tasks", &kanban_markdown::KanbanLabel::tasks);
    emscripten::register_vector<std::shared_ptr<kanban_markdown::KanbanLabel>>("KanbanLabel.List");

    emscripten::class_<kanban_markdown::KanbanTask>("KanbanTask")
        .smart_ptr<std::shared_ptr<kanban_markdown::KanbanTask>>("KanbanTask")
        .property("name", &kanban_markdown::KanbanTask::name)
        .property("description", &kanban_markdown::KanbanTask::description)
        .property("labels", &kanban_markdown::KanbanTask::labels)
        .property("attachments", &kanban_markdown::KanbanTask::attachments)
        .property("checklist", &kanban_markdown::KanbanTask::checklist);
    emscripten::register_vector<std::shared_ptr<kanban_markdown::KanbanTask>>("KanbanTask.List");

    emscripten::class_<kanban_markdown::KanbanAttachment>("KanbanAttachment")
        .smart_ptr<std::shared_ptr<kanban_markdown::KanbanAttachment>>("KanbanAttachment")
        .property("name", &kanban_markdown::KanbanAttachment::name)
        .property("url", &kanban_markdown::KanbanAttachment::url);
    emscripten::register_vector<std::shared_ptr<kanban_markdown::KanbanAttachment>>("KanbanAttachment.List");

    emscripten::class_<kanban_markdown::KanbanChecklistItem>("KanbanChecklistItem")
        .smart_ptr<std::shared_ptr<kanban_markdown::KanbanChecklistItem>>("KanbanChecklistItem")
        .property("checked", &kanban_markdown::KanbanChecklistItem::checked)
        .property("name", &kanban_markdown::KanbanChecklistItem::name);
    emscripten::register_vector<std::shared_ptr<kanban_markdown::KanbanChecklistItem>>("KanbanChecklistItem.List");

    emscripten::class_<Ok<kanban_markdown::KanbanBoard>>("Ok<KanbanBoard>")
        .property("value", &Ok<kanban_markdown::KanbanBoard>::value)
        .property("ok", &Ok<kanban_markdown::KanbanBoard>::ok);

    emscripten::class_<Err>("Err")
        .property("error", &Err::error)
        .property("ok", &Err::ok);

    emscripten::class_<kanban_markdown::writer::markdown::Flags>("KanbanMarkdownFlags")
        .constructor()
        .property("github", &kanban_markdown::writer::markdown::Flags::github);

    emscripten::function("parse", &parse);
    emscripten::function("update", &update);
    emscripten::function("json_format_str", &kanban_markdown::writer::json::format_str);
    emscripten::function("markdown_format_str", &kanban_markdown::writer::markdown::format_str);
}
