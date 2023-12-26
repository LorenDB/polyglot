// SPDX-FileCopyrightText: Matheus Catarino França
//
// SPDX-License-Identifier: GPL-3.0

#include "ZigWrapperWriter.h"

#include <ctime>
#include <format>
#include <string>
#include <iostream>

#include "Utils.h"

using namespace polyglot;

void ZigWrapperWriter::write(const AST &ast, std::ostream &out)
{
    static int s_onlyWriteHeaderOnce = 0;
    if (s_onlyWriteHeaderOnce == 0)
    {
        auto t = std::time(nullptr);
        std::string timeStr = std::asctime(std::localtime(&t));
        out << std::format(
R"(// *** WARNING: autogenerated file, do not modify. Changes will be overwritten. ***
// Generated by Polyglot version {} at {}.
// This file contains symbols that have been exported from {} into Zig.
)",
            Utils::POLYGLOT_VERSION,
            timeStr.substr(0, timeStr.size() - 1), // remove the '\n'
            Utils::getLanguageName(ast)) << "\n";
    }
    ++s_onlyWriteHeaderOnce;

    auto previousNodeType = ASTNodeType::Undefined;
    for (const auto &node : ast.nodes)
    {
        if (node->nodeType() == ASTNodeType::Namespace)
        {
            auto ns = dynamic_cast<NamespaceNode *>(node);
            if (ns == nullptr)
                throw std::runtime_error("Node claimed to be NamespaceNode, but cast failed");
        }
        else if (node->nodeType() == ASTNodeType::Function)
        {
            auto function = dynamic_cast<FunctionNode *>(node);
            if (function == nullptr)
                throw std::runtime_error("Node claimed to be FunctionNode, but cast failed");
            // extern "c++" need llvm-libc++.
            out << std::format(R"({}extern "c++" fn @"{}" )",
                               std::string(m_indentationDepth, '\t'),
                               function->mangledName)
               /*<< function->functionName*/ << '(';

            std::string params;
            // note that Zig doesn't support default arguments
            for (const auto &param : function->parameters)
                params += param.name + ": " + getTypeString(param.type) + ", ";
            out << params.substr(0, params.size() - 2) + ')';

            out << " " << getTypeString(function->returnType);
            out << ";\n";
            // function alias
            out << std::format("pub const {} = {};\n\n", function->functionName, function->mangledName);
        }
        else
        {
            if (node->nodeType() == ASTNodeType::Enum)
            {
                auto e = dynamic_cast<EnumNode *>(node);
                if (e == nullptr)
                    throw std::runtime_error("Node claimed to be EnumNode, but cast failed");
                
                std::string tag{""};
                // TODO: missing tagType
                if(e->tagType.nameString.empty()){
                    tag = "enum(c_int)"; // for example
                } else {
                    tag = std::format("enum({})", getTypeString(e->tagType));
                }

                out << std::string(m_indentationDepth, '\t') << "pub const " << e->enumName << " = " << tag << " {\n";
                ++m_indentationDepth;
                for (const auto &enumerator : e->enumerators)
                {
                    out << std::string(m_indentationDepth, '\t') << enumerator.name;
                    if (enumerator.value.has_value())
                        out << " = " + getValueString(enumerator.value.value());
                    out << ",\n";
                }
                --m_indentationDepth;
                out << std::string(m_indentationDepth, '\t') << "};\n";
            }
            else if (node->nodeType() == ASTNodeType::Class)
            {
                auto classNode = dynamic_cast<ClassNode *>(node);
                if (classNode == nullptr)
                    throw std::runtime_error("Node claimed to be ClassNode, but cast failed");

                out << std::string(m_indentationDepth, '\t') << "pub const " << classNode->name << " = extern struct " << " {\n";
                ++m_indentationDepth;
                for (const auto &member : classNode->members)
                {
                    out << std::string(m_indentationDepth, '\t')
                        << member.name + ": " + getTypeString(member.type);
                    if (member.value.has_value())
                        out << " = " << getValueString(member.value.value());
                    out << ",\n";
                }
                if(classNode->methods.empty())
                out << "};\n";

                // TODO: wrap constructors and destructors here

                if (!classNode->methods.empty())
                {
                    // First we will write an impl block. The impl block will contain function definitions that will be
                    // responsible for calling the actual functions. This probably doesn't support virtual functions yet.
                    // Eventually I intend to see how tools like bindgen or cxx.rs handle virtual functions and copy that
                    // method.
                    for (const auto &method : classNode->methods)
                    {
                        out << std::string(m_indentationDepth, '\t')
                            << std::format("pub fn {} (self: {}", method.functionName, classNode->name);

                        std::string params;
                        for (const auto &param : method.parameters)
                            params += ", " + param.name + ": " + getTypeString(param.type);
                        out << params << ')';

                        out << getTypeString(method.returnType);
                        out << " {\n";

                        // Write the call to the actual member function. Note that we have to wrap it as unsafe to allow Zig
                        // to compile it. On the plus side, the unsafe call here lets us use the wrapped function in safe
                        // Zig code; if you trust your external code to be safe, this could be really nice.
                        ++m_indentationDepth;
                        out << std::string(m_indentationDepth, '\t') << "polyglot_" << classNode->name << "_method_"
                            << method.functionName << "(self";
                        params.clear();
                        for (const auto &param : method.parameters)
                            params += ", " + param.name;
                        out << params << ");\n\t}\n";
                        --m_indentationDepth;

                        out << std::string(m_indentationDepth, '\t') << "\n";
                    }
                    --m_indentationDepth;
                    out << "};\n\n";

                    // Now we'll write the bindings to the actual class methods. The name pattern here should hopefully not
                    // ever cause conflicts with user defined symbols; I don't see any reasonable case where it would cause a
                    // problem; any naming collisions will probably be a result of abuse rather than accidentally breaking
                    // things.
                    for (const auto &method : classNode->methods)
                    {
                        out << std::format(
                                           R"(extern "c++" fn @"{}" (this: {})",
                                           method.mangledName,
                                           classNode->name);

                        std::string params;
                        for (const auto &param : method.parameters)
                            params += ", " + param.name + ": " + getTypeString(param.type);
                        out << params << ") " << getTypeString(method.returnType);

                        if (method.returnType.baseType != Type::Void)
                            out << getTypeString(method.returnType);
                        out << ";\n";
                        // function alias
                        out << std::format("pub const polyglot_{}_method_{} = {};\n\n",
                                           classNode->name,
                                           method.functionName,
                                           method.mangledName);
                    }
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

std::string ZigWrapperWriter::getTypeString(const QualifiedType &type) const
{
    std::string typeString;
    if (type.isConst)
        typeString += "const ";
    if (type.isReference)
        throw std::runtime_error("Zig not have reference keywords");
    if (type.isPointer)
        typeString += "?*";

    // Ptr format: C-like (T*), Zig (*T)
    // zig ptr not infer nullable, only optional or c-ptr:
    // x: *T = null; compiler error
    // x: [*]T = null; error again
    // x: [*c]T = null; c-ptr on zig - Ok
    // x: ?*T = null; optional ptr - OK

    switch (type.baseType)
    {
    case Type::Bool:
        typeString += "bool";
        break;
    case Type::Void:
        typeString += "void";
        break;
    case Type::Char:
        typeString += "u8";// "char";
        break;
    case Type::Char16:
        typeString += "u16";//"wchar";
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
    case Type::Float128:
        typeString += "f128";
        break;
    case Type::Enum:
    case Type::Class:
        if (type.nameString.empty())
            throw std::runtime_error("Enum or class name was not provided to ZigWrapperWriter");
        else
            typeString += type.nameString;
        break;
    case Type::CppStdString:
        typeString += "basic_string";
        break;
    case Type::Undefined:
    default:
        throw std::runtime_error("Undefined type in ZigWrapperWriter::getTypeString()");
        break;
    }

    return typeString;
}

std::string ZigWrapperWriter::getValueString(const Value &value) const
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
        throw std::runtime_error("Bad or unsupported type in ZigWrapperWriter::getValueString()");
        break;
    }
}