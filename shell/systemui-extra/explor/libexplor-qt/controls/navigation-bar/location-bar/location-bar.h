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

#ifndef LOCATIONBAR_H
#define LOCATIONBAR_H

#include <QToolBar>
#include "explor-core_global.h"
#include <memory>

class QLineEdit;
class QHBoxLayout;
class QToolButton;
class QMenu;

namespace Peony {

class FileInfo;

class LocationBar : public QWidget
{
    Q_OBJECT
public:
    explicit LocationBar(QWidget *parent = nullptr);
    ~LocationBar() override;
    const QString getCurentUri() {
        return m_current_uri;
    }
    void setAnimationMode(bool isAnimation);

Q_SIGNALS:
    void groupChangedRequest(const QString &uri);
    void blankClicked();
    void aboutToSetRootUri();

public Q_SLOTS:
    void setRootUri(const QString &uri);
    void updateTrashIcon();
protected:
    void clearButtons();
    void addButton(const QString &uri, bool setIcon = false, bool setMenu = true);
    void updateButtons();

    void mousePressEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *event) override;

    void doLayout();

private:
    QString m_current_uri;
    QLineEdit *m_styled_edit;
    QHBoxLayout *m_layout;

    QMap<QString, QToolButton *> m_buttons;
    QToolButton *m_indicator;
    QMenu *m_indicator_menu;
    bool m_isAnimation = false;   //在动画过程中不会重新布局

    const int ELIDE_TEXT_LENGTH = 16;

    std::shared_ptr<FileInfo> m_current_info;
    QList<std::shared_ptr<FileInfo>> m_buttons_info;
    QList<std::shared_ptr<FileInfo>> m_querying_buttons_info;
    QHash<QString, QList<std::shared_ptr<FileInfo>>> m_infos_hash;

    bool m_is_classical = false;
};

}

#endif // LOCATIONBAR_H
