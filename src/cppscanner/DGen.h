// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

#pragma once

#include "clang/ASTMatchers/ASTMatchers.h"

namespace DGen
{
    // basic binding file strings
    std::string getDFileHeader(const std::string &moduleName);
    std::string getDFileFooter();

    // strings for constructing various bits and pieces of declarations
    std::string getDTypeString(const clang::QualType &type, const clang::Decl *decl);
    std::string getDExprValueString(const clang::Expr *defaultValue, const clang::ASTContext &context);
} // namespace DGen
