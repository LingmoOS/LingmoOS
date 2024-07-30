/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2015 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOLLAPSIBLEGROUPBOX_H
#define KCOLLAPSIBLEGROUPBOX_H

#include <QWidget>
#include <kwidgetsaddons_export.h>
#include <memory>

/**
 * @class KCollapsibleGroupBox kcollapsiblegroupbox.h KCollapsibleGroupBox
 *
 * A groupbox featuring a clickable header and arrow indicator that can be
 * expanded and collapsed to reveal the contents.
 *
 * When expanded, the widget will resize to fit the sizeHint of child items.
 *
 * @since 5.16
 */
class KWIDGETSADDONS_EXPORT KCollapsibleGroupBox : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(bool expanded READ isExpanded WRITE setExpanded NOTIFY expandedChanged)

public:
    explicit KCollapsibleGroupBox(QWidget *parent = nullptr);
    ~KCollapsibleGroupBox() override;

    /**
     * Set the title that will be permanently shown at the top of the collapsing box
     * Mnemonics are supported
     */
    void setTitle(const QString &title);

    /**
     * The title
     */
    QString title() const;

    /**
     * Set whether contents are shown
     *
     * The default is false until the user clicks
     */
    void setExpanded(bool expanded);

    /**
     * Whether contents are shown
     * During animations, this will reflect the target state at the end of the animation
     */
    bool isExpanded() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public Q_SLOTS:
    /**
     * Expands if collapsed and vice versa
     */
    void toggle();

    /**
     * Equivalent to setExpanded(true)
     */
    void expand();

    /**
     * Equivalent to setExpanded(false)
     */
    void collapse();

Q_SIGNALS:
    /**
     * Emitted when the title is changed
     */
    void titleChanged();

    /**
     * Emitted when the widget expands or collapsed
     */
    void expandedChanged();

protected:
    void paintEvent(QPaintEvent *) override;

    bool event(QEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void leaveEvent(QEvent *) override;
    void keyPressEvent(QKeyEvent *) override;
    void resizeEvent(QResizeEvent *) override;

private Q_SLOTS:
    KWIDGETSADDONS_NO_EXPORT void overrideFocusPolicyOf(QWidget *widget);

private:
    std::unique_ptr<class KCollapsibleGroupBoxPrivate> const d;

    Q_DISABLE_COPY(KCollapsibleGroupBox)
};

#endif
