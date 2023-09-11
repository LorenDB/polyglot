// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

import std.process;
import std.sumtype;
import std.file;
import std.stdio;

import dyaml;
import argparse;

import polybuild.options;
import polybuild.build;
import polybuild.buildfile;

int main(string[] args)
{
    PolybuildOptions options;
    if (!CLI!PolybuildOptions.parseArgs(options, args[1 .. $]))
        return 1;

    return options.command.match!((.BuildAction) {
        Buildfile buildfile = buildfileFromYAML(Loader.fromFile("polybuild.yml").load());
        return buildApp(buildfile, options);
    }, (.RunAction r) {
        Buildfile buildfile = buildfileFromYAML(Loader.fromFile("polybuild.yml").load());

        if (!r.skipBuild)
        {
            int retval = buildApp(buildfile, options);
            if (retval != 0)
                return retval;
        }

        if (!exists("./build/" ~ buildfile.projectName))
        {
            stderr.writeln("You must build " ~ buildfile.projectName ~ " before running it");
            return 1;
        }
        else
        {
            if (options.verbose)
                writeln("Running build/" ~ buildfile.projectName ~ "...");
            return spawnProcess("./build/" ~ buildfile.projectName).wait();
        }
    });
}
