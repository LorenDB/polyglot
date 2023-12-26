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
    //! Generally, this represents the language that an AST has been parsed from. However, there are some other uses for it.
    //!
    //! A language's inclusion in this list does not mean that it is fully supported by Polyglot or even that it is supported
    //! at all. Languages are sometimes added in anticipation of their use to prevent having to modify this enum later.
    enum class Language
    {
        // These languages have some level of support in Polyglot
        Cpp,
        D,
        Rust,

        // These languages have planned support, but lack an implementation
        Swift,

        // Support for these languages is likely possible, but it is uncertain if or when they will recieve implementations
        Go,
        Nim,
        OCaml,
        Pascal,
        V,
        Vala,
        Zig,
    };

    //! This enum exists to allow you to use ASTNode::nodeType() to detect what an ASTNode actually is so you can upcast it
    //! to its proper type.
    enum class ASTNodeType
    {
        Function,
        Class,
        Enum,
        Variable,
        Namespace,

        Undefined,
    };

    //! Represents a basic type.
    //!
    //! All the basic builtin types are present in this enum, as well as indicators for user-defined types; however, this
    //! enum is inadequate for actually representing any realistic type information. For that, use the QualifiedType struct,
    //! which provides information about qualifiers (e.g. const), whether the type is a pointer or array type, and names for
    //! user defined types.
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

    //! The base class of all Polyglot AST nodes.
    struct ASTNode
    {
        //! Returns the node type. Use this to determine what to upcast the current object to.
        virtual ASTNodeType nodeType() const = 0;

        //! If not null, this represents the C++ namespace the node is part of.
//        std::shared_ptr<Namespace> cppNamespace;
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

    //! Represents a namespace.
    //!
    //! The concept of a namespace is most commonly taught from C++. However, some other languages have similarly powerful
    //! namespacing features; for example, Rust has a nested modules system that allows for C++-like namespacing. When
    //! supported, this struct should be handled
    struct NamespaceNode : public ASTNode
    {
        virtual ASTNodeType nodeType() const override;

        //! The name of this namespace.
        std::string name;

        //! The contents of the namespace.
        AST ast;
    };

    //! QualifiedType holds a Type with any additional qualifiers like "const". In addition, if the type is something like
    //! Enum or Class, QualifiedType will contain information about that type (e.g. an enum name).
    struct QualifiedType
    {
        //! The base type of the qualified type.
        Type baseType = Type::Undefined;

        //! Whether the type is const.
        bool isConst = false;

        //! Whether the type is a pointer.
        bool isPointer = false;

        //! Whether the type is volatile.
        bool isVolatile = false;

        //! Whether the type is an array.
        bool isArray = false;

        //! Whether the type is a reference.
        bool isReference = false;

        //! Whether the type is an rvalue reference. Mainly useful in C++ move constructors.
        bool isRvalueReference = false;

        //! This is only set if baseType is equal to Type::Class or Type::Enum.
        std::string nameString;

        bool operator==(const QualifiedType &other) const = default;
    };

    //! A raw value used in an expression (e.g. a default argument).
    struct Value
    {
        //! The type that this value has.
        Type type;

        //! The actual value.
        std::variant<bool, char, char16_t, char32_t, int64_t, uint64_t, double, std::string> value;
    };

    //! Represents a variable.
    //!
    //! This struct can be used for multiple purposes; it can represent global variables, function parameters, or class
    //! members.
    struct VariableNode : public ASTNode
    {
        virtual ASTNodeType nodeType() const override;

        //! The variable type.
        QualifiedType type;

        //! The variable name.
        std::string name;

        //! If the variable has a value set (e.g. a default argument for a function parameter), this holds that value.
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

        //! If a type proxy function has been created for this function, a representation will be stored here.
        struct TypeProxy
        {
            //! Whether the parent function is actually proxied.
            bool isValid = false;

            //! Whether the return value of the function is proxied.
            bool isReturnProxied = false;

            //! Contains the name of all parameters that should have their types proxied.
            std::vector<std::string> proxiedParameters;

            //! The representation of the proxy function.
            FunctionNode *proxy = nullptr;
        } typeProxy;
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
        
        //! Inferred tag type
        QualifiedType tagType;

        virtual ASTNodeType nodeType() const override;

        //! The name of the enum.
        std::string enumName;

        //! The enumerators.
        std::vector<Enumerator> enumerators;
    };

    struct ClassNode : public ASTNode
    {
        //! Represents whether this a a class or a struct. Generally, the class and struct keywords mean more or less the
        //! same thing, but there are situations in which it can be useful to know how a class was declared.
        enum class Type
        {
            Class,
            Struct,
        };

        virtual ASTNodeType nodeType() const override;

        //! The class name.
        std::string name;

        //! Whether this was declared as a class or a struct.
        Type type;

        //! The constructors for this class.
        std::vector<FunctionNode> constructors;

        //! The destructor for this class.
        std::optional<FunctionNode> destructor;

        //! Member variables for the class.
        std::vector<VariableNode> members;

        //! The class methods.
        std::vector<FunctionNode> methods;
    };
} // namespace polyglot
