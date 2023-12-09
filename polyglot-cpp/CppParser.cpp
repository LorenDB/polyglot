// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

#include "CppParser.h"

#include <format>
#include <fstream>
#include <iostream>

#include <clang/AST/Mangle.h>

#include "CppUtils.h"
#include "DWrapperWriter.h"
#include "RustWrapperWriter.h"
#include "ZigWrapperWriter.h"
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

CppParser::CppParser(std::vector<polyglot::Language> languages, std::string outputDir)
    : m_langs{languages},
      m_outputDir{outputDir}
{}

void CppParser::addFunction(const clang::FunctionDecl *function, const std::string &filename)
{
    auto moduleName = Utils::getModuleName(filename);
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
    functionNode->isNoreturn = function->isNoReturn();
    for (const auto &param : function->parameters())
    {
        polyglot::VariableNode p;
        p.name = param->getNameAsString();
        p.type = typeFromClangType(param->getType(), param);
        if (param->getDefaultArg())
            p.value = getExprValue(param->getDefaultArg(), function->getASTContext());
        functionNode->parameters.push_back(p);
    }

    pushNodeToProperNS(ast, function, functionNode);
}

void CppParser::addEnum(const clang::EnumDecl *e, const std::string &filename)
{
    auto moduleName = Utils::getModuleName(filename);
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

    pushNodeToProperNS(ast, e, enumNode);
}

void CppParser::addClass(const clang::CXXRecordDecl *classDecl, const std::string &filename)
{
    auto moduleName = Utils::getModuleName(filename);
    auto &ast = m_asts[moduleName];
    ast.moduleName = moduleName;
    ast.language = polyglot::Language::Cpp;

    auto classNode = new polyglot::ClassNode;
    classNode->name = classDecl->getNameAsString();
    if (classDecl->isClass())
        classNode->type = polyglot::ClassNode::Type::Class;
    else
        classNode->type = polyglot::ClassNode::Type::Struct;

    std::unique_ptr<clang::MangleContext> mangler;
    mangler.reset(classDecl->getASTContext().createMangleContext());

    for (const auto &method : classDecl->methods())
    {
        if (method->isDeleted())
            continue;

        polyglot::FunctionNode functionNode;
        functionNode.functionName = method->getNameAsString();
        functionNode.isVirtual = method->isVirtual();
        functionNode.isStatic = method->isStatic();

        for (const auto &param : method->parameters())
        {
            polyglot::VariableNode p;
            p.name = param->getNameAsString();
            p.type = typeFromClangType(param->getType(), param);
            if (param->getDefaultArg())
                p.value = getExprValue(param->getDefaultArg(), method->getASTContext());
            functionNode.parameters.push_back(p);
        }

        if (const auto ctor = llvm::dyn_cast<clang::CXXConstructorDecl>(method); ctor)
        {
            llvm::raw_string_ostream buf{functionNode.mangledName};
            mangler->mangleName(clang::GlobalDecl{ctor, clang::CXXCtorType::Ctor_Base}, buf);
            buf.flush();

            classNode->constructors.push_back(functionNode);
        }
        else if (const auto dtor = llvm::dyn_cast<clang::CXXDestructorDecl>(method); dtor)
        {
            llvm::raw_string_ostream buf{functionNode.mangledName};
            mangler->mangleName(clang::GlobalDecl{dtor, clang::CXXDtorType::Dtor_Base}, buf);
            buf.flush();

            classNode->destructor = functionNode;
        }
        else
        {
            functionNode.returnType = typeFromClangType(method->getReturnType(), method);
            functionNode.isNoreturn = method->isNoReturn();

            llvm::raw_string_ostream buf{functionNode.mangledName};
            mangler->mangleName(method, buf);
            buf.flush();

            classNode->methods.push_back(functionNode);
        }
    }

    for (const auto &member : classDecl->fields())
    {
        polyglot::VariableNode m;
        m.name = member->getNameAsString();
        m.type = typeFromClangType(member->getType(), member);
        if (member->getInClassInitializer())
            m.value = getExprValue(member->getInClassInitializer(), classDecl->getASTContext());
        classNode->members.push_back(m);
    }

    pushNodeToProperNS(ast, classDecl, classNode);
}

void CppParser::writeWrappers()
{
    for (const auto &[moduleName, ast] : m_asts)
    {
        if (std::find(m_langs.begin(), m_langs.end(), polyglot::Language::D) != m_langs.end())
        {
            std::ofstream dFile{m_outputDir + moduleName + ".d"};
            DWrapperWriter dWrapper;
            dWrapper.write(ast, dFile);
        }
        if (std::find(m_langs.begin(), m_langs.end(), polyglot::Language::Rust) != m_langs.end())
        {
            std::ofstream rustFile{m_outputDir + moduleName + ".rs"};
            RustWrapperWriter rustWrapper;
            rustWrapper.write(ast, rustFile);
        }
        if (std::find(m_langs.begin(), m_langs.end(), polyglot::Language::Zig) != m_langs.end())
        {
            std::ofstream zigFile{m_outputDir + moduleName + ".zig"};
            ZigWrapperWriter zigWrapper;
            zigWrapper.write(ast, zigFile);
        }
    }
}

polyglot::QualifiedType CppParser::typeFromClangType(const clang::QualType &type, const clang::Decl *decl) const
{
    clang::QualType underlyingType = type;

    polyglot::QualifiedType ret;
    ret.isConst = type.isConstQualified();
    ret.isPointer = type->isPointerType();
    ret.isArray = type->isArrayType();
    ret.isReference = type->isReferenceType();
    ret.isVolatile = type.isVolatileQualified();

    if (ret.isPointer)
        underlyingType = type->getPointeeType();

    using polyglot::Type;
    if (underlyingType->isVoidType() || type->isVoidPointerType())
        ret.baseType = Type::Void;
    else if (underlyingType->isBooleanType())
        ret.baseType = Type::Bool;
    else if (underlyingType->isCharType())
        ret.baseType = Type::Char;
    else if (underlyingType->isChar16Type() || underlyingType->isWideCharType())
        ret.baseType = Type::Char16;
    else if (underlyingType->isChar32Type())
        ret.baseType = Type::Char32;
    else if (underlyingType->isIntegerType())
    {
        if (!CppUtils::isFixedWidthIntegerType(underlyingType))
        {
            auto &diagnostics = decl->getASTContext().getDiagnostics();
            auto id = diagnostics.getDiagnosticIDs()->getCustomDiagID(clang::DiagnosticIDs::Warning,
                                                                      std::format("Use fixed-width integer types for "
                                                                                  "portablility"));
            diagnostics.Report(decl->getBeginLoc(), id);
        }

        auto uint = underlyingType->isUnsignedIntegerType();
        auto size = decl->getASTContext().getTypeSize(underlyingType);
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
    else if (underlyingType->isFloatingType())
    {
        auto size = decl->getASTContext().getTypeSize(underlyingType);
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
    else if (auto enumType = underlyingType->getAs<clang::EnumType>(); enumType)
    {
        ret.baseType = Type::Enum;
        ret.nameString = enumType->getDecl()->getNameAsString();
    }
    else if (auto classType = (underlyingType->getAsCXXRecordDecl()))
    {
        ret.baseType = Type::Class;
        ret.nameString = classType->getName();
    }
    else if (auto rValue = underlyingType->getAs<clang::RValueReferenceType>())
    {
        ret.isRvalueReference = true;
        auto qt = rValue->getPointeeType();
        ret.nameString = qt.getAsString();
    }
    else if (CppUtils::isStdString(type))
        ret.baseType = Type::CppStdString;
    else
        throw std::runtime_error(std::format("Unrecognized type: {}", ret.nameString));

    return ret;
}

void CppParser::pushNodeToProperNS(polyglot::AST &ast, const clang::Decl *decl, polyglot::ASTNode *node) const
{
    auto nsList = CppUtils::getNamespaceList(decl);
    polyglot::AST *astPtr = &ast;
    int i = 0;
    for (; i < nsList.size(); ++i)
    {
        if (astPtr && !astPtr->nodes.empty() && astPtr->nodes.back()->nodeType() == polyglot::ASTNodeType::Namespace)
        {
            if (auto ns = dynamic_cast<polyglot::NamespaceNode *>(astPtr->nodes.back()); ns && nsList[i] == ns->name)
                astPtr = &ns->ast;
            else
                break;
        }
        else
            break;
    }
    for (; i < nsList.size(); ++i)
    {
        auto ns = new polyglot::NamespaceNode;
        ns->name = nsList[i];
        astPtr->nodes.push_back(ns);
        astPtr = &ns->ast;
    }

    astPtr->nodes.push_back(node);
}
