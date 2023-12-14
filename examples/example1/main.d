// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

import std;
import example1;

void main()
{
	// 1. Basic functions
	helloWorldCpp();
	writeln("The square of 8 is " ~ square(8).to!string);

	// 2. Default arguments
	defaultArg();
	defaultArg(8);

	// 3. Enums
	checkMyFoo(MyFoo.Foo);
	checkMyFoo(MyFoo.Bar);
	checkMyFoo(MyFoo.Baz);
	checkMyFoo(MyFoo.Foobar);

	// 4. Namespaces
	Example1.foo();
	Example1.Test.foo();

	// 5. Classes
	auto f1 = new Foo1;
	f1.foo();
	f1.bar(42);

	auto f2 = new Example1.Foo2;
	f2.foo();
	f2.bar(42);

	auto f2_1 = new Example1.Foo2(32);
	f2_1.foo();

	// 6. Structs
	auto f3 = Foo3();
	f3.i = 1;
	f3.i.writeln;

	// 7. Pointers passed from C++
	long *longPtr = getInt64Ptr();
	writeln(*longPtr);

	// auto f2ptr = Example1.getFoo2();
	// f2ptr.foo();
	// f2ptr.bar(8);

	// 8. std::string support
	cppStdString().writeln;
}

unittest
{
	assert(square(2) == 4);
	assert(MyFoo.Baz == 100);
}
