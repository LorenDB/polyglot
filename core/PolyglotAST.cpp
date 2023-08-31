// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

#include "PolyglotAST.h"

polyglot::ASTNodeType polyglot::FunctionNode::nodeType() const
{
    return ASTNodeType::Function;
}

polyglot::ASTNodeType polyglot::EnumNode::nodeType() const
{
    return ASTNodeType::Enum;
}
