/***************************************************************************
 *   This file was modified by Uniontech Technology Co., Ltd.              *
 *   Copyright (C) 2011~2011 by CSSlayer                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

// Fcitx
#include <fcitx-config/fcitx-config.h>
#include <fcitx-config/xdg.h>
#include <fcitxqtconnection.h>
#include <fcitxqtinputmethodproxy.h>

// self
#include "global.h"

namespace Fcitx {
Global *Global::inst = nullptr;

Global *Global::instance()
{
    if (!inst)
        inst = new Global;
    return inst;
}

void Global::deInit()
{
    if (inst) {
        inst->deleteLater();
        inst = 0;
    }
}

Global::Global()
    : m_hash(new QHash<QString, FcitxConfigFileDesc *>)
    , m_connection(new FcitxQtConnection(this))
    , m_inputmethod(nullptr)
    , m_keyboard(nullptr)
    , m_timer(new QTimer)
{
    connect(m_connection, SIGNAL(connected()), this, SLOT(connected()));
    connect(m_connection, SIGNAL(disconnected()), this, SLOT(disconnected()));

    m_connection->startConnection();
    m_timer->setSingleShot(true);
}

Global::~Global()
{
    QHash<QString, FcitxConfigFileDesc *>::iterator iter;

    for (iter = m_hash->begin();
            iter != m_hash->end();
            iter++) {
        FcitxConfigFreeConfigFileDesc(iter.value());
    }

    delete m_hash;
}

void Global::connected()
{
    if(m_timer->isActive()) {
        m_timer->stop();
        return;
    }
    if (m_inputmethod)
        delete m_inputmethod;

    if (m_keyboard)
        delete m_keyboard;

    m_inputmethod = new FcitxQtInputMethodProxy(
        m_connection->serviceName(),
        "/inputmethod",
        *m_connection->connection(),
        this);

    m_keyboard = new FcitxQtKeyboardProxy(
        m_connection->serviceName(),
        "/keyboard",
        *m_connection->connection(),
        this);

#if QT_VERSION >= QT_VERSION_CHECK(4, 8, 0)
    m_inputmethod->setTimeout(3000);
    m_keyboard->setTimeout(3000);
#endif
    qDebug() << "connected";
    emit connectStatusChanged(true);
}

void Global::disconnected()
{
    m_timer->start(2000);
    connect(m_timer, &QTimer::timeout, this, [=]() {
        if (m_inputmethod)
            delete m_inputmethod;
        m_inputmethod = 0;
        if (m_keyboard)
            delete m_keyboard;
        m_keyboard = 0;
        qDebug() << "disconnected";
        emit connectStatusChanged(false);
    });
}

FcitxConfigFileDesc *Global::GetConfigDesc(const QString &name)
{
    if (m_hash->count(name) <= 0) {
        FILE *fp = FcitxXDGGetFileWithPrefix("configdesc", name.toLatin1().constData(), "r", NULL);
        FcitxConfigFileDesc *cfdesc = FcitxConfigParseConfigFileDescFp(fp);

        if (cfdesc)
            m_hash->insert(name, cfdesc);

        return cfdesc;
    } else
        return (*m_hash)[name];
}

QString Global::testWrapper(const QString &path) const
{
    char *qtguiwrapper[] = {
        fcitx_utils_get_fcitx_path_with_filename("libdir", "fcitx/libexec/fcitx-qt5-gui-wrapper"),
        fcitx_utils_get_fcitx_path_with_filename("libdir", "fcitx/libexec/fcitx-qt-gui-wrapper")
    };
    QString wrapper;
    for (int i = 0; i < FCITX_ARRAY_SIZE(qtguiwrapper); i++) {
        if (qtguiwrapper[i]) {
            QStringList args;
            args << QLatin1String("--test");
            args << path;
            int exitStatus = QProcess::startDetached("sh -c " + QString::fromLocal8Bit(qtguiwrapper[i]), args);
            if (exitStatus == 0) {
                wrapper = QString::fromLatin1(qtguiwrapper[i]);
                break;
            }
        }
    }

    return wrapper;
}

} // namespace Fcitx
