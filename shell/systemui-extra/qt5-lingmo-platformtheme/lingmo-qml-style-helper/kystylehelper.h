/*
 * Qt5-LINGMO's Library
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
 * Authors: Yan Wang <wangyan@kylinos.cn>
 *
 */

#ifndef KYSTYLEHELPER_H
#define KYSTYLEHELPER_H

#include <QApplication>
#include <QQuickItem>
#include <QPalette>
#include <QColor>

class KyStyleHelper : public QQuickItem
{
    Q_OBJECT
    Q_DISABLE_COPY(KyStyleHelper)

    Q_PROPERTY(QPalette palette READ palette NOTIFY paletteChanged)
    Q_PROPERTY(QFont font READ font NOTIFY fontChanged)

    Q_PROPERTY(QColor windowtextcolorrole READ windowtextcolor NOTIFY qcolorChanged)
    Q_PROPERTY(QColor buttoncolorrole READ buttoncolor NOTIFY qcolorChanged)
    Q_PROPERTY(QColor lightcolorrole READ lightcolor NOTIFY qcolorChanged)
    Q_PROPERTY(QColor midlightcolorrole READ midlightcolor NOTIFY qcolorChanged)
    Q_PROPERTY(QColor darkcolorrole READ darkcolor NOTIFY qcolorChanged)
    Q_PROPERTY(QColor midcolorrole READ midcolor NOTIFY qcolorChanged)
    Q_PROPERTY(QColor textcolorrole READ textcolor NOTIFY qcolorChanged)
    Q_PROPERTY(QColor brighttextcolorrole READ brighttextcolor NOTIFY qcolorChanged)
    Q_PROPERTY(QColor buttontextcolorrole READ buttontextcolor NOTIFY qcolorChanged)
    Q_PROPERTY(QColor basecolorrole READ basecolor NOTIFY qcolorChanged)
    Q_PROPERTY(QColor windowcolorrole READ windowcolor NOTIFY qcolorChanged)
    Q_PROPERTY(QColor shadowcolorrole READ shadowcolor NOTIFY qcolorChanged)
    Q_PROPERTY(QColor highlightcolorrole READ highlightcolor NOTIFY qcolorChanged)
    Q_PROPERTY(QColor highlightedtextcolorrole READ highlightedtextcolor NOTIFY qcolorChanged)
    Q_PROPERTY(QColor linkcolorrole READ linkcolor NOTIFY qcolorChanged)
    Q_PROPERTY(QColor linkvisitedcolorrole READ linkvisitedcolor NOTIFY qcolorChanged)
    Q_PROPERTY(QColor alternatebasecolorrole READ alternatebasecolor NOTIFY qcolorChanged)
    Q_PROPERTY(QColor tooltipbasecolorrole READ tooltipbasecolor NOTIFY qcolorChanged)
    Q_PROPERTY(QColor tooltiptextcolorrole READ tooltiptextcolor NOTIFY qcolorChanged)

    Q_PROPERTY( QString buttonType READ buttonType WRITE setbuttonType NOTIFY buttonTypeChanged)

public:
    explicit KyStyleHelper(QQuickItem *parent = nullptr);
    ~KyStyleHelper() override;

    static KyStyleHelper* qmlAttachedProperties(QObject* parent);

    QString buttonType() const { return m_buttonType;}
    void setbuttonType(QString buttonType) {
            m_buttonType = buttonType ;
            emit buttonTypeChanged();
    }

     /* Get palette */
    QPalette palette() {
        return qApp->palette();
    }

    /* Get font */
   QFont font() {
       return qApp->font();
   }

    /* Get different type of color */
   QColor windowtextcolor() {
       return qApp->palette().windowText().color();
   }
   QColor buttoncolor() {
       return qApp->palette().button().color();
   }
   QColor lightcolor() {
       return qApp->palette().light().color();
   }
   QColor midlightcolor() {
       return qApp->palette().midlight().color();
   }
   QColor darkcolor() {
       return qApp->palette().dark().color();
   }
   QColor midcolor() {
       return qApp->palette().mid().color();
   }
   QColor textcolor() {
       return qApp->palette().text().color();
   }
   QColor brighttextcolor() {
       return qApp->palette().brightText().color();
   }
   QColor buttontextcolor() {
       return qApp->palette().buttonText().color();
   }
   QColor basecolor() {
       return qApp->palette().base().color();
   }
   QColor windowcolor() {
       return qApp->palette().window().color();
   }
   QColor shadowcolor() {
       return qApp->palette().shadow().color();
   }
   QColor highlightcolor() {
       return qApp->palette().highlight().color();
   }
   QColor highlightedtextcolor() {
       return qApp->palette().highlightedText().color();
   }
   QColor linkcolor() {
       return qApp->palette().link().color();
   }
   QColor linkvisitedcolor() {
       return qApp->palette().linkVisited().color();
   }
   QColor alternatebasecolor() {
       return qApp->palette().alternateBase().color();
   }
   QColor tooltipbasecolor() {
       return qApp->palette().toolTipBase().color();
   }
   QColor tooltiptextcolor() {
       return qApp->palette().toolTipText().color();
   }


signals:
    void paletteChanged();
    void fontChanged();
    void qcolorChanged();
    void buttonTypeChanged();

 protected:
    QString m_buttonType;
};

QML_DECLARE_TYPEINFO(KyStyleHelper, QML_HAS_ATTACHED_PROPERTIES)
#endif // KYSTYLEHELPER_H
