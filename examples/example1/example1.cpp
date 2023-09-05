// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

#include <iostream>
#include <string>

void helloWorldCpp()
{
    std::cout << "Hello world from C++" << std::endl;
}

void defaultArg(int64_t i = 0)
{
    if (i == 0)
        std::cout << "Boo, you didn't pass an argument." << std::endl;
    else
        std::cout << "Good job! You were original and sent " << i << " to defaultArg()!" << std::endl;
}

int32_t square(int32_t root)
{
    return root * root;
}

// template<typename T>
// T squaredCppTemplate(T root)
//{
//     return root * root;
// }

// std::string cppStdString()
//{
//     return "foobar";
// }

// double foo1(float foo, float bar = 12.345, char baz = 'c', bool b = true, void *v = nullptr)
//{
//     return static_cast<double>(squaredCpp(foo * bar - static_cast<double>(baz)));
// }

// void foo2(char a, wchar_t b, char32_t c, char16_t d);
// void foo3(float a, double b, long double c);
// void foo4(short a, unsigned short b, int c, unsigned int d, long e, unsigned long f, long long g, unsigned long long h);

enum class MyFoo
{
    Foo,
    Bar,
    Baz = 100,
    Foobar,
};

int64_t checkMyFoo(MyFoo foo) noexcept
{
    switch (foo)
    {
    case MyFoo::Foo:
        std::cout << "Foo: ";
        break;
    case MyFoo::Bar:
        std::cout << "Bar: ";
        break;
    case MyFoo::Baz:
        std::cout << "Baz: ";
        break;
    case MyFoo::Foobar:
        std::cout << "Foobar: ";
        break;
    default:
        std::cout << "Unrecognized type: ";
    }
    std::cout << static_cast<int64_t>(foo) << std::endl;
    return static_cast<int64_t>(foo);
}

int64_t *getInt64Ptr() noexcept
{
    return new int64_t{2};
}

// typedef int MyBadIntType;
// typedef int32_t MyGoodIntType;

// void foo5(int int0, int32_t int1, MyBadIntType int2, MyGoodIntType int3);

class Foo1
{
public:
    Foo1() { std::cout << "Foo1::Foo1()" << std::endl; }
    ~Foo1() { std::cout << "Foo1::~Foo1()" << std::endl; }

    Foo1(const Foo1 &) = delete;
    Foo1(Foo1 &&) = delete;

    void foo() { std::cout << "Foo1::foo()" << std::endl; }
    void bar(int64_t i) { std::cout << "Foo1::bar(" << i << ")" << std::endl; }
};

namespace Example1
{
    namespace Test
    {
        void foo()
        {
            std::cout << "Called Example1::Test::foo()" << std::endl;
        }
    } // namespace Test

    void foo()
    {
        std::cout << "Called Example1::foo()" << std::endl;
    }

    class Foo2
    {
    public:
        Foo2() { std::cout << "Creating new Example1::Foo2" << std::endl; }

        Foo2(int32_t i) { m_foo = i; }

        ~Foo2() { std::cout << "Destroying Example1::Foo2" << std::endl; }

        Foo2(const Foo2 &) = delete;
        Foo2(Foo2 &&) = delete;

        void foo()
        {
            std::cout << "This Example1::Foo2 object has m_foo set to " << m_foo << " and m_bar set to " << m_bar
                      << std::endl;
        }
        void bar(int32_t a /*, char b*/) { std::cout << "Example1::Foo2::bar(): value of a: " << a << std::endl; }

        int32_t m_foo = 1;
        double m_bar = 2.0;
    };

    Foo2 *getFoo2()
    {
        return new Foo2{18};
    }
} // namespace Example1

struct Foo3
{
    int32_t i;
    char c;
};

void polyglot_make_sure_symbols_are_kept_by_the_linker()
{
    Foo1 f;
    f.foo();
    f.bar(1);

    Example1::Foo2 f2;
    f2.foo();
    f2.bar(3);
    Example1::Foo2 f3(42);
}
