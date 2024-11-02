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
 * Authors: Wenfei He <hewenfei@kylinos.cn>
 *
 */

#ifndef STABLEMARKPROPERTIESPAGE_H
#define STABLEMARKPROPERTIESPAGE_H
#include "properties-window-tab-iface.h"
#include "file-label-model.h"

#include <QTableWidget>
#include <QVBoxLayout>
#include <QLabel>

namespace Peony {

class MarkPropertiesPage : public PropertiesWindowTabIface
{
public:
    /*!
     * \brief 保存设置
     */
    void saveAllChange() override;
    /*!
     * \brief 初始化表格相关设置
     */
    void initTableWidget();

    /*!
     * \brief 初始化表格中的数据
     */
    void initTableData();

    void changeLabel(int labelId, bool checked);

public:
    MarkPropertiesPage(const QString &uri, QWidget *parent = nullptr);
    ~MarkPropertiesPage();

private:
    QString m_uri;
    QVBoxLayout  *m_layout      = nullptr;
    QTableWidget *m_tableWidget = nullptr;
    //文件标记模型
    FileLabelModel *m_fileLabelModel = nullptr;
    //当前文件的全部标签
    QList<int> m_thisFileLabelIds;

    /*!
     * \brief 把color转换为16进制字符串
     * \param color
     * \return
     */
    QString convertRGB16HexStr(const QColor color);

    void updateTableLabelShow(QLabel* label, const QString &str);
};

}

#endif //MARKPROPERTIESPAGE_H
