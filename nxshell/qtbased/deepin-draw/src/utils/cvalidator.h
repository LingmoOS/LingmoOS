// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CVALIDATOR_H
#define CVALIDATOR_H

#include <QIntValidator>

class CIntValidator : public QIntValidator
{
public:
    explicit CIntValidator(int bottom, int top, QObject *parent = nullptr);

    void setEmptyStrToBottom(bool b);
    //bool emptyStrIsBottom()const;

    virtual State validate(QString &, int &) const override;
    virtual void fixup(QString &) const override;


private:
    bool m_emptyStrIsBottom = true;
};

#endif // CVALIDATOR_H
