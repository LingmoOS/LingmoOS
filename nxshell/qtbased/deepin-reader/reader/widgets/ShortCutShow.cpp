// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ShortCutShow.h"
#include "Application.h"
#include "Global.h"
#include "DocSheet.h"

#include <DGuiApplicationHelper>

#include <QProcess>
#include <QDesktopWidget>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

ShortCutShow::ShortCutShow(QObject *parent)
    : QObject(parent)
{

}

void ShortCutShow::setSheet(DocSheet *sheet)
{
    if (nullptr == sheet)
        initPDF();
    else if (Dr::DJVU == sheet->fileType())
        initDJVU();
    else
        initPDF();
}

void ShortCutShow::show()
{
    QRect rect = qApp->desktop()->geometry();
    QPoint pos(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);
#if (DTK_VERSION >= DTK_VERSION_CHECK(5, 5, 2, 0))
    // 获取当前焦点位置（光标所在屏幕中心）
    QScreen *screen = nullptr;
    if (DGuiApplicationHelper::isTabletEnvironment()) {
        // bug 88079 避免屏幕旋转弹出位置错误
        screen = qApp->primaryScreen();
    } else {
        screen = QGuiApplication::screenAt(QCursor::pos());
    }

    if (screen) {
        pos = screen->geometry().center();
    }
#endif
    QJsonObject shortcutObj;
    QJsonArray jsonGroups;
    QString strvalue;
    QList<ShortCutType> listType{ShortCutType::Settings, ShortCutType::File, ShortCutType::Display, ShortCutType::Tools, ShortCutType::Edit};
    for(ShortCutType type : listType)
    {
        QJsonObject group;
        group.insert("groupName", tr("Settings"));
        QJsonArray items;

        for (const auto &d : m_shortcutMap[type]) {
            QJsonObject jsonItem;
            jsonItem.insert("name", d.second);
            jsonItem.insert("value", d.first);
            items.append(jsonItem);
        }

        group.insert("groupItems", items);
        jsonGroups.append(group);
    }

    shortcutObj.insert("shortcut", jsonGroups);

    QJsonDocument doc(shortcutObj);

    QStringList shortcutString;
    QString param1 = "-j=" + QString(doc.toJson().data());
    QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());

    shortcutString << param1 << param2;

    QProcess shortcutViewProcess;
    shortcutViewProcess.startDetached("deepin-shortcut-viewer", shortcutString);
}

void ShortCutShow::initDJVU()
{
    initPDF();
    //remove Dr::key_ctrl_f
    m_shortcutMap[ShortCutType::Tools].removeKey(Dr::key_ctrl_f);
}

void ShortCutShow::initPDF()
{
    m_shortcutMap = {
        //Settings
        {ShortCutType::Settings, {
             {Dr::key_f1              , tr("Help")               },
             {Dr::key_ctrl_shift_slash, tr("Display shortcuts")  }}},

        //File
        {ShortCutType::File, {
             {Dr::key_ctrl_o          , tr("Open")               },
             {Dr::key_ctrl_shift_s    , tr("Save as")            },
             {Dr::key_ctrl_p          , tr("Print")              },
             {Dr::key_ctrl_s          , tr("Save")               }}},

        //Display
        {ShortCutType::Display, {
             {Dr::key_ctrl_m          , tr("Thumbnails")         },
             {Dr::key_ctrl_1          , tr("1:1 size")           },
             {Dr::key_ctrl_2          , tr("Fit height")         },
             {Dr::key_ctrl_3          , tr("Fit width")          },
             {Dr::key_ctrl_r          , tr("Rotate left")        },
             {Dr::key_ctrl_shift_r    , tr("Rotate right")       },
             {Dr::key_alt_harger      , tr("Zoom in")            },
             {Dr::key_ctrl_smaller    , tr("Zoom out")           },
             {Dr::key_pgUp            , tr("Page up")            },
             {Dr::key_pgDown          , tr("Page down")          },
             {Dr::key_ctrl_home       , tr("Move to the beginning") },
             {Dr::key_ctrl_end        , tr("Move to the end")  },
             {Dr::key_esc             , tr("Exit")               }}},

        //Tools
        {ShortCutType::Tools, {
             {Dr::key_alt_1           , tr("Select text")        },
             {Dr::key_alt_2           , tr("Hand tool")          },
             {Dr::key_ctrl_d          , tr("Add bookmark")       },
             {Dr::key_alt_a           , tr("Add annotation")     },
             {Dr::key_alt_h           , tr("Highlight")          },
             {Dr::key_alt_z           , tr("Magnifier")          },
             {Dr::key_ctrl_f          , tr("Search")             },
             {Dr::key_f5              , tr("Slide show")         },
             {Dr::key_f11             , tr("Fullscreen")         }}},

        //Edit
        {ShortCutType::Edit, {
             {Dr::key_ctrl_c          , tr("Copy")               },
             {Dr::key_ctrl_x          , tr("Cut")                },
             {Dr::key_ctrl_v          , tr("Paste")              },
             {Dr::key_delete          , tr("Delete")             },
             {Dr::key_ctrl_z          , tr("Undo")               },
             {Dr::key_ctrl_a          , tr("Select all")         }}},
    };
}
