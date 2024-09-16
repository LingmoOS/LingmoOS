// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSOCR_ADAPTOR_H
#define DBUSOCR_ADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * @bref: dbusocr_adaptor 提供给外部程序调用的方法
*/
class DbusOcrAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.Ocr")
    Q_CLASSINFO("D-Bus Introspection", ""
                                       "  <interface name=\"com.deepin.Ocr\">\n"

                                       "    <method name=\"openImage\">\n"
                                       "      <arg direction=\"in\" type=\"ay\" name=\"openImage\"/>\n"
                                       "    </method>\n"

                                       "    <method name=\"openImageAndName\">\n"
                                       "      <arg direction=\"in\" type=\"ay\" name=\"images\"/>\n"
                                       "      <arg direction=\"in\" type=\"s\" name=\"imageName\"/>\n"
                                       "    </method>\n"

                                       "    <method name=\"openFile\">\n"
                                       "      <arg direction=\"in\" type=\"s\" name=\"openFile\"/>\n"
                                       "      <arg direction=\"out\" type=\"b\"/>\n"
                                       "    </method>\n"

                                       "  </interface>\n")
public:
    explicit DbusOcrAdaptor(QObject *parent);
    virtual ~DbusOcrAdaptor();

public Q_SLOTS: // METHODS
    void openImage(QByteArray images);
    void openImageAndName(QByteArray images,QString imageName);

    bool openFile(QString filePath);

Q_SIGNALS: // SIGNALS
};

#endif // DBUSDRAW_ADAPTOR_H
