// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

module cpphelper;

@safe:

import std.algorithm;
import std.process;

string clangIncludePath()
{
    auto task = execute(["clang++", "-print-resource-dir"]);
    if (task.status != 0)
        throw new Exception("Couldn't get clang resource path!");
    // the output has a trailing \n by default
    return task.output[0 .. $ - 1] ~ "/include";
}

bool isCppFile(string file)
{
    return file.endsWith(".cpp") || file.endsWith(".cxx") || file.endsWith(".c++") || file.endsWith(".cc") || file.endsWith(".C");
}

string getCppFileBasename(string file)
{
    if (file.endsWith(".cpp") || file.endsWith(".cxx") || file.endsWith(".c++"))
        return file[0 .. $ - 4];
    else if (file.endsWith(".cc"))
        return file[0 .. $ - 3];
    else if (file.endsWith(".C"))
        return file[0 .. $ - 2];
    else
        return file;
}