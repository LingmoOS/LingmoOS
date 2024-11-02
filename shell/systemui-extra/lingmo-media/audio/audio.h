/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef AUDIO_H
#define AUDIO_H

#include <QWidget>
#include <QObject>
#include <QtPlugin>
#include <QProcess>
#include "ukmedia_main_widget.h"
#include <ukcc/interface/interface.h>
#include <QTranslator>
#include <QtCore/qglobal.h>

#if defined(LIBUKCC_LIBRARY)
#  define LIBUKCC_EXPORT Q_DECL_EXPORT
#else
#  define LIBUKCC_EXPORT Q_DECL_IMPORT
#endif

namespace Ui {
class Audio;
}

class Audio : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:

    Audio();
    ~Audio();

    QString plugini18nName()        Q_DECL_OVERRIDE;
    int pluginTypes()               Q_DECL_OVERRIDE;
    QWidget *pluginUi()             Q_DECL_OVERRIDE;
    bool isEnable() const           Q_DECL_OVERRIDE;
    const QString name() const      Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const   Q_DECL_OVERRIDE;
    QIcon icon() const              Q_DECL_OVERRIDE;
    QString translationPath() const Q_DECL_OVERRIDE;

private:
    QString pluginName;
    int pluginType;
    QWidget *widget;
    bool mFirstLoad;

private:
    void initSearchText();      // 搜索翻译
};


#endif // AUDIO_H
