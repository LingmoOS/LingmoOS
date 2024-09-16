// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPLICATIONADPATOR_H
#define APPLICATIONADPATOR_H

#include "../filecontrol.h"

#include <QtDBus>

class ApplicationAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.imageViewer")
    Q_CLASSINFO("D-Bus Introspection",
                "<interface name=\"com.deepin.imageViewer\">\n"
                "    <method name=\"openImageFile\">\n"
                "        <arg direction=\"in\" type=\"s\" name=\"fileName\"/>\n"
                "    </method>\n"
                "</interface>\n")

public:
    explicit ApplicationAdaptor(FileControl *controller);

public Q_SLOTS:
    // 打开图片文件
    bool openImageFile(const QString &fileName);

private:
    FileControl *fileControl = nullptr;
};

#endif  // APPLICATIONADPATOR_H
