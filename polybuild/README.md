# Polybuild

Polybuild is the build tool for Polyglot.

## Creating a `polybuild.yml` file

Polybuild operates off of instructions passed in a `polybuild.yml` file. The syntax for these files is absurdly simple; therefore, the reference is just a complete representation of the polybuild syntax:

```yml
# Specify the project name here
name: project-name
# This is a list of all the sources for your program.
sources:
  - source1.cpp
  - source2.d
  - source3.rs
  - main.d
# This is a list of sources that you want to compile into your app but exclude from wrapping.
exclude-wrapping:
  - internalhelper.d
  - otherhelper.rs
# If you want to keep the generated wrappers directly in your source tree, use this option.
keep-wrappers: true
```

## Building a polybuild project

Just `cd` to your project root (where your `polybuild.yml` is) and run `polybuild`. Polybuild will wrap the sources, compile, and link your executable into a top-level folder called `build`.

## Operational limitations

Polybuild currently doesn't support adding in third-party dependencies for any language. However, this is planned to be implemented at some point in the long run.
