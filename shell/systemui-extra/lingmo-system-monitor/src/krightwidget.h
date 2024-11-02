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
#ifndef KRIGHTWIDGET_H
#define KRIGHTWIDGET_H

#include <QWidget>
#include <QToolButton>
#include <QMenu>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QPropertyAnimation>
#include <QLineEdit>
#include <QTimer>
#include <QGSettings>

#include "dialog/usmaboutdialog.h"
#include "kaboutdialog.h"
#include "ksearchlineedit.h"

using namespace kdk;

class KRightWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KRightWidget(QWidget *parent = nullptr);
    virtual ~KRightWidget();

    void initUI();
    void initConnections();
    void createAboutDialog();

    void addPanel(QWidget* pWidget, QString strName, QString strIcon, int nPanelIdx = -1);
    int currentIndex();
    void changeTitleWidget(bool isTablet);
    void updateMaxBtn(bool max);
    QLineEdit *getSearchLineEdit() {return m_searchEditNew;}

public slots:
    void onMinBtnClicked();
    void onMaxBtnClicked();
    void onCloseBtnClicked();
    void onSwichPanel(int nIndex);
    void onUpdateMaxBtnState();
    void animationFinishedSlot();
    void onRefreshSearchResult();
    void handleSearchTextChanged();

signals:
    void maximizeWindow();
    void minimizeWindow();
    void canelSearchEditFocus();
    void searchSignal(QString searchContent);

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *event);
    bool eventFilter(QObject *, QEvent *event) override;

private:

private:
    QWidget* m_titleWidget = nullptr;
    QToolButton *m_btnOption = nullptr;
    QToolButton *m_btnMin = nullptr;
    QToolButton *m_btnMax = nullptr;
    QToolButton *m_btnClose = nullptr;
    QMenu       *m_mainMenu = nullptr;
    QStackedWidget *m_stackedWidget = nullptr;

    USMAboutDialog *m_aboutDlg = nullptr;

    QVBoxLayout *m_mainLayout = nullptr;
    QHBoxLayout *m_titleLayout = nullptr;
    QHBoxLayout *m_searchLayout = nullptr;

    // search edit
    KSearchLineEdit *m_searchEditNew = nullptr;
    QString m_searchTextCache;
    QTimer *m_searchTimer = nullptr;
    bool m_isSearching = false;

    QGSettings *m_styleSettings = nullptr;

public:
    static QString searchText;
};

#endif // KRIGHTWIDGET_H
