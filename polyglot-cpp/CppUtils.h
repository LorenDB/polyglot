// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

#pragma once

#include "clang/ASTMatchers/ASTMatchers.h"

namespace CppUtils
{
    bool isStdString(const clang::QualType &type);
    bool isFixedWidthIntegerType(const clang::QualType &type);
} // namespace CppUtils
