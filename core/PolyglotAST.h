// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace polyglot
{
    enum class Language
    {
        Cpp,
        D,
        Rust,
        Swift, // not yet supported but currently planned longer-term
    };

    enum class ASTNodeType
    {
        Function,
        Class,
        Enum,
        Variable,

        Undefined,
    };

    enum class Type
    {
        // the main built-in types
        Void,
        Bool,
        Char,
        Char16,
        Char32,
        Int8,
        Int16,
        Int32,
        Int64,
        Int128,
        Uint8,
        Uint16,
        Uint32,
        Uint64,
        Uint128,
        Float32,
        Float64,
        Float128,

        // User-defined types
        Enum,
        Class,

        // Types that are known to need indirect bindings (at least in some cases)
        CppStdString,

        Undefined,
    };

    struct Namespace
    {
        std::shared_ptr<Namespace> parentNamespace;
        std::string name;
    };

    //! The base class of all Polyglot AST nodes.
    struct ASTNode
    {
        //! Returns the node type. Use this to determine what to upcast the current object to.
        virtual ASTNodeType nodeType() const = 0;

        //! If not null, this represents the C++ namespace the node is part of.
        std::shared_ptr<Namespace> cppNamespace;
    };

    //! QualifiedType holds a Type with any additional qualifiers like "const". In addition, if the type is something like
    //! Enum or Class, QualifiedType will contain information about that type (e.g. an enum name).
    struct QualifiedType
    {
        Type baseType = Type::Undefined;
        bool isConst = false;
        bool isPointer = false;
        bool isVolatile = false;
        bool isArray = false;
        bool isReference = false;
        bool isRvalueReference = false;

        //! This is only set if baseType is equal to Type::Class or Type::Enum.
        std::string nameString;
    };

    struct Value
    {
        Type type;
        std::variant<bool, char, char16_t, char32_t, int64_t, uint64_t, double, std::string> value;
    };

    struct VariableNode : public ASTNode
    {
        virtual ASTNodeType nodeType() const override;
        QualifiedType type;
        std::string name;
        std::optional<Value> value;
    };

    //! Represents a function.
    struct FunctionNode : public ASTNode
    {
        virtual ASTNodeType nodeType() const override;

        //! The name of the function.
        std::string functionName;

        //! This contains what the function will be mangled to by the compiler.
        std::string mangledName;

        //! The return type of the function.
        QualifiedType returnType;

        //! The parameter list for the function. See FunctionParameter for more details.
        std::vector<VariableNode> parameters;

        //! Whether the function is marked noreturn.
        bool isNoreturn;

        //! Whether the function is guaranteed to not throw exceptions.
        bool isNothrow;

        //! If the function is part of a class, whether the function is static.
        bool isStatic;

        //! If the function is part of a class, whether the function is virtual.
        bool isVirtual;

        //! If the function is part of a class, whether the function is marked override.
        bool isOverride;

        //! If the function is part of a class, whether the function is marked final.
        bool isFinal;
    };

    struct EnumNode : public ASTNode
    {
        struct Enumerator
        {
            //! The name of the enumerator
            std::string name;

            //! If the enumerator has an explicit value set, it will be stored here.
            std::optional<Value> value;
        };

        virtual ASTNodeType nodeType() const override;
        std::string enumName;
        std::vector<Enumerator> enumerators;
    };

    struct ClassNode : public ASTNode
    {
        enum class Type
        {
            Class,
            Struct,
        };

        virtual ASTNodeType nodeType() const override;
        std::string name;
        Type type;
        std::vector<FunctionNode> constructors;
        std::optional<FunctionNode> destructor;
        std::vector<VariableNode> members;
        std::vector<FunctionNode> methods;
    };

    //! A top-level collection of symbols from a source file. An instance of AST corresponds to precisely one source module.
    struct AST
    {
        //! The contents of this AST.
        //!
        //! Generally, you will want to iterate over each node and call nodeType() on it to determine what kind of node it
        //! is; once you have retrieved the node type, you can upcast the node to a more appropriate type like FunctionNode
        //! or EnumNode.
        std::vector<ASTNode *> nodes;

        //! The source language that this AST is representing.
        Language language;

        //! The name of the module that this AST represents.
        std::string moduleName;
    };
} // namespace polyglot
