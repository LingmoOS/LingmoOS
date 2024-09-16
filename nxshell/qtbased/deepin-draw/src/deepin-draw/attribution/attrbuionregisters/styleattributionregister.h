// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STYLEATTRIBUTIONREGISTER_H
#define STYLEATTRIBUTIONREGISTER_H

#include "iattributionregister.h"

class StyleAttriWidget;

class StyleAttributionRegister : public IAttributionRegister
{
    Q_OBJECT
public:
    explicit StyleAttributionRegister(DrawBoard *drawboard);
    virtual void registe() override;
    StyleAttriWidget *getStyleWidget();
private:
    void registeStyles();

private:
    StyleAttriWidget *m_styleAttri = nullptr;
};

#endif // STYLEATTRIBUTIONREGISTER_H
