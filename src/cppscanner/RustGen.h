// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

#pragma once

#include "clang/ASTMatchers/ASTMatchers.h"

namespace RustGen
{
    // basic binding file strings
    std::string getRustFileHeader(const std::string &moduleName);
    std::string getRustFileFooter();
    std::string getBeginFunctionBlock();
    std::string getEndFunctionBlock();

    // strings for constructing various bits and pieces of declarations
    std::string getRustTypeString(const clang::QualType &type, const clang::Decl *decl);
    std::string getRustExprValueString(const clang::Expr *defaultValue, const clang::ASTContext &context);
} // namespace RustGen
