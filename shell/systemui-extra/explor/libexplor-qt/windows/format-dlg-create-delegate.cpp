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
 * Authors: XiaoFeng Zhou <zhouxiaofeng@kylinos.cn>
 *
 */

#include "format-dlg-create-delegate.h"
#include "format_dialog.h"
#ifdef LINGMO_UDF_BURN
#include "ky-udf-format-dialog.h"
#include "volumeManager.h"
#include <libkyudfburn/disccontrol.h>
#else
#include "udfFormatDialog.h"
#include "disccontrol.h"
#endif

static FormatDlgCreateDelegate *global_instance = nullptr;

FormatDlgCreateDelegate *FormatDlgCreateDelegate::getInstance()
{
    if (!global_instance) {
        global_instance = new FormatDlgCreateDelegate;
    }
    return global_instance;
}

Format_Dialog *FormatDlgCreateDelegate::createUDiskDlg(const QString &uris, SideBarAbstractItem *item, QWidget *parent)
{
    if (m_udiskDlgMap.contains(uris)) {
        m_udiskDlgMap[uris]->raise();
        return m_udiskDlgMap[uris];
    }

    Format_Dialog *dlg = new Format_Dialog(uris, item);
    m_udiskDlgMap[uris] = dlg;
    return dlg;
}

void FormatDlgCreateDelegate::removeFromUdiskMap(QString &uris)
{
    m_udiskDlgMap.remove(uris);
}

#ifndef LINGMO_UDF_BURN
UdfFormatDialog *FormatDlgCreateDelegate::createUdfDlg(const QString &uris, DiscControl *discControl, QWidget *parent)
{
    if (m_udfDlgMap.contains(uris)) {
        m_udfDlgMap[uris]->raise();
        return m_udfDlgMap[uris];
    }

    UdfFormatDialog *dlg = new UdfFormatDialog(uris, discControl);
    m_udfDlgMap[uris] = dlg;
    return dlg;
}

void FormatDlgCreateDelegate::removeFromUdfMap(QString &uris)
{
    m_udfDlgMap.remove(uris);
}

#else
UdfBurn::UdfFormatDialogWrapper *FormatDlgCreateDelegate::createUdfDlgWrapper(const QString &uris, UdfBurn::DiscControl *discControl, QWidget *parent)
{
    if (m_udfDlgWrapperMap.contains(uris)) {
        m_udfDlgWrapperMap[uris]->raise();
        return m_udfDlgWrapperMap[uris];
    }

    UdfBurn::UdfFormatDialogWrapper *wrapper = new UdfBurn::UdfFormatDialogWrapper(uris, discControl);
    m_udfDlgWrapperMap[uris] = wrapper;
    return wrapper;
}

void FormatDlgCreateDelegate::removeFromUdfWrapperMap(QString &uris)
{
    m_udfDlgWrapperMap.remove(uris);
}

#endif

FormatDlgCreateDelegate::FormatDlgCreateDelegate(QObject *parent)
{

}

FormatDlgCreateDelegate::~FormatDlgCreateDelegate()
{

}
