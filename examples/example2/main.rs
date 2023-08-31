// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

mod example2;

fn main()
{
	unsafe
	{
		let foo: i32 = example2::square(8);
		println!("The square of 8 is {}", foo);
		example2::helloWorldCpp();
		// Rust has no concept of default arguments.
		// example2::defaultArg();
		example2::defaultArg(8);
		example2::testLoop();
		example2::checkMyFoo(example2::MyFoo::Foo);
		example2::checkMyFoo(example2::MyFoo::Bar);
		example2::checkMyFoo(example2::MyFoo::Baz);
		example2::checkMyFoo(example2::MyFoo::Foobar);
	}
}
