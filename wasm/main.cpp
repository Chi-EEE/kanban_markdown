#include <emscripten/emscripten.h>
#include <emscripten/bind.h>

#include <kanban_markdown/kanban_markdown.hpp>

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN
#endif

int main() {
  return 0;
}

EMSCRIPTEN_BINDINGS(my_class_example) {
  emscripten::class_<kanban_markdown::KanbanBoard>("KanbanBoard");
}

EMSCRIPTEN_BINDINGS(my_module) {
    emscripten::function("parse", &kanban_markdown::reader::parse);
}

EXTERN EMSCRIPTEN_KEEPALIVE void myFunction(int argc, char ** argv) {
    printf("MyFunction Called\n");
}