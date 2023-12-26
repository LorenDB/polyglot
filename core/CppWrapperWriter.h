// SPDX-FileCopyrightText: Loren Burkholder
// SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
//
// SPDX-License-Identifier: GPL-3.0

#pragma once

#include "WrapperWriter.h"

class CppWrapperWriter : public WrapperWriter
{
public:
    void write(const polyglot::AST &ast, std::ostream &out) final;

protected:
    std::string getTypeString(const polyglot::QualifiedType &type) const final;
    std::string getValueString(const polyglot::Value &value) const final;

    friend class CppTypeProxyWriter;

private:
    int16_t m_indentationDepth = 0;
};
