// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

double foo1(float foo, float bar = 12.345, char baz = 'c', bool b = true, void *v = nullptr)
{
   return static_cast<double>(foo * bar - static_cast<double>(baz));
}

void foo2(char a, wchar_t b, char32_t c, char16_t d);
void foo3(float a, double b, long double c);
