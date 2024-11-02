/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KLEFTSIDEITEM_H
#define KLEFTSIDEITEM_H

#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <qgsettings.h>

class KLeftSideItem : public QPushButton
{
    Q_OBJECT
public:
    explicit KLeftSideItem(QWidget *parent = nullptr);
    KLeftSideItem(QString strText, QString strIcon, QWidget *parent = nullptr);
    virtual ~KLeftSideItem();

    void initUI();
    void setText(QString &strText);
    void setIcon(QString &strIcon);
    QString btnHoverColor(QString styleName, bool hoverFlag);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;  //事件过滤

private:
    void initStyleTheme();

private:
    QHBoxLayout *m_layoutMain = nullptr;
    QLabel *m_labelIcon = nullptr;
    QLabel *m_labelText = nullptr;

    QString m_strIcon;
    QString m_strText;
    QGSettings *m_styleSettings = nullptr;
    QString m_strThemeName = "";
    bool m_isNormal = true;
    bool m_isHover = false;
    QString hoverColor;
    QString clickColor;
};

#endif // KLEFTSIDEITEM_H
