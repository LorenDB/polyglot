module wrapsources;

@safe:

import std.algorithm;
import std.process;

import cpphelper;

string[] wrapFile(string file)
{
    if (file.endsWith(".cpp") || file.endsWith(".cxx") || file.endsWith(".c++") || file.endsWith(".cc") || file.endsWith(".C"))
    {
        if (spawnProcess(["polyglot-cpp", file, "--", "-isystem", clangIncludePath]).wait() != 0)
            throw new Exception("polyglot-cpp failed");

        string basename;
        if (file.endsWith(".cpp") || file.endsWith(".cxx") || file.endsWith(".c++"))
            basename = file[0 .. $ - 4];
        else if (file.endsWith(".cc"))
            basename = file[0 .. $ - 3];
        else if (file.endsWith(".C"))
            basename = file[0 .. $ - 2];
        return ["build/pgwrappers/" ~ basename ~ ".d", "build/pgwrappers/" ~ basename ~ ".rs"];
    }

    // TODO: these don't exist yet :(
    else if (file.endsWith(".d"))
    {
        // if (spawnProcess(["polyglot-d", file]).wait() != 0)
        //     throw new Exception("polyglot-d failed");

        // return ["build/pgwrappers/" ~ file[0 .. $ - 2] ~ ".h", "build/pgwrappers/" ~ file[0 .. $ - 2] ~ ".rs"];
        return [];
    }
    else if (file.endsWith(".rs"))
    {
        // if (spawnProcess(["polyglot-rs", file]).wait() != 0)
        //     throw new Exception("polyglot-rs failed");

        // return ["build/pgwrappers/" ~ file[0 .. $ - 3] ~ ".h", "build/pgwrappers/" ~ file[0 .. $ - 3] ~ ".d"];
        return [];
    }
    else
        throw new Exception("Couldn't identify file type: " ~ file);
}