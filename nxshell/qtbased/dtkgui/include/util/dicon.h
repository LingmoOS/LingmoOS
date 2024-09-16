// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DICON_H
#define DICON_H

#include <QIcon>
#include <dtkgui_global.h>

DGUI_BEGIN_NAMESPACE
class DIcon : public QIcon
{
public:
    explicit DIcon(const QIcon &other);
    virtual ~DIcon();
    QPixmap pixmap(const QSize &size, qreal devicePixelRatio, Mode mode = Normal, State state = Off);

    static QPixmap loadNxPixmap(const QString &fileName);
};

DGUI_END_NAMESPACE
#endif // DICON_H
