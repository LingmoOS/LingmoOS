/*
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef COLOREDITOR_H
#define COLOREDITOR_H

#include <QColor>
#include <QObject>

class ColorEditor : public QObject
{
    Q_OBJECT
    // Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)

    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor highlightColor READ highlightColor WRITE setHighlightColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor linkColor READ linkColor WRITE setLinkColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor visitedLinkColor READ visitedLinkColor WRITE setVisitedLinkColor NOTIFY colorsChanged)

    Q_PROPERTY(QColor buttonTextColor READ buttonTextColor WRITE setButtonTextColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor buttonBackgroundColor READ buttonBackgroundColor WRITE setButtonBackgroundColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor buttonHoverColor READ buttonHoverColor WRITE setButtonHoverColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor buttonFocusColor READ buttonFocusColor WRITE setButtonFocusColor NOTIFY colorsChanged)

    Q_PROPERTY(QColor viewTextColor READ viewTextColor WRITE setViewTextColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor viewBackgroundColor READ viewBackgroundColor WRITE setViewBackgroundColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor viewHoverColor READ viewHoverColor WRITE setViewHoverColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor viewFocusColor READ viewFocusColor WRITE setViewFocusColor NOTIFY colorsChanged)

    Q_PROPERTY(QColor complementaryTextColor READ complementaryTextColor WRITE setComplementaryTextColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor complementaryBackgroundColor READ complementaryBackgroundColor WRITE setComplementaryBackgroundColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor complementaryHoverColor READ complementaryHoverColor WRITE setComplementaryHoverColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor complementaryFocusColor READ complementaryFocusColor WRITE setComplementaryFocusColor NOTIFY colorsChanged)
public:
    explicit ColorEditor(QObject *parent = nullptr);
    ~ColorEditor() override;

    QString theme() const;
    void setTheme(const QString &theme);

    QColor textColor() const;
    void setTextColor(const QColor &color);

    QColor highlightColor() const;
    void setHighlightColor(const QColor &color);

    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &color);

    QColor buttonTextColor() const;
    void setButtonTextColor(const QColor &color);

    QColor buttonBackgroundColor() const;
    void setButtonBackgroundColor(const QColor &color);

    QColor linkColor() const;
    void setLinkColor(const QColor &color);

    QColor visitedLinkColor() const;
    void setVisitedLinkColor(const QColor &color);

    QColor buttonHoverColor() const;
    void setButtonHoverColor(const QColor &color);

    QColor buttonFocusColor() const;
    void setButtonFocusColor(const QColor &color);

    QColor viewTextColor() const;
    void setViewTextColor(const QColor &color);

    QColor viewBackgroundColor() const;
    void setViewBackgroundColor(const QColor &color);

    QColor viewHoverColor() const;
    void setViewHoverColor(const QColor &color);

    QColor viewFocusColor() const;
    void setViewFocusColor(const QColor &color);

    QColor complementaryTextColor() const;
    void setComplementaryTextColor(const QColor &color);

    QColor complementaryBackgroundColor() const;
    void setComplementaryBackgroundColor(const QColor &color);

    QColor complementaryHoverColor() const;
    void setComplementaryHoverColor(const QColor &color);

    QColor complementaryFocusColor() const;
    void setComplementaryFocusColor(const QColor &color);

    void load();
    Q_INVOKABLE void save();

Q_SIGNALS:
    void colorsChanged();
    void themeChanged();

private:
    QString m_theme;

    QColor m_textColor;
    QColor m_highlightColor;
    QColor m_backgroundColor;
    QColor m_linkColor;
    QColor m_visitedLinkColor;

    QColor m_buttonTextColor;
    QColor m_buttonBackgroundColor;
    QColor m_buttonHoverColor;
    QColor m_buttonFocusColor;

    QColor m_viewTextColor;
    QColor m_viewBackgroundColor;
    QColor m_viewHoverColor;
    QColor m_viewFocusColor;

    QColor m_complementaryTextColor;
    QColor m_complementaryBackgroundColor;
    QColor m_complementaryHoverColor;
    QColor m_complementaryFocusColor;
};

#endif // COLOREDITOR_H
