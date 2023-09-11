module polybuild.build;

@safe:

import std.stdio;
import std.algorithm;
import std.process;
import std.file;
import std.array;
import std.conv : to;
import std.exception;
import std.range;

import polybuild.rusthelper;
import polybuild.buildfile;
import polybuild.wrapsources;
import polybuild.options;
import polybuild.build;

int buildApp(ref Buildfile buildfile, ref PolybuildOptions options)
{
    // ensure all required build directories exist
    if (!exists("build") || !isDir("build"))
        mkdir("build");
    if (!exists("build/pgwrappers") || !isDir("build/pgwrappers"))
        mkdir("build/pgwrappers");

    int retval;
    string[] objFiles;

    // wrap each file
    if (options.verbose)
        writeln("Wrapping " ~ buildfile.sources.to!string);
    buildfile.generatedSources = wrapFiles(buildfile.sources);

    // compile each file
    if (options.verbose)
        writeln("Compiling " ~ buildfile.allSources.to!string);
    if (buildfile.allSources.languages.cpp)
    {
        foreach (file; buildfile.allSourcesPlusWrappers.cppSources)
        {
            string objFile = "build/" ~ file ~ ".o";
            auto command = ["clang++", file, "-c", "-o", objFile];
            if (options.verbose)
                writeln("Executing " ~ command.join(' '));
            retval = spawnProcess(command).wait();
            if (retval != 0)
                return retval;

            objFiles ~= objFile;

            // TODO: add option to generate assembly; the following comment contains code for that
            // spawnProcess(["clang++", "-S", file, "-o", file ~ ".s"]);
        }
    }

    if (buildfile.allSources.languages.rust)
    {
        // Rust does *not* like looking for dependencies in a separate build directory, so we'll
        // temporarily copy the wrappers into the source tree (and immediately remove them afterward!)

        // We'll start by building a list of the files we need to copy.
        Sources wrappersInBuildTree;
        enum buildDirPrefix = "build/pgwrappers/";
        foreach (file; buildfile.generatedSources.rustSources)
        {
            if (!file.startsWith(buildDirPrefix))
                continue;
            wrappersInBuildTree ~= file[buildDirPrefix.length .. $];
        }

        // Now we copy them into the source tree...
        foreach (wrapper; wrappersInBuildTree)
        {
            // TODO: this doesn't work if we are keeping wrappers
            // if (wrapper.exists)
            //     throw new Exception("Couldn't copy wrapper into build tree");
            copy(buildDirPrefix ~ wrapper, wrapper);
        }

        // ...if needed, make sure they will be deleted no matter what...
        scope (exit)
            if (!buildfile.keepWrappers)
                foreach (wrapper; wrappersInBuildTree)
                    wrapper.remove();

        // ..and compile them.
        foreach (file; buildfile.allSources.rustSources)
        {
            string objFile = "build/" ~ file ~ ".o";
            auto command = ["rustc", file, "--emit", "obj", "-o", objFile];
            if (options.verbose)
                writeln("Executing " ~ command.join(' '));
            retval = spawnProcess(command).wait();
            if (retval != 0)
                return retval;

            objFiles ~= objFile;
        }
    }

    if (buildfile.allSources.languages.d)
    {
        // D wants to compile all the D files in one go, so we'll just do that now.
        string[] dFiles;
        foreach (fileEntry; chain(buildfile.allSourcesPlusWrappers.dSources))
            dFiles ~= fileEntry;

        auto command = ["ldc2"] ~ dFiles.sort.uniq.array ~ [
            "-c", "-of", "build/d_monolithic_obj_file.o"
        ];
        if (options.verbose)
            writeln("Executing " ~ command.join(' '));
        retval = spawnProcess(command).wait();
        // TODO: more assembly generation
        // spawnProcess(["ldc2"] ~ dFiles.sort.uniq.array ~ ["--output-s"]);

        if (retval != 0)
            return retval;

        objFiles ~= "build/d_monolithic_obj_file.o";
    }

    if (options.verbose)
        writeln("Linking " ~ objFiles.sort.uniq.array.to!string);
    retval = spawnProcess(["clang++"] ~ objFiles.sort.uniq.array ~ [
        "-lphobos2-ldc-shared", "-ldruntime-ldc-shared",
        getRustStandardLibraryPath(), "-o", "build/" ~ buildfile.projectName
    ]).wait();

    return retval;
}
