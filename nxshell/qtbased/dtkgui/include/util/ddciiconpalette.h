// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DDCIICONPALETTE_H
#define DDCIICONPALETTE_H

#include <dtkgui_global.h>
#include <qobjectdefs.h>

#include <QColor>
#include <QVector>

QT_BEGIN_NAMESPACE
class QPalette;
QT_END_NAMESPACE

DGUI_BEGIN_NAMESPACE

class DDciIconPalette
{
    Q_GADGET
    Q_PROPERTY(QColor foreground READ foreground WRITE setForeground FINAL)
    Q_PROPERTY(QColor background READ background WRITE setBackground FINAL)
    Q_PROPERTY(QColor highlight READ highlight WRITE setHighlight FINAL)
    Q_PROPERTY(QColor highlightForeground READ highlightForeground WRITE setHighlightForeground FINAL)

public:
    enum PaletteRole {
        NoPalette = -1,
        Foreground = 0,
        Background = 1,
        HighlightForeground = 2,
        Highlight = 3,
        PaletteCount
    };

    DDciIconPalette(QColor foreground = QColor::Invalid, QColor background = QColor::Invalid,
                    QColor highlight = QColor::Invalid, QColor highlightForeground = QColor::Invalid);
    bool operator==(const DDciIconPalette &other) const;
    bool operator!=(const DDciIconPalette &other) const;

    QColor foreground() const;
    void setForeground(const QColor &foreground);

    QColor background() const;
    void setBackground(const QColor &background);

    QColor highlightForeground() const;
    void setHighlightForeground(const QColor &highlightForeground);

    QColor highlight() const;
    void setHighlight(const QColor &highlight);

    static QString convertToString(const DDciIconPalette &palette);
    static DDciIconPalette convertFromString(const QString &data);
    static DDciIconPalette fromQPalette(const QPalette &pa);
private:
    QVector<QColor> colors;
};

DGUI_END_NAMESPACE

QT_BEGIN_NAMESPACE
#ifndef QT_NO_DEBUG_STREAM
Q_GUI_EXPORT QDebug operator<<(QDebug, const DTK_GUI_NAMESPACE::DDciIconPalette &);
#endif
QT_END_NAMESPACE
#endif // DDCIICONPALETTE_H
