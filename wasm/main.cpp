#include <emscripten/bind.h>
#include <string>
#include <tl/expected.hpp>

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

// Function to create MyClass using tl::expected and return MyClassResult
emscripten::val createMyClass(int value)
{
    if (value < 0)
        return emscripten::val(Err{"Value cannot be negative"});
    MyClass obj;
    obj.setValue(value);
    return emscripten::val(Ok<MyClass>{obj});
}

// Binding code
EMSCRIPTEN_BINDINGS(my_class_example)
{
    emscripten::class_<MyClass>("MyClass")
        .constructor()
        .function("setValue", &MyClass::setValue)
        .function("getValue", &MyClass::getValue)
        .function("greet", &MyClass::greet);

    emscripten::class_<Ok<MyClass>>("Ok")
        .property("value", &Ok<MyClass>::value)
        .property("ok", &Ok<MyClass>::ok);

    emscripten::class_<Err>("Err")
        .property("error", &Err::error)
        .property("ok", &Err::ok);

    emscripten::function("createMyClass", &createMyClass);
}
