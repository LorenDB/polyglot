// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

#pragma once

#include "clang/ASTMatchers/ASTMatchers.h"

#include "../core/PolyglotAST.h"

enum class BindingType
{
    Function,
    Enum,
    Struct,

    Undefined,
};

struct BindingFile
{
    polyglot::Language language;
    std::string moduleName;
    std::vector<std::pair<BindingType, std::string>> declarations;
};

class CppParser
{
public:
    CppParser();

    void addFunction(const clang::FunctionDecl *function, const std::string &filename);
    void addEnum(const clang::EnumDecl *e, const std::string &filename);

    void dumpToFile();

private:
    polyglot::QualifiedType typeFromClangType(const clang::QualType &qualType, const clang::Decl *decl) const;

    std::map<std::string, polyglot::AST> m_asts;
};
