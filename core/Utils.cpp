// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

#include "Utils.h"

std::string Utils::getModuleName(std::string filename)
{
    // TODO: this should not be the only source of truth for module names; also implement source scanning to check module
    // names
    std::string moduleName;
    if (filename.ends_with(".cpp") || filename.ends_with(".cxx") || filename.ends_with(".c++") || filename.ends_with(".zig"))
        moduleName = filename.substr(0, filename.size() - 4);
    else if (filename.ends_with(".cc") || filename.ends_with(".rs"))
        moduleName = filename.substr(0, filename.size() - 3);
    else if (filename.ends_with(".C") || filename.ends_with(".d"))
        moduleName = filename.substr(0, filename.size() - 2);
    else
        moduleName = filename;

    if (moduleName.find('/') != std::string::npos)
        moduleName = moduleName.substr(moduleName.find_last_of('/') + 1);
    if (moduleName.find('\\') != std::string::npos)
        moduleName = moduleName.substr(moduleName.find_last_of('\\') + 1);

    return moduleName;
}

std::string Utils::getLanguageName(const polyglot::AST &ast)
{
    switch (ast.language)
    {
    case polyglot::Language::Cpp:
        return "C++";
    case polyglot::Language::D:
        return "D";
    case polyglot::Language::Rust:
        return "Rust";
    case polyglot::Language::Swift:
        return "Swift";
    case polyglot::Language::Zig:
        return "Zig";
    default:
        return "<unrecognized language>";
    }
}
