// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SLIDESHOWPANEL_H
#define SLIDESHOWPANEL_H

#include "imageanimation.h"

#include <DMenu>
#include <DIconButton>
#include <DLabel>
#include <DFloatingWidget>

#include <QHBoxLayout>
#include <QShortcut>
#include <QObject>

DWIDGET_USE_NAMESPACE

// For view images
struct ViewInfo {
    QWidget *lastPanel {nullptr};  // For back to the last panel
    int viewMainWindowID = 0;
    bool fullScreen = false;
    bool slideShow = false;
    QString album = QString();
    QString path;                       // Specific current open one
    QStringList paths = QStringList();  // Limit the view range
};

class SlideShowBottomBar : public DFloatingWidget
{
    Q_OBJECT
public:
    explicit SlideShowBottomBar(QWidget *parent = nullptr);
public:
    DIconButton *m_preButton;
    DIconButton *m_nextButton;
    DIconButton *m_playpauseButton;
    DIconButton *m_cancelButton;
    bool isStop = false;

public slots:
    void onPreButtonClicked();
    void onPlaypauseButtonClicked();
    void onUpdatePauseButton();
    void onInitSlideShowButton();
    void onNextButtonClicked();
    void onCancelButtonClicked();

signals:
    void showPrevious();
    void showPause();
    void showContinue();
    void showNext();
    void showCancel();
};

class LibSlideShowPanel : public QWidget
{
    Q_OBJECT
public:
    enum MenuItemId {
        IdStopslideshow,
        IdPlayOrPause,
        IdPlay,
        IdPause
    };
    explicit LibSlideShowPanel(QWidget *parent = nullptr);
    void initConnections();
    void initMenu();
    void appendAction(int id, const QString &text, const QString &shortcut);
    void backToLastPanel();
    void showNormal();
    void showFullScreen();

    SlideShowBottomBar *slideshowbottombar;
signals:
    void hideSlidePanel(const QString &path = "");

public slots:
    void startSlideShow(const ViewInfo &vinfo);
    void onMenuItemClicked(QAction *action);
//    void onThemeChanged(ViewerThemeManager::AppTheme dark);
    void onSingleAnimationEnd();
    void onESCKeyStopSlide();
    void onShowPause();
    void onShowContinue();
    void onShowPrevious();
    void onShowNext();
    void onCustomContextMenuRequested();

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
private:
    LibImageAnimation *m_animation;
    DMenu *m_menu;
    QShortcut *m_sEsc;
    ViewInfo m_vinfo;
    QColor m_bgColor;
    bool m_isMaximized = false;
    int m_hideCursorTid;
    bool m_isf5move = false;
};

#endif // SLIDESHOWPANEL_H
