/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "status-bar.h"
#include "fm-window.h"
#include "file-info.h"

#include <QLabel>
#include <QPainter>
#include <QApplication>
#include <QStyle>

#include <QToolBar>

using namespace Peony;

StatusBar::StatusBar(FMWindowIface *window, QWidget *parent) : QStatusBar(parent)
{
    m_styled_toolbar = new QToolBar;

    setContentsMargins(0, 0, 0, 0);
    setStyleSheet("padding: 0;");
    setSizeGripEnabled(false);
    setMinimumHeight(30);

    m_window = window;
    m_label = new QLabel(this);
    m_label->setContentsMargins(0, 0, 0, 0);
    m_label->setWordWrap(false);
    m_label->setAlignment(Qt::AlignCenter);
    addWidget(m_label, 1);

    //setStyleSheet("align: center;");
    //showMessage(tr("Status Bar"));
}

StatusBar::~StatusBar()
{
    m_styled_toolbar->deleteLater();
}

void StatusBar::update()
{
    if (!m_window)
        return;

    auto selections = m_window->getCurrentSelectionFileInfos();
    if (!selections.isEmpty()) {
        QString directoriesString;
        int directoryCount = 0;
        QString filesString;
        int fileCount = 0;
        goffset size = 0;
        for (auto selection : selections) {
            if(selection->isDir()) {
                directoryCount++;
            } else if (!selection->isVolume()) {
                fileCount++;
                size += selection->size();
            }
        }
        //auto format_size = g_format_size(size);
        //Calculated by 1024 bytes
        auto format_size_GIB = g_format_size_full(size,G_FORMAT_SIZE_IEC_UNITS);
        QString format_size(format_size_GIB);
        //根据设计要求，按照1024字节对数据进行格式化（1GB = 1024MB），同时将GiB改为GB显示，以便于用户理解。参考windows显示样式。
        format_size.replace("iB", "B");

        if (selections.count() == 1) {
            if (directoryCount == 1)
                directoriesString = QString(", %1").arg(selections.first()->displayName());
            if (fileCount == 1)
                filesString = QString(", %1, %2").arg(selections.first()->displayName()).arg(format_size);
        } else if (directoryCount > 1 && (fileCount > 1)) {
            directoriesString = tr("; %1 folders").arg(directoryCount);
            filesString = tr("; %1 files, %2 total").arg(fileCount).arg(format_size);
        } else if (directoryCount > 1 && (fileCount > 1)) {
            directoriesString = tr("; %1 folder").arg(directoryCount);
            filesString = tr("; %1 file, %2").arg(fileCount).arg(format_size);
        } else if (fileCount == 0) {
            directoriesString = tr("; %1 folders").arg(directoryCount);
        } else {
            filesString = tr("; %1 files, %2 total").arg(fileCount).arg(format_size);
        }

        m_label->setText(tr("%1 selected").arg(selections.count()) + directoriesString + filesString);
        //showMessage(tr("%1 files selected ").arg(selections.count()));
        g_free(format_size_GIB);
    } else {
        m_label->setText(m_window->getCurrentUri());
        //showMessage(m_window->getCurrentUri());
    }
}

void StatusBar::update(const QString &message)
{
    m_label->setText(message);
}

void StatusBar::paintEvent(QPaintEvent *e)
{
    //I do not want status bar draw the inserted widget's
    //'border', so I use painter overwrite it.
    QStatusBar::paintEvent(e);
    QPainter p(this);
    auto rect = this->rect();
    //rect.adjust(0, 2, 0, 0);
    auto bg = m_styled_toolbar->palette().window().color();
    p.fillRect(rect, bg);
    auto base = m_styled_toolbar->palette().base().color();
    base.setAlpha(0);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    //p.fillRect(this->rect(), this->palette().base());
    p.fillRect(rect, base);
}
