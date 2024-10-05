#include <emscripten/bind.h>
#include <string>
#include <tl/expected.hpp>
#include <kanban_markdown/kanban_markdown.hpp>

class MyClass
{
public:
    MyClass() : value(0) {}

    void setValue(int val)
    {
        value = val;
    }

    int getValue() const
    {
        return value;
    }

    std::string greet() const
    {
        return "Hello from MyClass!";
    }

private:
    int value;
};

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

    emscripten::class_<kanban_markdown::KanbanBoard>("kanban_markdown::KanbanBoard")
        .property("color", &kanban_markdown::KanbanBoard::color)
        .property("created", &kanban_markdown::KanbanBoard::created)
        .property("last_modified", &kanban_markdown::KanbanBoard::last_modified)
        .property("version", &kanban_markdown::KanbanBoard::version)
        .property("checksum", &kanban_markdown::KanbanBoard::checksum)
        .property("name", &kanban_markdown::KanbanBoard::name)
        .property("description", &kanban_markdown::KanbanBoard::description)
        .property("labels", &kanban_markdown::KanbanBoard::labels)
        .property("list", &kanban_markdown::KanbanBoard::list);

    emscripten::class_<kanban_markdown::KanbanList>("kanban_markdown::KanbanList")
        .smart_ptr<std::shared_ptr<kanban_markdown::KanbanList>>("std::shared_ptr<kanban_markdown::KanbanList>")
        .property("checked", &kanban_markdown::KanbanList::checked)
        .property("counter", &kanban_markdown::KanbanList::counter)
        .property("name", &kanban_markdown::KanbanList::name)
        .property("tasks", &kanban_markdown::KanbanList::tasks);
    emscripten::register_vector<std::shared_ptr<kanban_markdown::KanbanList>>("KanbanListList");

    emscripten::class_<kanban_markdown::KanbanLabel>("kanban_markdown::KanbanLabel")
        .smart_ptr<std::shared_ptr<kanban_markdown::KanbanLabel>>("std::shared_ptr<kanban_markdown::KanbanLabel>")
        .property("color", &kanban_markdown::KanbanLabel::color)
        .property("name", &kanban_markdown::KanbanLabel::name)
        .property("tasks", &kanban_markdown::KanbanLabel::tasks);
    emscripten::register_vector<std::shared_ptr<kanban_markdown::KanbanLabel>>("KanbanLabelList");

    emscripten::class_<kanban_markdown::KanbanTask>("kanban_markdown::KanbanTask")
        .smart_ptr<std::shared_ptr<kanban_markdown::KanbanTask>>("std::shared_ptr<kanban_markdown::KanbanTask>")
        .property("name", &kanban_markdown::KanbanTask::name)
        .property("description", &kanban_markdown::KanbanTask::description)
        .property("labels", &kanban_markdown::KanbanTask::labels)
        .property("attachments", &kanban_markdown::KanbanTask::attachments)
        .property("checklist", &kanban_markdown::KanbanTask::checklist);
    emscripten::register_vector<std::shared_ptr<kanban_markdown::KanbanTask>>("KanbanTaskList");

    emscripten::class_<kanban_markdown::KanbanAttachment>("kanban_markdown::KanbanAttachment")
        .smart_ptr<std::shared_ptr<kanban_markdown::KanbanAttachment>>("std::shared_ptr<kanban_markdown::KanbanAttachment>")
        .property("name", &kanban_markdown::KanbanAttachment::name)
        .property("url", &kanban_markdown::KanbanAttachment::url);
    emscripten::register_vector<std::shared_ptr<kanban_markdown::KanbanAttachment>>("KanbanAttachmentList");

    emscripten::class_<kanban_markdown::KanbanChecklistItem>("kanban_markdown::KanbanChecklistItem")
        .smart_ptr<std::shared_ptr<kanban_markdown::KanbanChecklistItem>>("std::shared_ptr<kanban_markdown::KanbanChecklistItem>")
        .property("checked", &kanban_markdown::KanbanChecklistItem::checked)
        .property("name", &kanban_markdown::KanbanChecklistItem::name);
    emscripten::register_vector<std::shared_ptr<kanban_markdown::KanbanChecklistItem>>("KanbanChecklistItemList");

    emscripten::class_<Ok<kanban_markdown::KanbanBoard>>("Ok<kanban_markdown::KanbanBoard>")
        .property("value", &Ok<kanban_markdown::KanbanBoard>::value)
        .property("ok", &Ok<kanban_markdown::KanbanBoard>::ok);

    emscripten::class_<Err>("Err")
        .property("error", &Err::error)
        .property("ok", &Err::ok);

    emscripten::function("parse", &parse);
}
