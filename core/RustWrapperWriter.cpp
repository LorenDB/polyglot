// SPDX-FileCopyrightText: Loren Burkholder
// SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
//
// SPDX-License-Identifier: GPL-3.0

#include "RustWrapperWriter.h"

#include <algorithm>
#include <ctime>
#include <format>
#include <string>
#include <iostream>

#include "Utils.h"

using namespace polyglot;

void RustWrapperWriter::write(const AST &ast, std::ostream &out)
{
    static int s_onlyWriteHeaderOnce = 0;
    if (s_onlyWriteHeaderOnce == 0)
    {
        auto t = std::time(nullptr);
        std::string timeStr = std::asctime(std::localtime(&t));
        out << std::format(
R"(// *** WARNING: autogenerated file, do not modify. Changes will be overwritten. ***
// Generated by Polyglot version {} at {}.
// This file contains symbols that have been exported from {} into Rust.

// This import supports various Polyglot features; however, it may not be used in every
// Polyglot wrapper file.
use std::ffi::CString;
)",
            Utils::POLYGLOT_VERSION,
            timeStr.substr(0, timeStr.size() - 1), // remove the '\n'
            Utils::getLanguageName(ast));
    }
    ++s_onlyWriteHeaderOnce;

    auto writeFunctionString = [this, &ast, &out](const polyglot::FunctionNode &function, bool isClassMethod, bool isProxied) {
        out << std::format(R"({}#[link_name = "{}"] )",
                           std::string(m_indentationDepth, '\t'),
                           function.mangledName);
        if (!isProxied)
            out << "pub ";
        out << "fn " << function.functionName << '(';

        std::string params;
        // note that Rust doesn't support default arguments
        for (const auto &param : function.parameters)
        {
            params += param.name + ": " ;
            if (isProxied && std::find(function.typeProxy.proxiedParameters.begin(),
                                                        function.typeProxy.proxiedParameters.end(),
                                                        param.name) != function.typeProxy.proxiedParameters.end())
                params += "string";
            else
                params += getTypeString(param.type);
            params += ", ";
        }
        out << params.substr(0, params.size() - 2) + ')';

        if (function.returnType.baseType != Type::Void)
        {
            auto type = function.returnType;
            if (type.baseType == Type::Char && type.isConst && type.isPointer)
                type.isConst = false;
            out << " -> " << getTypeString(type);
        }
        out << ";\n";
    };
    auto writeProxyFunction = [this, &ast, &out](const polyglot::FunctionNode &function) {
        out << std::string(m_indentationDepth, '\t') << "#[allow(non_snake_case)]\n";
        out << std::string(m_indentationDepth, '\t') << "pub fn " << function.functionName << '(';

        std::string params;
        for (const auto &param : function.parameters)
        {
            params += param.name + ": ";
            if (std::find(function.typeProxy.proxiedParameters.begin(),
                                                        function.typeProxy.proxiedParameters.end(),
                                                        param.name) != function.typeProxy.proxiedParameters.end())
                params += "String";
            else
                params += getTypeString(param.type);
            params += ", ";
        }
        out << params.substr(0, params.size() - 2) + ')';
        // TODO: am I missing any other qualifiers?

        if (function.returnType.baseType != Type::Void)
        {
            out << " -> ";
            if (function.typeProxy.isReturnProxied)
                // For now we're assuming that the only thing being proxied is string types
                out << "String";
            else
                out << getTypeString(function.returnType);
        }

        out << " {\n" << std::string(++m_indentationDepth, '\t') << "unsafe {\n" << std::string(++m_indentationDepth, '\t');

        if (function.typeProxy.isReturnProxied)
            out << "CString::from_raw(";
        out << function.typeProxy.proxy->functionName << '(';

        params.clear();
        bool addedNewline = false;
        for (const auto &param : function.parameters)
        {
            const auto convertToRawStr = std::find(function.typeProxy.proxiedParameters.begin(),
                                                    function.typeProxy.proxiedParameters.end(),
                                                    param.name) != function.typeProxy.proxiedParameters.end();
            if (convertToRawStr)
            {
                params += '\n' + std::string(m_indentationDepth + 1, '\t') + "CString::new(";
                addedNewline = true;
            }
            params += param.name;
            if (convertToRawStr)
                params += std::format(R"().expect("Failed to convert parameter {} of {} into CString").into_raw())",
                                      param.name,
                                      function.functionName);
            params += ", ";
        }
        out << params.substr(0, params.size() - 2);

        if (function.typeProxy.isReturnProxied)
        {
            out << ")";
            if (addedNewline)
                out << "\n" << std::string(m_indentationDepth, '\t');
            out << ")\n"
                << std::string(++m_indentationDepth, '\t') << ".into_string()\n"
                << std::string(m_indentationDepth--, '\t')
                << std::format(R"(.expect("Failed to convert C-style string to String in {}")",
                               function.functionName);
        }
        out << ")\n";
        out << std::string(--m_indentationDepth, '\t') << "}\n" << std::string(--m_indentationDepth, '\t') << "}\n";
    };

    auto previousNodeType = ASTNodeType::Undefined;
    for (const auto &node : ast.nodes)
    {
        if (node->nodeType() == ASTNodeType::Function && previousNodeType != ASTNodeType::Function)
        {
            out << '\n' << std::string(m_indentationDepth, '\t') << "extern {\n";
            ++m_indentationDepth;
        }
        else if (node->nodeType() != ASTNodeType::Function && previousNodeType == ASTNodeType::Function)
        {
            --m_indentationDepth;
            out << std::string(m_indentationDepth, '\t') << "}\n\n";
        }

        if (node->nodeType() == ASTNodeType::Namespace)
        {
            auto ns = dynamic_cast<NamespaceNode *>(node);
            if (ns == nullptr)
                throw std::runtime_error("Node claimed to be NamespaceNode, but cast failed");

            // We'll make sure that namespaces from other languages don't make Rust yell, because it would be rude to ignore
            // the standards of other languages just for Rust's sake. ;)
            out << std::string(m_indentationDepth, '\t') << "#[allow(non_snake_case)]\n"
                << std::string(m_indentationDepth, '\t') << "pub mod " << ns->name << " {\n";
            ++m_indentationDepth;
            write(ns->ast, out);
            --m_indentationDepth;
            out << std::string(m_indentationDepth, '\t') << "}\n";
        }
        else if (node->nodeType() == ASTNodeType::Function)
        {
            auto function = dynamic_cast<FunctionNode *>(node);
            if (function == nullptr)
                throw std::runtime_error("Node claimed to be FunctionNode, but cast failed");

            if (function->typeProxy.isValid)
            {
                writeFunctionString(*function->typeProxy.proxy, false, true);
                out << std::string(--m_indentationDepth, '\t') << "}\n\n";
                writeProxyFunction(*function);
                out << std::string(m_indentationDepth++, '\t') << "\nextern {\n";
            }
            else
                writeFunctionString(*function, false, false);
        }
        else
        {
            if (node->nodeType() == ASTNodeType::Enum)
            {
                auto e = dynamic_cast<EnumNode *>(node);
                if (e == nullptr)
                    throw std::runtime_error("Node claimed to be EnumNode, but cast failed");

                out << std::string(m_indentationDepth, '\t') << "#[repr(C)]\n"
                    << std::string(m_indentationDepth, '\t') << "pub enum " << e->enumName << " {\n";
                ++m_indentationDepth;
                for (const auto &enumerator : e->enumerators)
                {
                    out << std::string(m_indentationDepth, '\t') << enumerator.name;
                    if (enumerator.value.has_value())
                        out << " = " + getValueString(enumerator.value.value());
                    out << ",\n";
                }
                --m_indentationDepth;
                out << std::string(m_indentationDepth, '\t') << "}\n";
            }
            else if (node->nodeType() == ASTNodeType::Class)
            {
                auto classNode = dynamic_cast<ClassNode *>(node);
                if (classNode == nullptr)
                    throw std::runtime_error("Node claimed to be ClassNode, but cast failed");

                out << std::string(m_indentationDepth, '\t') << "#[repr(C)]\n"
                    << std::string(m_indentationDepth, '\t') << "pub struct " << classNode->name << " {\n";
                ++m_indentationDepth;
                for (const auto &member : classNode->members)
                {
                    out << std::string(m_indentationDepth, '\t') << "pub "
                        << member.name + ": " + getTypeString(member.type);
                    // TODO: Rust doesn't support default values for struct fields; figure out a workaround
                    // if (member.value.has_value())
                    //     out << " = " << getValueString(member.value.value());
                    out << ",\n";
                }
                --m_indentationDepth;
                out << std::string(m_indentationDepth, '\t') << "}\n";

                // TODO: wrap constructors and destructors here

                if (!classNode->methods.empty())
                {
                    // First we will write an impl block. The impl block will contain function definitions that will be
                    // responsible for calling the actual functions. This probably doesn't support virtual functions yet.
                    // Eventually I intend to see how tools like bindgen or cxx.rs handle virtual functions and copy that
                    // method.
                    out << '\n' << std::string(m_indentationDepth, '\t') << "impl " << classNode->name << " {\n";
                    ++m_indentationDepth;
                    for (const auto &method : classNode->methods)
                    {
                        out << std::string(m_indentationDepth, '\t')
                            << std::format("pub fn {}(&mut self", method.functionName);

                        std::string params;
                        for (const auto &param : method.parameters)
                            params += ", " + param.name + ": " + getTypeString(param.type);
                        out << params << ')';

                        if (method.returnType.baseType != Type::Void)
                            out << " -> " << getTypeString(method.returnType);
                        out << " {\n";

                        // Write the call to the actual member function. Note that we have to wrap it as unsafe to allow Rust
                        // to compile it. On the plus side, the unsafe call here lets us use the wrapped function in safe
                        // Rust code; if you trust your external code to be safe, this could be really nice.
                        ++m_indentationDepth;
                        out << std::string(m_indentationDepth, '\t') << "unsafe { polyglot_" << classNode->name << "_method_"
                            << method.functionName << "(self";
                        params.clear();
                        for (const auto &param : method.parameters)
                            params += ", " + param.name;
                        out << params << ") }\n";
                        --m_indentationDepth;

                        out << std::string(m_indentationDepth, '\t') << "}\n";
                    }
                    --m_indentationDepth;
                    out << "}\n\n";

                    // Now we'll write the bindings to the actual class methods. The name pattern here should hopefully not
                    // ever cause conflicts with user defined symbols; I don't see any reasonable case where it would cause a
                    // problem; any naming collisions will probably be a result of abuse rather than accidentally breaking
                    // things.
                    out << std::string(m_indentationDepth, '\t') << "extern {\n";
                    ++m_indentationDepth;
                    for (const auto &method : classNode->methods)
                    {
                        out << std::format("\t"
                                           R"(#[link_name = "{}"] fn polyglot_{}_method_{}(this: &mut {})",
                                           method.mangledName,
                                           classNode->name,
                                           method.functionName,
                                           classNode->name);

                        std::string params;
                        for (const auto &param : method.parameters)
                            params += ", " + param.name + ": " + getTypeString(param.type);
                        out << params << ')';

                        if (method.returnType.baseType != Type::Void)
                            out << " -> " << getTypeString(method.returnType);
                        out << ";\n";
                    }
                    --m_indentationDepth;
                    out << "}\n";
                }
            }
        }

        previousNodeType = node->nodeType();
    }

    if (previousNodeType == ASTNodeType::Function)
        out << "}\n";

    out.flush();

    --s_onlyWriteHeaderOnce;
}

std::string RustWrapperWriter::getTypeString(const QualifiedType &type) const
{
    std::string typeString;
    if (type.isReference)
        typeString += "ref ";

    switch (type.baseType)
    {
    case Type::Bool:
        typeString += "bool";
        break;
    case Type::Void:
        typeString += "void";
        break;
    case Type::Char:
        typeString += "i8";// "char";
        break;
    case Type::Char16:
        typeString += "i16";//"wchar";
        break;
    case Type::Char32:
        typeString += "char";
        break;
    case Type::Int8:
        typeString += "i8";
        break;
    case Type::Int16:
        typeString += "i16";
        break;
    case Type::Int32:
        typeString += "i32";
        break;
    case Type::Int64:
        typeString += "i64";
        break;
    case Type::Int128:
        typeString += "i128";
        break;
    case Type::Uint8:
        typeString += "u8";
        break;
    case Type::Uint16:
        typeString += "u16";
        break;
    case Type::Uint32:
        typeString += "u32";
        break;
    case Type::Uint64:
        typeString += "u64";
        break;
    case Type::Uint128:
        typeString += "u128";
        break;
    case Type::Float32:
        typeString += "f32";
        break;
    case Type::Float64:
        typeString += "f64";
        break;
        //    case Type::Float128:
        //        typeString += "real";
        //        break;
    case Type::Enum:
    case Type::Class:
        if (type.nameString.empty())
            throw std::runtime_error("Enum or class name was not provided to RustWrapperWriter");
        else
            typeString += type.nameString;
        break;
    case Type::CppStdString:
        typeString += "basic_string";
        break;
    case Type::Undefined:
    default:
        throw std::runtime_error("Undefined type in RustWrapperWriter::getTypeString()");
        break;
    }

    if (type.isPointer)
        typeString = (type.isConst ? "*const " : "*mut ") + typeString;

    return typeString;
}

std::string RustWrapperWriter::getValueString(const Value &value) const
{
    switch (value.type)
    {
    case Type::Bool:
        return std::to_string(std::get<bool>(value.value));
        break;
    case Type::Char:
        return std::to_string(std::get<char>(value.value));
        break;
    case Type::Char16:
        return std::to_string(std::get<char16_t>(value.value));
        break;
    case Type::Char32:
        return std::to_string(std::get<char32_t>(value.value));
        break;
    case Type::Int8:
    case Type::Int16:
    case Type::Int32:
    case Type::Int64:
        return std::to_string(std::get<int64_t>(value.value));
        break;
    case Type::Uint8:
    case Type::Uint16:
    case Type::Uint32:
    case Type::Uint64:
        return std::to_string(std::get<uint64_t>(value.value));
        break;
    case Type::Float32:
    case Type::Float64:
        return std::to_string(std::get<double>(value.value));
        break;
    case Type::Enum:
    case Type::Class:
        throw std::runtime_error("Enum or class expressions are not yet supported here");
        break;
    case Type::CppStdString:
        return std::get<std::string>(value.value);
        break;
    case Type::Int128:
    case Type::Uint128:
    case Type::Float128:
    case Type::Void:
    case Type::Undefined:
    default:
        throw std::runtime_error("Bad or unsupported type in RustWrapperWriter::getValueString()");
        break;
    }
}
