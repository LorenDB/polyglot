// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

#pragma once

#include "clang/ASTMatchers/ASTMatchers.h"

#include "../core/PolyglotAST.h"

namespace
{
    constexpr auto POLYGLOT_VERSION = "0.0.1-devel";
}

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

class CppBindingGenerator
{
public:
    CppBindingGenerator();

    void addFunction(const clang::FunctionDecl *function, const std::string &filename);
    void addEnum(const clang::EnumDecl *e, const std::string &filename);

    void dumpToFile();

private:
//    std::vector<std::string> m_cppToDBindingDecls;
//    std::vector<std::string> m_dToRustBindingDefs;

//    std::string m_dBackendTranslationSource;
//    std::string m_dBindingSource;
//    std::string m_cppBindingSource;
//    std::string m_rustBindingSource;

    std::map<std::string, BindingFile> m_bindingFiles;
};
