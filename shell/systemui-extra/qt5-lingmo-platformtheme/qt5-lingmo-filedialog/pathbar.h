/*
 * KWin Style LINGMO
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

#include "debug.h"

#ifndef PATHBAR_H
#define PATHBAR_H
#include <QLineEdit>
#include <QResizeEvent>
#include <explorer-qt/controls/navigation-bar/advanced-location-bar.h>

namespace LINGMOFileDialog {
class FileDialogPathBar : public QWidget//public QLineEdit//
{
    Q_OBJECT
public:
    explicit FileDialogPathBar(QWidget *parent = nullptr);
    ~FileDialogPathBar();
    void resizeEvent(QResizeEvent *e);
    Peony::AdvancedLocationBar *getPathBar();

Q_SIGNALS:
    void goToUriRequest(const QString &uri, bool addToHistory = true);
    void updateWindowLocationRequest(const QString &uri, bool addHistory, bool forceUpdate = false);
    void refreshRequest();
    void searchRequest(const QString &path, const QString &key);

public Q_SLOTS:
    void updateTableModel(bool isTable);
    void updatePath(const QString &uri);
private:
    Peony::AdvancedLocationBar *m_pathBar = nullptr;
};
}
#endif // PATHBAR_H
