// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

#pragma once

#include "clang/ASTMatchers/ASTMatchers.h"

namespace polyglot
{
    enum class Language
    {
        Cpp,
        D,
        Rust,
        Swift,
    };

    enum class ASTNodeType
    {
        Function,
        Class,
        Enum,
        GlobalVariable,
    };

    enum class Type
    {
        // the main built-in types
        Void,
        Bool,
        Char,
        Char16,
        Char32,
        Int8,
        Int16,
        Int32,
        Int64,
        Int128,
        Uint8,
        Uint16,
        Uint32,
        Uint64,
        Uint128,
        Float32,
        Float64,
        Float128,

        // User-defined types
        Enum,
        Class,

        // Types that are known to need implicit bindings (at least in some cases)
        CppStdString,
    };

    struct ASTNode
    {
    public:
        virtual ASTNodeType nodeType() const = 0;

    private:
        std::vector<ASTNode *> m_children;
    };

    class AST
    {
    public:

    private:
        Language m_language;
    };
}
