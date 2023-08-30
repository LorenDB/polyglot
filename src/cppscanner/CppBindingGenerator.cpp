// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

#include "CppBindingGenerator.h"

#include <iostream>
#include <fstream>
#include <format>

#include <clang/AST/Mangle.h>

#include "DGen.h"
#include "RustGen.h"

CppBindingGenerator::CppBindingGenerator() {}

void CppBindingGenerator::addFunction(const clang::FunctionDecl *function, const std::string &filename)
{
    // baseFilename can have an extension like ".d" or ".rs" appended to it to produce a valid filename
    std::string baseFilename;
    if (filename.ends_with(".cpp") || filename.ends_with(".cxx") || filename.ends_with(".c++"))
        baseFilename = filename.substr(0, filename.size() - 4);
    else if (filename.ends_with(".cc"))
        baseFilename = filename.substr(0, filename.size() - 3);
    else if (filename.ends_with(".C"))
        baseFilename = filename.substr(0, filename.size() - 2);
    else
        baseFilename = filename;
    std::string moduleName = baseFilename;
    if (moduleName.find('/') != std::string::npos)
        moduleName = moduleName.substr(moduleName.find_last_of('/') + 1);
    if (moduleName.find('\\') != std::string::npos)
        moduleName = moduleName.substr(moduleName.find_last_of('\\') + 1);

    baseFilename = moduleName;

    auto returnType = function->getReturnType();
    auto mangler = function->getASTContext().createMangleContext();
    // TODO: add support for custom types
//    if (!returnType->isBuiltinType())
//        return;

    // generate the D binding
    std::string dBindingSource{"extern(C++) "};
    dBindingSource += DGen::getDTypeString(returnType, function) + function->getNameAsString() + '(';
    std::string params;
    for (const auto &param : function->parameters())
    {
        params += DGen::getDTypeString(param->getType(), function) + param->getNameAsString();
        if (param->hasDefaultArg())
            params += " = " + DGen::getDExprValueString(param->getDefaultArg(), function->getASTContext());
        params += ", ";
    }
    dBindingSource += params.substr(0, params.size() - 2) + ')';
    // TODO: any other qualifiers after the parenthesis?
    dBindingSource += ';';
    m_bindingFiles[baseFilename + ".d"].declarations.push_back({BindingType::Function, dBindingSource});
    m_bindingFiles[baseFilename + ".d"].language = polyglot::Language::D;
    m_bindingFiles[baseFilename + ".d"].moduleName = moduleName;

    // generate the Rust binding
    std::string rustBindingSource;
    rustBindingSource += "#[link_name = \"";
    llvm::raw_string_ostream temp(rustBindingSource);
    mangler->mangleName(function, temp);
    temp.flush();
    rustBindingSource += "\"]\tpub fn " + function->getNameAsString() + '(';
    params.clear();
    // note that Rust doesn't support default arguments
    for (const auto &param : function->parameters())
        params +=
            param->getNameAsString() + ": " + RustGen::getRustTypeString(param->getType(), function) + ", ";
    rustBindingSource += params.substr(0, params.size() - 2) + ')';
    if (!returnType->isVoidType())
        rustBindingSource += " -> " + RustGen::getRustTypeString(returnType, function);
    rustBindingSource += ';';
    m_bindingFiles[baseFilename + ".rs"].declarations.push_back({BindingType::Function, rustBindingSource});
    m_bindingFiles[baseFilename + ".rs"].language = polyglot::Language::Rust;
    m_bindingFiles[baseFilename + ".rs"].moduleName = moduleName;

    delete mangler;
}

void CppBindingGenerator::addEnum(const clang::EnumDecl *e, const std::string &filename)
{
    // baseFilename can have an extension like ".d" or ".rs" appended to it to produce a valid filename
    std::string baseFilename;
    if (filename.ends_with(".cpp") || filename.ends_with(".cxx") || filename.ends_with(".c++"))
        baseFilename = filename.substr(0, filename.size() - 4);
    else if (filename.ends_with(".cc"))
        baseFilename = filename.substr(0, filename.size() - 3);
    else if (filename.ends_with(".C"))
        baseFilename = filename.substr(0, filename.size() - 2);
    else
        baseFilename = filename;
    std::string moduleName = baseFilename;
    if (moduleName.find('/') != std::string::npos)
        moduleName = moduleName.substr(moduleName.find_last_of('/') + 1);
    if (moduleName.find('\\') != std::string::npos)
        moduleName = moduleName.substr(moduleName.find_last_of('\\') + 1);

    baseFilename = moduleName;

    // generate the D binding
    std::string dBindingSource{"enum " + e->getNameAsString() + "\n{\n"};
    for (const auto &enumerator : e->enumerators())
    {
        dBindingSource += '\t' + enumerator->getNameAsString();
        if (enumerator->getInitExpr())
            dBindingSource += " = " + DGen::getDExprValueString(enumerator->getInitExpr(), e->getASTContext());
        dBindingSource += ",\n";
    }
    dBindingSource += "}";
    m_bindingFiles[baseFilename + ".d"].declarations.push_back({BindingType::Enum, dBindingSource});
    m_bindingFiles[baseFilename + ".d"].language = polyglot::Language::D;
    m_bindingFiles[baseFilename + ".d"].moduleName = moduleName;

    // generate the Rust binding
    std::string rustBindingSource{"#[repr(C)]\npub enum " + e->getNameAsString() + "\n{\n"};
    for (const auto &enumerator : e->enumerators())
    {
        rustBindingSource += '\t' + enumerator->getNameAsString();
        if (enumerator->getInitExpr())
            rustBindingSource += " = " + DGen::getDExprValueString(enumerator->getInitExpr(), e->getASTContext());
        rustBindingSource += ",\n";
    }
    rustBindingSource += "}";
    m_bindingFiles[baseFilename + ".rs"].declarations.push_back({BindingType::Enum, rustBindingSource});
    m_bindingFiles[baseFilename + ".rs"].language = polyglot::Language::Rust;
    m_bindingFiles[baseFilename + ".rs"].moduleName = moduleName;
}

void CppBindingGenerator::dumpToFile()
{
    for (const auto &[filename, fileSource] : m_bindingFiles)
    {
        std::cerr << "Writing file at " << filename << std::endl;
        std::ofstream file{filename};
        switch (fileSource.language)
        {
        case polyglot::Language::D:
            file << DGen::getDFileHeader(fileSource.moduleName) << std::endl;
            break;
        case polyglot::Language::Cpp:
            break;
        case polyglot::Language::Rust:
            file << RustGen::getRustFileHeader(fileSource.moduleName) << std::endl;
            break;
        }

        BindingType previousType = BindingType::Undefined;
        for (const auto &[type, source] : fileSource.declarations)
        {
            if (previousType != type)
            {
                if (type == BindingType::Function)
                {
                    switch (fileSource.language)
                    {
                    case polyglot::Language::D:
                    case polyglot::Language::Cpp:
                        file << std::endl;
                        break;
                    case polyglot::Language::Rust:
                        file << std::endl;
                        file << RustGen::getBeginFunctionBlock() << std::endl;
                        break;
                    }
                }
                else if (previousType == BindingType::Function)
                {
                    switch (fileSource.language)
                    {
                    case polyglot::Language::D:
                    case polyglot::Language::Cpp:
                        file << std::endl;
                        break;
                    case polyglot::Language::Rust:
                        file << RustGen::getEndFunctionBlock() << std::endl;
                        file << std::endl;
                        break;
                    }
                }
            }
            previousType = type;
            file << source << std::endl;
        }

        switch (fileSource.language)
        {
        case polyglot::Language::D:
            file << DGen::getDFileFooter() << std::endl;
            break;
        case polyglot::Language::Cpp:
            break;
        case polyglot::Language::Rust:
            if (previousType == BindingType::Function)
                file << RustGen::getEndFunctionBlock();
            file << RustGen::getRustFileFooter() << std::endl;
            break;
        }
        file.close();
    }
}
