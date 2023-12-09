// SPDX-FileCopyrightText: Matheus Catarino França
//
// SPDX-License-Identifier: GPL-3.0
const std = @import("std");
const example5 = @import("example5");

// explicitly declare the C++ functions
extern "c++" fn _ZN4Foo13fooEv() void;
extern "c++" fn _ZN4Foo13barEl(i32) void;
extern "c++" fn _ZN4Foo1C2Ev() void;
extern "c++" fn _ZN4Foo1D2Ev() void;
extern "c++" fn _ZN8Example24Test3fooEv() void;

// D ABI
const helloWorldD = @extern(*const fn () callconv(.C) void, .{ .name = "_D9example5d11helloWorldDFNfZv" });
// or (explicit "library name")
// extern "druntime-ldc" fn _D9example5d11helloWorldDFNfZv() void;
// const helloWorldD = _D9example5d11helloWorldDFNfZv;

pub fn main() void {

    // 1. Basic functions
    example5.helloWorldCpp();
    helloWorldD();
    std.debug.print("The square of 8 is {}\n", .{example5.square(8)});

    // 2. Default arguments - not supported by Zig yet
    example5.defaultArg(8);

    // 3. Enums
    _ = example5.checkMyFoo(example5.MyFoo.Foo);
    _ = example5.checkMyFoo(example5.MyFoo.Bar);
    _ = example5.checkMyFoo(example5.MyFoo.Baz);
    _ = example5.checkMyFoo(example5.MyFoo.Foobar);

    // 4. Namespaces
    const Testfoo = _ZN8Example24Test3fooEv;
    Testfoo();

    // 5. Classes
    const Foo = _ZN4Foo1C2Ev; // constructor class Foo;
    Foo();
    defer _ZN4Foo1D2Ev(); // destructor of class Foo;
    const foo = _ZN4Foo13fooEv;
    const bar = _ZN4Foo13barEl;
    foo();
    bar(42);

    // zig str: []const u8
    // c str: [*:0]const u8
    const name: []const u8 = "Zig";
    const foo_msg = @extern(*const fn (msg: [*:0]const u8) callconv(.C) void, .{ .name = "_D9example5d8d_stringFNbNixPaZv" });

    // @ptrCast: Converts a pointer of one type to a pointer of another type.
    // The return type is the inferred result type.
    foo_msg(@ptrCast(name));
    // @constCast: Remove const qualifier from a pointer.
    example5.cpp_string(@ptrCast(@constCast(name))); // name ([]const u8 to *u8)

    // const f2 = Example1.Foo2;
    // f2.foo();
    // f2.bar(42);

    // const f2_1 = Example1.Foo2(32);
    // f2_1.foo();

    // 6. Structs
    var f3: example5.Foo3 = undefined;
    f3.i = 1;
    std.debug.print("{}\n", .{f3.i});

    // 7. Pointers passed from C++
    var longPtr = example5.getInt64Ptr();
    if (longPtr) |ptr| {
        std.debug.print("ptr value: {}\n", .{ptr.*});
        std.c.free(longPtr);
        longPtr = null;
    } else {
        std.debug.print("ptr value: null\n", .{});
    }

    if (longPtr == null) {
        std.debug.print("ptr value: null\n", .{});
    }

    // const f2ptr = Example1.getFoo2();
    // f2ptr.foo();
    // f2ptr.bar(8);

    std.debug.assert(example5.square(2) == 4);
    std.debug.assert(@intFromEnum(example5.MyFoo.Baz) == @as(i64, 100));
}

test {
    std.debug.assert(example5.square(2) == 4);
    std.debug.assert(example5.MyFoo.Baz == @as(i64, 100));
}
