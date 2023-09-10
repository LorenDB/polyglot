// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

// @safe:

import std.stdio;
import std.algorithm;
import std.process;
import std.file;
import std.array;
import std.conv: to;
import std.exception;
import std.range;

import dyaml;

import rusthelper;
import buildfile;
import wrapsources;

int main(string[] args)
{
	if (args.length > 1)
	{
		writeln("Polybuild currently does not take any arguments!");
		return -1;
	}

	Buildfile buildfile = buildfileFromYAML(Loader.fromFile("polybuild.yml").load());

	// ensure all required build directories exist
	if (!exists("build") || !isDir("build"))
		mkdir("build");
	if (!exists("build/pgwrappers") || !isDir("build/pgwrappers"))
		mkdir("build/pgwrappers");

	int retval;
	string[] objFiles;

	// wrap each file
	writeln("Wrapping " ~ buildfile.sources.to!string);
	foreach (file; buildfile.sources)
		buildfile.generatedSources ~= wrapFile(file);

	// compile each file
	writeln("Compiling " ~ buildfile.allSources.to!string);
	if (buildfile.allSources.languages.cpp)
	{
		foreach (file; buildfile.allSourcesPlusWrappers.cppSources)
		{
			string objFile = "build/" ~ file ~ ".o";
			auto command = ["clang++", file, "-c", "-o", objFile];
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
		foreach (file; buildfile.allSourcesPlusWrappers.rustSources)
		{
			string objFile = "build/" ~ file ~ ".o";
			auto command = ["rustc", file, "--emit", "obj", "-L", "build/pgwrappers", "-o", objFile];
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
		foreach(fileEntry; chain(buildfile.allSourcesPlusWrappers.dSources))
			dFiles ~= fileEntry;

		auto command = ["ldc2"] ~ dFiles.sort.uniq.array ~ ["-c", "-of", "build/d_monolithic_obj_file.o"];
		writeln("Executing " ~ command.join(' '));
		retval = spawnProcess(command).wait();
		// TODO: more assembly generation
		// spawnProcess(["ldc2"] ~ dFiles.sort.uniq.array ~ ["--output-s"]);

		if (retval != 0)
			return retval;

		objFiles ~= "build/d_monolithic_obj_file.o";
	}

	writeln("Linking " ~ objFiles.sort.uniq.array.to!string);
	retval = spawnProcess(["clang++"] ~ objFiles.sort.uniq.array ~ 
				["-lphobos2-ldc-shared",
				"-ldruntime-ldc-shared",
				getRustStandardLibraryPath(),
				"-o", 
				"build/" ~ buildfile.projectName]
			 ).wait();

	return retval;
}