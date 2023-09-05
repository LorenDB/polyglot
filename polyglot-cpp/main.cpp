// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

#include <iostream>

#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>

#include "CppParser.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;

DeclarationMatcher functionMatcher = functionDecl().bind("function");
DeclarationMatcher enumMatcher = enumDecl().bind("enum");
// we need unless(isImplicit()) to prevent double matching of classes; see
// https://stackoverflow.com/questions/55088770/why-clang-ast-shows-two-cxxrecorddecl-for-a-single-class
DeclarationMatcher classMatcher = cxxRecordDecl(unless(isImplicit())).bind("class");

static llvm::cl::OptionCategory polyglotOptions("polyglot options");
static llvm::cl::extrahelp commonHelp(CommonOptionsParser::HelpMessage);
// static llvm::cl::extrahelp extendedHelp("\nMore help text...\n");

int main(int argc, const char **argv)
{
    auto expectedParser = CommonOptionsParser::create(argc, argv, polyglotOptions);
    if (!expectedParser)
    {
        // Fail gracefully for unsupported options.
        llvm::errs() << expectedParser.takeError();
        return 1;
    }
    CommonOptionsParser &optionsParser = expectedParser.get();
    ClangTool tool(optionsParser.getCompilations(), optionsParser.getSourcePathList());

    PolyglotVisitor visitor;
    MatchFinder finder;
    finder.addMatcher(functionMatcher, &visitor);
    finder.addMatcher(enumMatcher, &visitor);
    finder.addMatcher(classMatcher, &visitor);

    auto retval = tool.run(newFrontendActionFactory(&finder).get());
    if (retval == 0)
        visitor.save();
    else
        std::cerr << "Source wrapping failed" << std::endl;
    return retval;
}
