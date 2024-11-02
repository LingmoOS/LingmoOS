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
#ifndef SHORTCUTWIDGET_H
#define SHORTCUTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QPaintEvent>
#include <QPushButton>
#include <QFont>
#include "src/tools/capturecontext.h"
#include "xatomhelper.h"
class ShortCutWidget : public QWidget
{
    Q_OBJECT

public:
    ShortCutWidget(QWidget *parent = nullptr);
    ~ShortCutWidget();

protected:
    void paintEvent(QPaintEvent *e);

private slots:
    void setFont();

private:
// void initInfoTable();
    QLabel *m_titleIcon_label, *m_titleName_label;
    QPushButton *m_min_btn, *m_exit_btn;
    void initLabel();
    QList<QLabel *> labels;
    static QVector<const char *> m_keys;
    static QVector<const char *> m_description;
    QLabel *appIcon;
    QLabel *appName;
    QLabel *tableName;
    QIcon *windowIcon;
    QLabel *windowTitle;
    CaptureContext context;
    QGSettings *m_styleSettings;
};
#endif // WIDGET_H
