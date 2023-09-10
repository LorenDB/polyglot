// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

mod example2;

fn main()
{
	unsafe
	{
		// 1. Basic functions
		example2::helloWorldCpp();
		let foo: i32 = example2::square(8);
		println!("The square of 8 is {}", foo);

		// 2. Default arguments
		// Rust has no concept of default arguments.
		// example2::defaultArg();
		example2::defaultArg(8);

		// 3. Enums
		example2::checkMyFoo(example2::MyFoo::Foo);
		example2::checkMyFoo(example2::MyFoo::Bar);
		example2::checkMyFoo(example2::MyFoo::Baz);
		example2::checkMyFoo(example2::MyFoo::Foobar);

		// 4. Namespaces
		example2::Example2::foo();

		// 5. Classes/structs
		let f3 = example2::Foo3{i: 42};
		println!("f3.i = {}", f3.i);
	}

	let mut f1 = example2::Foo1{sixteen: 16};
	println!("f1.sixteen = {}", f1.sixteen);
	f1.foo();
	f1.bar(7);
}
