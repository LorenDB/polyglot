module example5d;

// SPDX-FileCopyrightText: Matheus Catarino França
//
// SPDX-License-Identifier: GPL-3.0

struct Foo4
{
    @disable this();
    // not working, msg print null
    // void baz(const (char*) msg) @nogc nothrow {
    //     import core.stdc.stdio: printf;
    //     printf("Hello, %s! Welcome to my D-World.\n", msg);
    // }
}
// Works
void d_string(const (char*) msg) @nogc nothrow
{
    import core.stdc.stdio : printf;
    printf("Hello, %s! Welcome to my D World.\n", msg);
    // writeln or writefln also works, but print trash
    // D string return null
}

void helloWorldD() @safe
{
    import std.stdio;
    writeln("Hello World from D");
}
