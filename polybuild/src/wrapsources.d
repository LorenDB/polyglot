// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

module polybuild.wrapsources;

@safe:

import std.algorithm;
import std.process;

import polybuild.buildfile;
import polybuild.cpphelper;

string[] wrapFiles(Sources sources, string outdir)
{
    string[] ret;

    {
        auto command = ["polyglot-cpp"] ~ sources.cppSources;
        if (sources.languages.d)
            command ~= ["--lang", "d"];
        if (sources.languages.rust)
            command ~= ["--lang", "rust"];
        command ~= ["--output-dir", outdir, "--", "-isystem", clangIncludePath];
        if (spawnProcess(command).wait() != 0)
            throw new Exception("polyglot-cpp failed");

        foreach (file; sources.cppSources)
        {
            if (sources.languages.d)
                ret ~= [outdir ~ '/' ~ file.getCppFileBasename ~ ".d"];
            if (sources.languages.rust)
                ret ~= [outdir ~ '/' ~ file.getCppFileBasename ~ ".rs"];
        }
    }

    // TODO: these don't exist yet :(
    foreach (file; sources.dSources)
    {
    }

    foreach (file; sources.rustSources)
    {
    }

    return ret;
}
