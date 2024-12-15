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

#ifndef KCM_FCITX_GLOBAL_H
#define KCM_FCITX_GLOBAL_H

// Qt
#include <QHash>
#include <QTimer>

// Fcitx
#include <fcitx-config/fcitx-config.h>
#include <fcitxqtinputmethodproxy.h>
#include <fcitxqtkeyboardproxy.h>

class FcitxQtKeyboardProxy;
class FcitxQtInputMethodProxy;
class FcitxQtConnection;
namespace Fcitx {

class Global : public QObject
{
    Q_OBJECT
public:
    static Global *instance();
    static void deInit();
    virtual ~Global();
    FcitxConfigFileDesc *GetConfigDesc(const QString &name);
    FcitxQtConnection *connection() { return m_connection; }
    FcitxQtInputMethodProxy *inputMethodProxy() { return (m_inputmethod && m_inputmethod->isValid()) ? m_inputmethod : 0; }
    FcitxQtKeyboardProxy *keyboardProxy() { return (m_keyboard && m_keyboard->isValid()) ? m_keyboard : 0; }
    QString testWrapper(const QString &path) const;

signals:
    void connectStatusChanged(bool connected);

private slots:
    void connected();
    void disconnected();

private:
    Global();
    QHash<QString, FcitxConfigFileDesc *> *m_hash;
    FcitxQtConnection *m_connection;
    FcitxQtInputMethodProxy *m_inputmethod;
    FcitxQtKeyboardProxy *m_keyboard;
    static Global *inst;
    QTimer *m_timer;
};

} // namespace Fcitx

#endif
