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

#ifndef FORMAT_DLG_CREATE_DELEGATE_H
#define FORMAT_DLG_CREATE_DELEGATE_H

#include <QObject>
#include <QMap>
#include <QDialog>
#include "explor-core_global.h"

class Format_Dialog;
namespace Peony {
    class SideBarAbstractItem;
}

#ifdef LINGMO_UDF_BURN
namespace UdfBurn {
    class UdfFormatDialog;
    class DiscControl;
    class UdfFormatDialogWrapper;
}
#else
class UdfFormatDialog;
class DiscControl;
#endif

class PEONYCORESHARED_EXPORT FormatDlgCreateDelegate : public QObject
{
    Q_OBJECT
public:
    static FormatDlgCreateDelegate *getInstance();
    Format_Dialog *createUDiskDlg(const QString &uris, Peony::SideBarAbstractItem *item, QWidget *parent = nullptr);
    void removeFromUdiskMap(QString &uris);
#ifndef LINGMO_UDF_BURN
    UdfFormatDialog *createUdfDlg(const QString &uris, DiscControl *discControl, QWidget *parent = nullptr);
    void removeFromUdfMap(QString &uris);
#else
    UdfBurn::UdfFormatDialogWrapper *createUdfDlgWrapper(const QString &uris, UdfBurn::DiscControl *discControl, QWidget *parent = nullptr);
    void removeFromUdfWrapperMap(QString &uris);
#endif
private:
    explicit FormatDlgCreateDelegate(QObject *parent = nullptr);
    ~FormatDlgCreateDelegate();

private:
    QMap<QString, Format_Dialog *> m_udiskDlgMap;
#ifndef LINGMO_UDF_BURN
    QMap<QString, UdfFormatDialog *> m_udfDlgMap;
#else
    QMap<QString, UdfBurn::UdfFormatDialogWrapper *> m_udfDlgWrapperMap;
#endif
};

#endif // FORMAT_DLG_CREATE_DELEGATE_H
