// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

#pragma once

#include "clang/ASTMatchers/ASTMatchers.h"

namespace Utils
{
    constexpr auto POLYGLOT_VERSION = "0.0.1-devel";

    bool isStdString(const clang::QualType &type);
    bool isFixedWidthIntegerType(const clang::QualType &type);
} // namespace Utils
