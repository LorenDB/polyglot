// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

#include "CppParser.h"

#include <iostream>
#include <fstream>
#include <format>

#include <clang/AST/Mangle.h>

#include "DGen.h"
#include "RustGen.h"
#include "../core/DWrapperWriter.h"
#include "../core/RustWrapperWriter.h"
#include "Utils.h"

polyglot::Value getExprValue(const clang::Expr *defaultValue, const clang::ASTContext &context)
{
    clang::Expr::EvalResult result;
    if (!defaultValue->EvaluateAsConstantExpr(result, context))
        throw std::runtime_error("Failed to evaluate expression");

    using namespace polyglot;
    Value ret;
    if (result.Val.isInt())
    {
        ret.type = Type::Int64;
        ret.value = result.Val.getInt().getExtValue();
    }
    else if (result.Val.isFloat())
    {
        ret.type = Type::Float64;
        ret.value = result.Val.getFloat().convertToDouble();
    }
    else if (result.Val.isNullPointer()) // TODO: integrate this case with the wrappers
    {
        ret.type = Type::Void;
        ret.value = nullptr;
    }

    return ret;
}

CppParser::CppParser() {}

void CppParser::addFunction(const clang::FunctionDecl *function, const std::string &filename)
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
    std::string mangledName;
    llvm::raw_string_ostream buf(mangledName);
    mangler->mangleName(function, buf);
    buf.flush();
    delete mangler;

    auto &ast = m_asts[moduleName];
    ast.moduleName = moduleName;
    ast.language = polyglot::Language::Cpp;

    auto functionNode = new polyglot::FunctionNode;
    functionNode->functionName = function->getNameAsString();
    functionNode->mangledName = mangledName;
    functionNode->returnType = typeFromClangType(function->getReturnType(), function);
    for (const auto &param : function->parameters())
    {
        polyglot::FunctionNode::Parameter p;
        p.name = param->getNameAsString();
        p.type = typeFromClangType(param->getType(), param);
        if (param->getDefaultArg())
            p.defaultValue = getExprValue(param->getDefaultArg(), function->getASTContext());
        functionNode->parameters.push_back(p);
    }

    ast.nodes.push_back(functionNode);
}

void CppParser::addEnum(const clang::EnumDecl *e, const std::string &filename)
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

    auto &ast = m_asts[moduleName];
    ast.moduleName = moduleName;
    ast.language = polyglot::Language::Cpp;

    auto enumNode = new polyglot::EnumNode;
    enumNode->enumName = e->getNameAsString();
    for (const auto &enumerator : e->enumerators())
    {
        // so many enum type names!
        polyglot::EnumNode::Enumerator enumerator2;
        enumerator2.name = enumerator->getNameAsString();
        if (enumerator->getInitExpr())
            enumerator2.value = getExprValue(enumerator->getInitExpr(), e->getASTContext());
        enumNode->enumerators.push_back(enumerator2);
    }

    ast.nodes.push_back(enumNode);
}

void CppParser::dumpToFile()
{
    for (const auto &[moduleName, ast] : m_asts)
    {
        std::ofstream dFile{moduleName + ".d"};
        DWrapperWriter dWrapper;
        dWrapper.write(ast, dFile);
        std::ofstream rustFile{moduleName + ".rs"};
        RustWrapperWriter rustWrapper;
        rustWrapper.write(ast, rustFile);
    }
}

polyglot::QualifiedType CppParser::typeFromClangType(const clang::QualType &type, const clang::Decl *decl) const
{
    polyglot::QualifiedType ret;
    ret.isConst = type.isConstQualified();
    ret.isPointer = type->isPointerType();
    ret.isArray = type->isArrayType();
    ret.isReference = type->isReferenceType();
    ret.isVolatile = type.isVolatileQualified();

    using polyglot::Type;
    if (type->isVoidType() || type->isVoidPointerType())
        ret.baseType = Type::Void;
    else if (type->isBooleanType())
        ret.baseType = Type::Bool;
    else if (type->isCharType())
        ret.baseType = Type::Char;
    else if (type->isChar16Type() || type->isWideCharType())
        ret.baseType = Type::Char16;
    else if (type->isChar32Type())
        ret.baseType = Type::Char32;
    else if (type->isIntegerType())
    {
        if (!Utils::isFixedWidthIntegerType(type))
        {
            auto &diagnostics = decl->getASTContext().getDiagnostics();
            auto id = diagnostics.getDiagnosticIDs()->getCustomDiagID(clang::DiagnosticIDs::Warning,
                                                                      std::format("Use fixed-width integer types for "
                                                                                  "portablility"));
            diagnostics.Report(decl->getBeginLoc(), id);
        }

        auto uint = type->isUnsignedIntegerType();
        auto size = decl->getASTContext().getTypeSize(type);
        switch (size)
        {
        case 8:
            ret.baseType = uint ? Type::Uint8 : Type::Int8;
            break;
        case 16:
            ret.baseType = uint ? Type::Uint16 : Type::Int16;
            break;
        case 32:
            ret.baseType = uint ? Type::Uint32 : Type::Int32;
            break;
        case 64:
            ret.baseType = uint ? Type::Uint64 : Type::Int64;
            break;
        case 128:
            ret.baseType = uint ? Type::Uint128 : Type::Int128;
            break;
        default:
            throw std::runtime_error(std::string("Unrecognized integer size: ") + std::to_string(size));
        }
    }
    else if (type->isFloatingType())
    {
        auto size = decl->getASTContext().getTypeSize(type);
        switch (size)
        {
        case 32:
            ret.baseType = Type::Float32;
            break;
        case 64:
            ret.baseType = Type::Float64;
            break;
        case 128:
            ret.baseType = Type::Float128;
            break;
        default:
            throw std::runtime_error(std::string("Unrecognized floating-point size: ") + std::to_string(size));
        }
    }
    else if (auto enumType = type->getAs<clang::EnumType>(); enumType)
    {
        ret.baseType = Type::Enum;
        ret.nameString = enumType->getDecl()->getNameAsString();
    }
    else if (Utils::isStdString(type))
        ret.baseType = Type::CppStdString;
    else
        throw std::runtime_error("Unrecognized type");

    return ret;
}
