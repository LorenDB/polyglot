// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

#include "Utils.h"

std::string Utils::getModuleName(std::string filename)
{
    // TODO: this should not be the only source of truth for module names; also implement source scanning to check module
    // names
    std::string moduleName;
    if (filename.ends_with(".cpp") || filename.ends_with(".cxx") || filename.ends_with(".c++"))
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