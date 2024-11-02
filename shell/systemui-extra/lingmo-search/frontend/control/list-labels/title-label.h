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
#ifndef TITLELABEL_H
#define TITLELABEL_H

#include <QLabel>
#include "show-more-label.h"
#include <QHBoxLayout>

namespace LingmoUISearch {
class TitleLabel : public QLabel {
    Q_OBJECT
public:
    TitleLabel(QWidget * parent = nullptr);
    ~TitleLabel() = default;

    void setShowMoreLableVisible();

protected:
    void paintEvent(QPaintEvent *);

private:
    void initUi();
    void initConnections();

    QHBoxLayout * m_titleLyt = nullptr;
    ShowMoreLabel * m_showMoreLabel = nullptr;
    int m_radius = 6;

public Q_SLOTS:
    void onListLengthChanged(const int &);
    void labelReset();

Q_SIGNALS:
    void startSearch(const QString &);
    void stopSearch();
    void showMoreClicked();
    void retractClicked();
};
}

#endif // TITLELABEL_H
