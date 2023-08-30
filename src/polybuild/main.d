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
import std.logger;

import rusthelper;

struct Languages
{
	bool cpp;
	bool d;
	bool rust;
}

int main(string[] args)
{
	if (args[1 .. $].length == 0)
	{
		// TODO: in this case try to use a homebrew buildsystem?
		writeln("Please specify one or more files");
		return -1;
	}

	int retval;
	Languages languages;

	writeln("Calling polyglot-cpp on " ~ args[1 .. $].to!string);

	// wrap each file
	foreach (file; args[1 .. $])
	{
		if (file.endsWith(".cpp") || file.endsWith(".cxx") || file.endsWith(".c++") || file.endsWith(".cc") || file.endsWith(".C"))
		{
			languages.cpp = true;
			retval = spawnProcess(["polyglot-cpp", file, "--"]).wait();
		}

		// TODO: these don't exist yet :(
		else if (file.endsWith(".d"))
		{
			languages.d = true;
			// retval = spawnProcess(["polyglot-d", file]).wait();
		}
		else if (file.endsWith(".rs"))
		{
			languages.rust = true;
			// retval = spawnProcess(["polyglot-rs", file]).wait();
		}

		// TODO: temporary hack because polyglot-cpp throws an error any time you parse a file that includes stddef.h
		// if (retval != 0)
			// return retval;
	}

	writeln("Compiling " ~ args[1 .. $].to!string);
	
	// compile each file
	foreach (file; args[1 .. $])
	{
		string objFile = file ~ ".o";

		if (file.endsWith(".cpp") || file.endsWith(".cxx") || file.endsWith(".c++") || file.endsWith(".cc") || file.endsWith(".C"))
		{
			writeln("Executing " ~ ["clang++", file, "-c", "-o", objFile].to!string);
			retval = spawnProcess(["clang++", file, "-c", "-o", objFile]).wait();
			if (retval != 0)
				return retval;
		}
		else if (file.endsWith(".rs"))
		{
			writeln("Executing " ~ ["rustc", file, "--emit", "obj", "-o", objFile].to!string);
			retval = spawnProcess(["rustc", file, "--emit", "obj", "-o", objFile]).wait();
			if (retval != 0)
				return retval;
		}
	}

	// D wants to compile all the D files in one go, so we'll just do that now.
	string[] dFiles;
	foreach(fileEntry; dirEntries("", "*.d", SpanMode.depth))
		dFiles ~= fileEntry;

	writeln("Executing " ~ (["ldc2"] ~ dFiles.sort.uniq.array ~ ["-c", "--of", "objFile.o"]).to!string);
	retval = spawnProcess(["ldc2"] ~ dFiles.sort.uniq.array ~ ["-c", "--of", "objFile.o"]).wait();

	if (retval != 0)
		return retval;

	// link the whole shebang together
	string[] objFiles;
	foreach (fileEntry; dirEntries("", "*.o", SpanMode.depth))
		objFiles ~= fileEntry;

	writeln("Linking " ~ objFiles.sort.uniq.array.to!string);
	retval = spawnProcess(["clang++"] ~ objFiles.sort.uniq.array ~ 
				["-lphobos2-ldc-shared",
				"-ldruntime-ldc-shared",
				getRustStandardLibraryPath(),
				"-o", 
				"./main"]
			 ).wait();

	return retval;
}