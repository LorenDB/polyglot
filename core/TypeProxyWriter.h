// SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
//
// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <sstream>

#include "PolyglotAST.h"

class TypeProxyWriter
{
public:
    TypeProxyWriter() {}
    virtual ~TypeProxyWriter() {}

    virtual void generateNeededProxies(polyglot::AST &ast, std::ostream &out) = 0;
};
