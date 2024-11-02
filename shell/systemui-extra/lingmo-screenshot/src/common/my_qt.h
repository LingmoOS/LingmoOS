/*
 *
 * Copyright:  2020 KylinSoft Co., Ltd.
 * Authors:
 *   wenjie bi <biwenjie@cs2c.com.cn>
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 */
#pragma once

#include <QDebug>

#include <QtGui>
#include <QButtonGroup>
#include <QLabel>
#include <QPainter>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QProgressDialog>
#include <QX11Info>
#include <QApplication>
#include <QLibraryInfo>
#include <QTranslator>
#include <QTextStream>
#include <QTimer>
#include <QDir>
#include <QFile>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextCodec>
#include <QGroupBox>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>
#include <QDesktopWidget>
#include <QGSettings/qgsettings.h>

#include <set>

#include "Enum.h"

struct ContainerData {
    QString name, avname;
    QStringList suffixes;
    QString filter;
    std::set<ssr::enum_video_codec> supported_video_codecs;
    std::set<ssr::enum_audio_codec> supported_audio_codecs;
    inline bool operator<(const ContainerData &other) const
    {
        return avname < other.avname;
    }
};
struct VideoCodecData {
    QString name, avname;
    inline bool operator<(const VideoCodecData &other) const
    {
        return avname < other.avname;
    }
};
struct AudioCodecData {
    QString name, avname;
    inline bool operator<(const AudioCodecData &other) const
    {
        return avname < other.avname;
    }
};
struct AudioKBitRate {
    QString rate;
    inline bool operator<(const AudioKBitRate &other) const
    {
        return rate < other.rate;
    }
};

inline void GroupVisible(std::initializer_list<QWidget *> widgets, bool visible)
{
    for (QWidget *w : widgets) {
        w->setVisible(visible);
    }
}
