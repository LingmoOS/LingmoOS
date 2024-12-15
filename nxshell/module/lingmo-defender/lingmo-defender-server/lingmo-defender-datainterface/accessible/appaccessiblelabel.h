// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPACCESSIBLELABEL_H
#define APPACCESSIBLELABEL_H

#include "appaccessiblebase.h"

#include <QLabel>

class AppAccessibleLabel : public AppAccessibleBase
{
public:
    explicit AppAccessibleLabel(QLabel *);
    virtual QString text(int startOffset, int endOffset) const Q_DECL_OVERRIDE;

private:
    QLabel *m_widget;
};

#endif // APPACCESSIBLELABEL_H
