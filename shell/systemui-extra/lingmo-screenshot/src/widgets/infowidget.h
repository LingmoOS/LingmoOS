/*
 *
 * Copyright: 2020 KylinSoft Co., Ltd.
 * Authors:
 *   huanhuan zhang <zhanghuanhuan@kylinos.cn>
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
 */
#ifndef INFOWIDGET_H
#define INFOWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QFont>
#include <QPaintEvent>

class infoWidget : public QWidget
{
    Q_OBJECT

public:
    infoWidget(QWidget *parent = nullptr);
    ~infoWidget();

protected:
    void paintEvent(QPaintEvent *event);
private:
    void listenToGsettings();
    QPushButton *m_exitButton;
    QFont font;
    // QVBoxLayout *m_layout;
    QLabel *m_appIcon;
    QLabel *m_appName;
    QLabel *m_appVersion;
    QLabel *m_Descript;
    QLabel *m_EmailInfo;
};
#endif // WIDGET_H
