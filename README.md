# Polyglot

Polyglot is the binding generator to end all binding generators. Its aim is to be able to create bindings from any normal compiled language to any other normal compiled language. To do this, Polyglot makes use of clang's libtooling and a whole lot of ugly-looking code generation :)

If you want a full list of what Polyglot can do right now, you can check the full [capabilities list](./capabilities.md).

## Getting started

To get started using Polyglot, you will need to make sure that you have the following dependencies installed:

- clang
- clang-devel
- the Rust toolchain (only tested using rustup for now)
- a D toolchain (currently only ldc2 is supported; this will change in the future)

Then run `./build.sh` from this repository. This will build `polyglot-cpp` (the C++ scanner and binding generator) and `polybuild` (the wrapper build tool) and install them for you. Once installed, you can use Polyglot by running `polybuild [sources-to-compile]`. For example projects to build, see the `tests/` folder in this repository.

## Operational limitations

There are a few known issues that have not yet been fixed:

- Polyglot leaves the generated bindings, the .o files, and the generated executable in your source tree. This will be fixed once polybuild is integrated with the scanner tools into a monolithic executable.
- Polyglot generates bindings for every supported language even if you are not compiling all of those languages. This will also be fixed by the monolithic executable.
- It is currently not possible to link in extra libraries other than the standard libraries for each language.
- The only supported platform is Linux. Polyglot could probably support Windows, macOS, and others as well, but I haven't been able to try it on those platforms; also, there are currently some hardcoded paths in polybuild that are Linux-specific that would need to be changed.

## How does it work?

Polyglot uses two modes of binding generation:

1. Direct bindings

   Direct bindings are bindings that simply create a direct pointer from the target language to the source language. This is achieved either by using `extern(C++)`-style calls ([D](https://dlang.org/spec/cpp_interface.html) and [Swift](https://www.swift.org/documentation/cxx-interop/)) or by manually overriding name mangling ([D](https://dlang.org/spec/pragma.html#mangle) and [Rust](https://doc.rust-lang.org/reference/items/external-blocks.html#the-link_name-attribute)). This is the best kind of binding available, since it provides a zero-overhead calling system. When you see libraries that expose all symbols via `extern "C"`-style linkage, that is an example of direct bindings as traditionally used.

2. Proxied bindings

   Proxied bindings are for languages that cannot be forced to speak each other's language; for example, C++ and Go. To create a binding here, Polyglot creates a proxy file in D. This proxy file contains direct bindings from C++ to D to allow D to call the C++ functions. It also contains a set of functions that have been mangled to look like Go functions; these functions simply call the C++ functions. Now all that is left to do is create a .go file that tells Go about the functions from D that are mangled like Go functions. Proxied bindings are suboptimal, since they require an extra function call every time you call into the binding, but they are certainly better than the alternative (nothing). Proxied bindings may also be used to facilitate automatic type conversion where needed (e.g. convert C++ `std::string`s into more generic string types for other languages).

## How can I help?

Currently, Polyglot is very minimal; it only supports C++, D, and Rust, and only fundamental types are supported. There is no support for classes and structs, templates are probably not going to be supported for a long time, and advanced things like coroutines and automatic type conversion are way off in the distance. If you are familiar with clang's libtooling, you can help by working on some basic things like structs and classes. Also, the current binding generator is a one-way street: it only supports wrapping C++. I'd appreciate any help building new language wrappers.

## Contact

You can discuss Polyglot on Matrix by joining [#polyglot:nheko.im](https://matrix.to/#/#polyglot:nheko.im).
