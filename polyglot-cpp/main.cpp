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
static llvm::cl::list<polyglot::Language> languages{"lang",
                                                    llvm::cl::desc{"A list of languages to output wrappers for. If "
                                                                   "unset, all supported languages will be "
                                                                   "generated."},
                                                    llvm::cl::values(clEnumValN(polyglot::Language::Cpp, "cpp", "C++"),
                                                                     clEnumValN(polyglot::Language::D, "d", "D"),
                                                                     clEnumValN(polyglot::Language::Rust, "rust", "Rust")),
                                                    llvm::cl::ZeroOrMore,
                                                    llvm::cl::cat(polyglotOptions)};
static llvm::cl::opt<std::string> outputDir{"output-dir",
                                            llvm::cl::desc{"The directory to output wrappers into. By default, this is set "
                                                           "to the current directory."}};

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

    std::vector<polyglot::Language> langs;
    for (const auto &l : languages)
        langs.push_back(l);
    if (langs.size() == 0)
        langs = {polyglot::Language::D, polyglot::Language::Rust};

    std::string outdir = outputDir.getValue();
    if (!outdir.ends_with('/'))
        outdir += '/';
    PolyglotVisitor visitor{langs, outdir};
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
