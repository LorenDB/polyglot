module buildfile;

@safe:

import std.algorithm;
import std.array;

import dyaml;

struct Languages
{
	bool cpp;
	bool d;
	bool rust;
}

struct Sources
{
    string[] _allSources;
    alias _allSources this;

    this(string[] s) { _allSources = s; }

    string[] cppSources()
    {
        return _allSources.filter!(file => file.endsWith(".cpp") || file.endsWith(".cxx") || file.endsWith(".c++") || file.endsWith(".cc") || file.endsWith(".C")).array;
    }

    string[] dSources()
    {
        return _allSources.filter!(file => file.endsWith(".d")).array;
    }

    string[] rustSources()
    {
        return _allSources.filter!(file => file.endsWith(".rs")).array;
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