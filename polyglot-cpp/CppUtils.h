// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <clang/ASTMatchers/ASTMatchers.h>

#include "PolyglotAST.h"

namespace CppUtils
{
    bool isStdString(const clang::QualType &type);
    bool isFixedWidthIntegerType(const clang::QualType &type);

    //! Returns a list of namespace names, starting with the outermost namespace.
    std::vector<std::string> getNamespaceList(const clang::Decl *decl);
} // namespace CppUtils
