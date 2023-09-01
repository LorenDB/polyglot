// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

#pragma once

#include "WrapperWriter.h"

class DWrapperWriter : public WrapperWriter
{
public:
    ~DWrapperWriter();

    void write(const polyglot::AST &ast, std::ostream &out) final;

protected:
    std::string getTypeString(const polyglot::QualifiedType &type) const final;
    std::string getValueString(const polyglot::Value &value) const final;

private:
    struct NamespaceOrganizer
    {
        polyglot::Namespace currentNamespace;
        std::vector<polyglot::ASTNode *> childNodes;
        std::vector<NamespaceOrganizer *> childNamespaces;
    };

    void writeFromNamespaceOrganizer(const polyglot::AST &ast, const NamespaceOrganizer *organizer, std::ostream &out);

    NamespaceOrganizer m_namespaceOrganizer;
    int16_t m_indentationDepth = 0;
};
