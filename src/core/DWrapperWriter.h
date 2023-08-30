#pragma once

#include "WrapperWriter.h"

class DWrapperWriter : public WrapperWriter
{
public:
    void write(const polyglot::AST &ast, std::ostream &out) const final;

protected:
    std::string getTypeString(const polyglot::QualifiedType &type) const final;
    std::string getValueString(const polyglot::Value &value) const final;
};
