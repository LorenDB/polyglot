// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

module polybuild.buildfile;

@safe:

import std.algorithm;
import std.array;

import dyaml;

import polybuild.cpphelper;

struct Languages
{
	bool cpp;
	bool d;
	bool rust;
}

struct Sources
{
    string[] _sources;
    alias _sources this;

    this(string[] s) { _sources = s; }

    string[] cppSources()
    {
        return _sources.filter!(file => file.isCppFile).array;
    }

    string[] dSources()
    {
        return _sources.filter!(file => file.endsWith(".d")).array;
    }

    string[] rustSources()
    {
        return _sources.filter!(file => file.endsWith(".rs")).array;
    }

    Languages languages()
    {
        Languages ret;
        if (cppSources.length > 0)
            ret.cpp = true;
        if (dSources.length > 0)
            ret.d = true;
        if (rustSources.length > 0)
            ret.rust = true;

        return ret;
    }
}

struct Buildfile
{
    string projectName;
    Sources sources;
    Sources nonWrappingSources;
    Sources generatedSources;

    Sources allSources()
    {
        return Sources(sources ~ nonWrappingSources);
    }

    Sources allSourcesPlusWrappers()
    {
        return Sources(sources ~ nonWrappingSources ~ generatedSources);
    }
}

Buildfile buildfileFromYAML(Node yaml)
{
    Buildfile b;
	b.projectName = yaml["name"].as!string;
	foreach (string source; yaml["sources"])
		b.sources ~= source;
	if (yaml.containsKey("exclude-wrapping"))
		foreach (string source; yaml["exclude-wrapping"])
			b.nonWrappingSources ~= source;

    return b;
}
