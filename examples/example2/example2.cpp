// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

#include <iostream>
#include <string>

void helloWorldCpp()
{
    std::cout << "Hello world from C++" << std::endl;
}

void defaultArg(int i = 0)
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

//template<typename T>
//T squaredCppTemplate(T root)
//{
//    return root * root;
//}

void testLoop()
{
    for (int i = 0; i < 3; ++i)
        std::cout << "Loop is at " << i << std::endl;
}

//std::string cppStdString()
//{
//    return "foobar";
//}

//double foo1(float foo, float bar = 12.345, char baz = 'c', bool b = true, void *v = nullptr)
//{
//    return static_cast<double>(squaredCpp(foo * bar - static_cast<double>(baz)));
//}

//void foo2(char a, wchar_t b, char32_t c, char16_t d);
//void foo3(float a, double b, long double c);
//void foo4(short a, unsigned short b, int c, unsigned int d, long e, unsigned long f, long long g, unsigned long long h);

enum class MyFoo
{
    Foo,
    Bar,
    Baz = 100,
    Foobar,
};

int64_t checkMyFoo(MyFoo foo)
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

    }
    std::cout << static_cast<int64_t>(foo) << std::endl;
    return static_cast<int64_t>(foo);
}

//typedef int MyBadIntType;
//typedef int32_t MyGoodIntType;

//void foo5(int int0, int32_t int1, MyBadIntType int2, MyGoodIntType int3);

//class Foo1
//{
//    void foo();
//    void bar(int i);
//};
