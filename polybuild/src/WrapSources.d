module wrapsources;

@safe:

import std.algorithm;
import std.process;

import buildfile;
import cpphelper;

string[] wrapFiles(Sources sources)
{
    string[] ret;
    auto langs = sources.languages;
    foreach (file; sources)
    {
        if (file.isCppFile)
        {
            auto command = ["polyglot-cpp", file];
            if (langs.d)
                command ~= ["--lang", "d"];
            if (langs.rust)
                command ~= ["--lang", "rust"];
            command ~= ["--", "-isystem", clangIncludePath];
            if (spawnProcess(command).wait() != 0)
                throw new Exception("polyglot-cpp failed");

            if (langs.d)
                ret ~= ["build/pgwrappers/" ~ file.getCppFileBasename ~ ".d"];
            if (langs.rust)
                ret ~= ["build/pgwrappers/" ~ file.getCppFileBasename ~ ".rs"];
        }

        // TODO: these don't exist yet :(
        else if (file.endsWith(".d"))
        {
            // if (spawnProcess(["polyglot-d", file]).wait() != 0)
            //     throw new Exception("polyglot-d failed");

            // ret ~= ["build/pgwrappers/" ~ file[0 .. $ - 2] ~ ".h", "build/pgwrappers/" ~ file[0 .. $ - 2] ~ ".rs"];
        }
        else if (file.endsWith(".rs"))
        {
            // if (spawnProcess(["polyglot-rs", file]).wait() != 0)
            //     throw new Exception("polyglot-rs failed");

            // ret ~= ["build/pgwrappers/" ~ file[0 .. $ - 3] ~ ".h", "build/pgwrappers/" ~ file[0 .. $ - 3] ~ ".d"];
        }
        else
            throw new Exception("Couldn't identify file type: " ~ file);
    }
    return ret;
}