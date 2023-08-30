// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

mod test2;

fn main()
{
	unsafe
	{
		let foo: i32 = test2::square(8);
		println!("The square of 8 is {}", foo);
		test2::helloWorldCpp();
		// Rust has no concept of default arguments.
		// test2::defaultArg();
		test2::defaultArg(8);
		test2::testLoop();
		test2::checkMyFoo(test2::MyFoo::Foo);
		test2::checkMyFoo(test2::MyFoo::Bar);
		test2::checkMyFoo(test2::MyFoo::Baz);
		test2::checkMyFoo(test2::MyFoo::Foobar);
	}
}
