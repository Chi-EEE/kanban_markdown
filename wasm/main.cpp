#include <emscripten/bind.h>
#include <string>

class MyClass {
public:
    MyClass() : value(0) {}

    void setValue(int val) {
        value = val;
    }

    int getValue() const {
        return value;
    }

    std::string greet() const {
        return "Hello from MyClass!";
    }

private:
    int value;
};

// Bindings
EMSCRIPTEN_BINDINGS(my_class_example) {
    emscripten::class_<MyClass>("MyClass")
        .constructor()
        .function("setValue", &MyClass::setValue)
        .function("getValue", &MyClass::getValue)
        .function("greet", &MyClass::greet);
}
