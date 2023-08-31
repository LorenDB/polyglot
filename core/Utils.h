// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <string>

namespace Utils
{
    constexpr auto POLYGLOT_VERSION = "0.0.1-devel";

    std::string getModuleName(std::string filename);
} // namespace Utils
