// SPDX-FileCopyrightText: Loren Burkholder
// SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
//
// SPDX-License-Identifier: GPL-3.0

#include "CppWrapperWriter.h"

#include <algorithm>
#include <ctime>
#include <format>
#include <iostream>
#include <stack>
#include <string>

#include "Utils.h"

using namespace polyglot;

void CppWrapperWriter::write(const AST &ast, std::ostream &out)
{
    throw std::runtime_error("CppWrapperWriter cannot write wrappers yet (it is only used to enable creating type proxies");
}

std::string CppWrapperWriter::getTypeString(const QualifiedType &type) const
{
    std::string typeString;
    if (type.isConst)
        typeString += "const ";
    if (type.isReference)
        typeString += "& ";

    switch (type.baseType)
    {
    case Type::Bool:
        typeString += "bool";
        break;
    case Type::Void:
        typeString += "void";
        break;
    case Type::Char:
        typeString += "char";
        break;
    case Type::Char16:
        typeString += "char16_t";
        break;
    case Type::Char32:
        typeString += "char32_t";
        break;
    case Type::Int8:
        typeString += "int8_t";
        break;
    case Type::Int16:
        typeString += "int16_t";
        break;
    case Type::Int32:
        typeString += "int32_t";
        break;
    case Type::Int64:
        typeString += "int64_t";
        break;
        break;
    case Type::Uint8:
        typeString += "uint8_t";
        break;
    case Type::Uint16:
        typeString += "uint16_t";
        break;
    case Type::Uint32:
        typeString += "uint32_t";
        break;
    case Type::Uint64:
        typeString += "uint64_t";
        break;
    case Type::Int128:
    case Type::Uint128:
        throw std::runtime_error("C++ does not support 128-bit integers");
        break;
    case Type::Float32:
        typeString += "float";
        break;
    case Type::Float64:
        typeString += "double";
        break;
    case Type::Float128:
        // TODO: figure out if there is a workaround for platforms that don't have 128-bit long double
        // 16 bytes == 128 bits
        static_assert(sizeof(long double) == 16);
        typeString += "long double";
        break;
    case Type::Enum:
    case Type::Class:
        if (type.nameString.empty())
            throw std::runtime_error("Enum or class name was not provided to DWrapperWriter");
        else
            typeString += type.nameString;
        break;
    case Type::CppStdString:
        typeString += "std::string";
        break;
    case Type::Undefined:
        throw std::runtime_error("Undefined type in CppWrapperWriter::getTypeString()");
        break;
    }

    if (type.isPointer)
        typeString += " *";

    return typeString;
}

std::string CppWrapperWriter::getValueString(const Value &value) const
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
    case Type::Float128:
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
    case Type::Void:
    case Type::Undefined:
    default:
        throw std::runtime_error("Bad or unsupported type in CppWrapperWriter::getValueString()");
        break;
    }
}
