// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

module cpphelper;

@safe:

import std.process;

string clangIncludePath()
{
    auto task = execute(["clang++", "-print-resource-dir"]);
    if (task.status != 0)
        throw new Exception("Couldn't get clang resource path!");
    // the output has a trailing \n by default
    return task.output[0 .. $ - 1] ~ "/include";
}