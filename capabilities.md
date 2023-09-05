# Polyglot capabilities

This file documents what all Polyglot is and is not capable of doing right now.

## Source language support

These languages can be parsed by Polyglot and wrapped for use from other languages.

- C++

## Wrapper language support

Bindings can be created for use from these languages.

- D
- Rust

## Feature support chart

This chart shows what features Polyglot supports in its source languages. Support for a feature in a source language does not guarantee that Polyglot can replicate that feature in every wrapper  language (e.g. you cannot wrap classes for use in C because C does not support classes).

|Feature            |C++        |D      |Rust   |
|-------------------|-----------|-------|-------|
|Basic types        |yes        |       |       |
|Pointers           |partial    |       |       |
|Basic functions    |yes        |       |       |
|Enums              |yes        |       |       |
|Structs/classes    |partial    |       |       |
|noreturn           |yes        |       |       |
|nothrow            |           |       |       |
|Memory safety      | -         |       |       |