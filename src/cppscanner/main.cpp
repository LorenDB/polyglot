// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

#include <iostream>

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Path.h>

#include "CppParser.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;

DeclarationMatcher functionMatcher = functionDecl().bind("basicFunction");
DeclarationMatcher enumMatcher = enumDecl().bind("enum");

class BasicFunctionParser : public MatchFinder::MatchCallback
{
public:
    virtual void run(const MatchFinder::MatchResult &result)
    {
        if (const FunctionDecl *function = result.Nodes.getNodeAs<FunctionDecl>("basicFunction"))
        {
            if (function->isTemplated())
                return;

//            const auto filename = llvm::sys::path::filename(result.SourceManager->getFilename(function->getLocation())).str();
            const auto filename = result.SourceManager->getFilename(function->getLocation()).str();

            if (result.SourceManager->isInSystemHeader(function->getLocation()) || filename.empty())
                return;

            std::cout << "Wrapping function " << function->getNameAsString() << " from " << filename << std::endl;
            try
            {
                m_generator.addFunction(function, filename);
            }
            catch (const std::runtime_error &e)
            {
                std::cerr << "Unable to wrap " << filename << ": " << e.what() << std::endl;
            }
        }
        else if (const EnumDecl *e = result.Nodes.getNodeAs<EnumDecl>("enum"))
        {
            if (e->isTemplated())
                return;

//            const auto filename = llvm::sys::path::filename(result.SourceManager->getFilename(function->getLocation())).str();
            const auto filename = result.SourceManager->getFilename(e->getLocation()).str();

            if (result.SourceManager->isInSystemHeader(e->getLocation()) || filename.empty())
                return;

            std::cout << "Wrapping enum " << e->getNameAsString() << " from " << filename << std::endl;
            try
            {
                m_generator.addEnum(e, filename);
            }
            catch (const std::runtime_error &e)
            {
                std::cerr << "Unable to wrap " << filename << ": " << e.what() << std::endl;
            }
        }
    }

    void dump()
    {
        m_generator.dumpToFile();
    }

private:
    CppParser m_generator;
};

static llvm::cl::OptionCategory polyglotOptions("polyglot options");
static llvm::cl::extrahelp commonHelp(CommonOptionsParser::HelpMessage);
// static cl::extrahelp extendedHelp("\nMore help text...\n");

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

    BasicFunctionParser p;
    MatchFinder f;
    f.addMatcher(functionMatcher, &p);
    f.addMatcher(enumMatcher, &p);

    auto retval = tool.run(newFrontendActionFactory(&f).get());
    p.dump();
    return retval;
}
