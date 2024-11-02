/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: WenJie Xiang <xiangwenjie@kylinos.cn>
 *
 */

#ifndef EXTENSIONSMANAGERWIDGET_H
#define EXTENSIONSMANAGERWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QTableWidget>
#include <QVBoxLayout>
#include "plugin-iface.h"
#include "explor-core_global.h"

#define EXTENSIONS_SHOW_WIDTH   537
#define EXTENSIONS_SHOW_HEIGHT  860
#define DISABLED_EXTENSIONS         "disabledExtensions"

namespace Peony {

class PEONYCORESHARED_EXPORT ExtensionsManagerWidget : public QWidget
{
    Q_OBJECT
public:
    static ExtensionsManagerWidget *getInstance();
    void initUI();
    void initTableWidget();
    void initExtensionInfo();
    bool updateCheckBox(const QString &path);
    void addSeparator();

private:
    explicit ExtensionsManagerWidget(QWidget *parent = nullptr);
    ~ExtensionsManagerWidget();

private:
    QVBoxLayout *m_mainLayout = nullptr;
    QTableWidget *m_tableWidget = nullptr;
    QLabel *m_useLabel = nullptr;
    QPushButton *m_okBtn = nullptr;
    QPushButton *m_cancelBtn = nullptr;

    QMap<QString, PluginInterface*> m_pluginMap;
    QStringList m_disabledList;
};
}
#endif // EXTENSIONSMANAGERWIDGET_H
