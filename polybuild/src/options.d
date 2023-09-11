module polybuild.options;

@safe:

import std.sumtype;

import argparse;

enum PolybuildAction
{
    NoAction,

    Build,
    Run,
}

@(Command("build").ShortDescription("Build the project"))
struct BuildAction
{
}

@(Command("run").ShortDescription("Build and run the project"))
struct RunAction
{
    @(NamedArgument("skip-build").Description("Run the executable without building"))
    bool skipBuild;
}

struct PolybuildOptions
{
    @(NamedArgument(["v", "verbose"]).Description("Use verbose output"))
    bool verbose;

    @SubCommands SumType!(BuildAction, RunAction) command;
}
