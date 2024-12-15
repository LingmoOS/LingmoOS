// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QObject>

class Utils : public QObject
{
    Q_OBJECT
    
public:
    static bool fileExists(QString path);
    static bool fileIsWritable(QString path);
};
