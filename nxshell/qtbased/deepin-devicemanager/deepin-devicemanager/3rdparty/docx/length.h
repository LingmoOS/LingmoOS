// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LENGTH_H
#define LENGTH_H

#include "docx_global.h"

namespace Docx {
class DOCX_EXPORT Length
{
public:
    Length();
    Length(int emu);
    float cm() const;
    int emu() const;
    float inches() const;
    float mm() const;
    int px() const;
    int twips() const;
    bool isEmpty() const;

private:
    bool m_isEmpty = true;
    int m_value;

};

class DOCX_EXPORT Inches
{
public:
    static Length emus(float inches);
};

class DOCX_EXPORT Cm
{
public:
    static Length emus(float cm);
};

class DOCX_EXPORT Emu
{
public:
    static Length emus(float emu);
};

class DOCX_EXPORT Mm
{
public:
    static Length emus(float mm);
};

class DOCX_EXPORT Pt
{
public:
    static Length emus(float pts);

};

class DOCX_EXPORT Px
{
public:
    static Length emus(float px);
};

class DOCX_EXPORT Twips
{
public:
    static Length emus(float twips);
};
}

#endif // LENGTH_H
