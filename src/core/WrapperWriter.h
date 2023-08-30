// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

#pragma once

#include "PolyglotAST.h"

class WrapperWriter
{
public:
    WrapperWriter();

    virtual void write(const polyglot::AST &ast, std::ostream &out) const = 0;

protected:
    virtual std::string getTypeString(const polyglot::QualifiedType &type) const = 0;
    virtual std::string getValueString(const polyglot::Value &value) const = 0;
};
