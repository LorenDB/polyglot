// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

#include "Utils.h"

#include <iostream>

bool Utils::isStdString(const clang::QualType &type)
{
    auto t = type.getTypePtr();
    const auto *record = t->isPointerType() ? t->getPointeeCXXRecordDecl() : t->getAsCXXRecordDecl();
    if (!record)
      return false;
    return record->getQualifiedNameAsString() == "std::basic_string";
}

bool Utils::isFixedWidthIntegerType(const clang::QualType &type)
{
    auto checkName = [](const std::string_view name) {
        if (name == "int8_t" || name == "int16_t" || name == "int32_t" || name == "int64_t" || name == "uint8_t" ||
            name == "uint16_t" || name == "uint32_t" || name == "uint64_t")
            return true;
        return false;
    };

    if (!type->isTypedefNameType())
        return false;
    if (checkName(type.getAsString()))
        return true;

    const clang::TypedefType *t = type->getAs<clang::TypedefType>();
    while (t)
    {
        auto underlying = t->getDecl()->getUnderlyingType();
        if (checkName(underlying.getAsString()))
            return true;
        t = underlying->getAs<clang::TypedefType>();
    }
    return false;
}
