/*
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: zhangjiaping <zhangjiaping@kylinos.cn>
 *
 */
#ifndef SHOWMORELABEL_H
#define SHOWMORELABEL_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QTimer>

namespace LingmoUISearch {
class ShowMoreLabel : public QWidget {
    Q_OBJECT
public:
    explicit ShowMoreLabel(QWidget *parent = nullptr);
    ~ShowMoreLabel() = default;
    void resetLabel();
    void setLabel();
    bool getExpanded();

protected:
    bool eventFilter(QObject *, QEvent *);

private:
    QHBoxLayout * m_layout = nullptr;
    QLabel * m_iconLabel = nullptr;
    QLabel * m_loadingIconLabel = nullptr;
    QTimer * m_timer = nullptr;
    bool m_isOpen = false;
    int m_currentState = 0;

    void initUi();

Q_SIGNALS:
    void showMoreClicked();
    void retractClicked();

public Q_SLOTS:
};
}

#endif // SHOWMORELABEL_H
