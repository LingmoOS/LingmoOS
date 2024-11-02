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
 * Authors: Yan Wei Yang <yangyanwei@kylinos.cn>
 *
 */

#ifndef TAGMANAGEMENT_H
#define TAGMANAGEMENT_H

#include <QMainWindow>
#include <QTabWidget>
#include <QWidget>
#include <QPushButton>
#include <QPainter>
#include <QColor>
#include <QEnterEvent>
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QProxyStyle>
#include "explor-core_global.h"

namespace Peony {
class PEONYCORESHARED_EXPORT TagManagement : public QMainWindow
{
    Q_OBJECT
public:
    static TagManagement *getInstance();
    explicit TagManagement(QWidget *parent = nullptr);

private:
    void init();
};

class FileLabelEdit : public QWidget
{
    Q_OBJECT
public:
    explicit FileLabelEdit(QColor color, QString name, QWidget *parent = nullptr);
    void startEdit();

Q_SIGNALS:
    void changeName(const QString &name);

public Q_SLOTS:
    void setColor(QColor color);
    void setName(const QString& name);

private:
    QPushButton *m_colorButton = nullptr;
    QLabel *m_text = nullptr;
    QLineEdit *m_edit = nullptr;

};

class LabelSettings : public QWidget
{
    Q_OBJECT
public:
    explicit LabelSettings(QWidget *parent = nullptr);
    void loadAllFileLabels();

public Q_SLOTS:
    void addFileLabel();
    void delFileLabel();

private:
    QTableWidget *m_fileLabel = nullptr;
};
}

class tabStyle : public QProxyStyle
{
public:
    void drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter,
                     const QWidget *widget) const;

    QSize sizeFromContents(QStyle::ContentsType ct, const QStyleOption *opt, const QSize &contentsSize,
                           const QWidget *w) const;
};

class tabWidgetStyle : public QProxyStyle
{
public:
    /**
     * QTabBar设置居中
     * @brief subElementRect
     * @param element
     * @return 返回QTabBar的矩形
     */
    QRect subElementRect(SubElement element,
                         const QStyleOption *option,
                         const QWidget *widget = nullptr) const;
};

#endif // TAGMANAGEMENT_H
