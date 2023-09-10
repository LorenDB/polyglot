// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

module rusthelper;

@safe:

import std.file;
import std.path;

@trusted string getRustStandardLibraryPath()
{
    string[] rustLibDirs = 
        ["/usr/lib64/rustlib/x86_64-unknown-linux-gnu/lib",
         "/usr/lib/rustlib/x86_64-unknown-linux-gnu/lib",
         "~/.rustup/toolchains/stable-x86_64-unknown-linux-gnu/lib/rustlib/x86_64-unknown-linux-gnu/lib".expandTilde];
    foreach (dir; rustLibDirs)
    {
        if (dir.exists && dir.isDir)
        {
            foreach (fileEntry; dirEntries(dir, "libstd-*.so", SpanMode.shallow))
                return fileEntry;
        } 
    }

    return "";
}