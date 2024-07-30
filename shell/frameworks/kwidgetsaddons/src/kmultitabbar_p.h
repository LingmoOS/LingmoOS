/*
    SPDX-FileCopyrightText: 2003 Joseph Wenninger <jowenn@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef K_MULTI_TAB_BAR_P_H
#define K_MULTI_TAB_BAR_P_H
#include <QScrollArea>
#include <QVBoxLayout>
#include <kmultitabbar.h>

class KMultiTabBarInternal : public QFrame
{
    Q_OBJECT
public:
    KMultiTabBarInternal(QWidget *parent, KMultiTabBar::KMultiTabBarPosition pos);
    ~KMultiTabBarInternal() override;
    int appendTab(const QIcon &, int = -1, const QString & = QString());
    int appendTab(const QPixmap &, int = -1, const QString & = QString());
    KMultiTabBarTab *tab(int) const;
    void removeTab(int);
    void setPosition(enum KMultiTabBar::KMultiTabBarPosition pos);
    void setStyle(enum KMultiTabBar::KMultiTabBarStyle style);
    void showActiveTabTexts(bool show);
    QList<KMultiTabBarTab *> *tabs()
    {
        return &m_tabs;
    }

private:
    friend class KMultiTabBar;
    QBoxLayout *mainLayout;
    QList<KMultiTabBarTab *> m_tabs;
    enum KMultiTabBar::KMultiTabBarPosition m_position;
    enum KMultiTabBar::KMultiTabBarStyle m_style;

protected:
    /**
     * [contentsM|m]ousePressEvent are reimplemented from QScrollView
     * in order to ignore all mouseEvents on the viewport, so that the
     * parent can handle them.
     */
    virtual void contentsMousePressEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *) override;
};

#endif
