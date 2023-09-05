// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

#pragma once

#include "clang/ASTMatchers/ASTMatchers.h"
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>

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
    void addClass(const clang::CXXRecordDecl *classDecl, const std::string &filename);

    void writeWrappers();

private:
    polyglot::QualifiedType typeFromClangType(const clang::QualType &qualType, const clang::Decl *decl) const;

    std::map<std::string, polyglot::AST> m_asts;
};

class PolyglotVisitor : public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    virtual void run(const clang::ast_matchers::MatchFinder::MatchResult &result)
    {
        if (const clang::FunctionDecl *function = result.Nodes.getNodeAs<clang::FunctionDecl>("function"))
        {
            if (function->isTemplated() || function->isCXXClassMember() ||
                function->getNameAsString() == "polyglot_make_sure_symbols_are_kept_by_the_linker")
                return;

            const auto filename = result.SourceManager->getFilename(function->getLocation()).str();
            if (result.SourceManager->isInSystemHeader(function->getLocation()) || filename.empty())
                return;

            try
            {
                m_generator.addFunction(function, filename);
            }
            catch (const std::runtime_error &e)
            {
                auto &diagnostics = function->getASTContext().getDiagnostics();
                auto id = diagnostics.getDiagnosticIDs()->getCustomDiagID(
                    clang::DiagnosticIDs::Error,
                    std::format("Could not wrap function `{}`: {}", function->getNameAsString(), e.what()));
                diagnostics.Report(function->getBeginLoc(), id);
            }
        }
        else if (const clang::EnumDecl *enumDecl = result.Nodes.getNodeAs<clang::EnumDecl>("enum"))
        {
            if (enumDecl->isTemplated())
                return;

            const auto filename = result.SourceManager->getFilename(enumDecl->getLocation()).str();
            if (result.SourceManager->isInSystemHeader(enumDecl->getLocation()) || filename.empty())
                return;

            try
            {
                m_generator.addEnum(enumDecl, filename);
            }
            catch (const std::runtime_error &e)
            {
                auto &diagnostics = enumDecl->getASTContext().getDiagnostics();
                auto id = diagnostics.getDiagnosticIDs()->getCustomDiagID(
                    clang::DiagnosticIDs::Error,
                    std::format("Could not wrap enum `{}`: {}", enumDecl->getNameAsString(), e.what()));
                diagnostics.Report(enumDecl->getBeginLoc(), id);
            }
        }
        else if (const clang::CXXRecordDecl *classDecl = result.Nodes.getNodeAs<clang::CXXRecordDecl>("class"))
        {
            if (classDecl->isTemplated() || classDecl->isImplicit())
                return;

            const auto filename = result.SourceManager->getFilename(classDecl->getLocation()).str();
            if (result.SourceManager->isInSystemHeader(classDecl->getLocation()) || filename.empty())
                return;

            try
            {
                m_generator.addClass(classDecl, filename);
            }
            catch (const std::runtime_error &e)
            {
                auto &diagnostics = classDecl->getASTContext().getDiagnostics();
                auto id = diagnostics.getDiagnosticIDs()->getCustomDiagID(
                    clang::DiagnosticIDs::Error,
                    std::format("Could not wrap class `{}`: {}", classDecl->getNameAsString(), e.what()));
                diagnostics.Report(classDecl->getBeginLoc(), id);
            }
        }
    }

    void save() { m_generator.writeWrappers(); }

private:
    CppParser m_generator;
};
