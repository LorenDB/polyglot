// SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
//
// SPDX-License-Identifier: GPL-3.0

#include "TypeProxyWriter.h"

class CppTypeProxyWriter : public TypeProxyWriter
{
public:
    CppTypeProxyWriter() {}

    virtual void generateNeededProxies(polyglot::AST &ast, std::ostream &out) override;
};
