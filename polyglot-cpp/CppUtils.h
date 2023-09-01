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
    std::shared_ptr<polyglot::Namespace> buildNamespaceTree(const clang::Decl *decl);

} // namespace CppUtils
