// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "util_updateui.h"

void updateStyle(QString styleFiles, QWidget* widget){
    QFile qssFile(QString("%1").arg(styleFiles));

    QString qss;
    qssFile.open(QFile::ReadOnly);
    if(qssFile.isOpen()) {
        qss = QLatin1String(qssFile.readAll());
        widget->setStyleSheet(qss);
        qssFile.close();
    }
}

