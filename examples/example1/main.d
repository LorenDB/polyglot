// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

import std;
import example1;

void main()
{
	writeln("The square of 8 is " ~ square(8).to!string);
	helloWorldCpp();
	defaultArg();
	defaultArg(8);
	testLoop();
	checkMyFoo(MyFoo.Foo);
	checkMyFoo(MyFoo.Bar);
	checkMyFoo(MyFoo.Baz);
	checkMyFoo(MyFoo.Foobar);
	Example1.foo();
	Example1.Test.foo();
}

unittest
{
	assert(square(2) == 4);
	assert(MyFoo.Baz == 100);
}
