// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <string>

#include "PolyglotAST.h"

namespace Utils
{
    constexpr auto POLYGLOT_VERSION = "0.0.1-devel";

    std::string getModuleName(std::string filename);
    std::string getLanguageName(const polyglot::AST &ast);
} // namespace Utils
