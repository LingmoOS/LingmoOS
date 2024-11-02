#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFrame>
#include <QPushButton>
#include <QLabel>
#include <QToolButton>
#include <QMenu>
#include <QGSettings>
#include <QSettings>

#include "kleftwidget.h"
#include "krightwidget.h"
#include "process/processwidget.h"
#include "filesystem/filesystemwidget.h"
#include "service/servicewidget.h"
#include "windowmanager/windowmanager.h"
#include "sysresource/cpudetailswidget.h"
#include "sysresource/memdetailswidget.h"
#include "sysresource/netdetailswidget.h"
#include "sysresource/basedetailviewwidget.h"

#include "trayicon.h"

class MainWindow : public QFrame
{
    Q_OBJECT
public:
    enum {
        FUNCTION_MODULE_PROCESS,
        FUNCTION_MODULE_SERVICE,
        FUNCTION_MODULE_FILESYSTEM,
        FUNCTION_MODULE_COUNT
    };
    explicit MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow();
    void windowShow();

public slots:
    void handleMessage(const QString &);
    void onSwitchPage(int nIndex);
    void onMinimizeWindow();
    void onMaximizeWindow();
    void switchPage(int nIndex);
    void tabletmodeDbusSlot(bool isTabletmode);

signals:

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE; //键盘松开事件
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *watched, QEvent *event);

private:
    void initUI();
    void initConnections();
    void initOpacityGSettings();

    void initLeftSideBar();
    void initRightPanel();
    void getOsRelease();
    bool loadSettings();
    void saveSettings();
    bool dblOnEdge(QMouseEvent *event);
    void moveCenter();

private:
    KLeftWidget *m_leftWidget = nullptr;
    KRightWidget *m_rightWidget = nullptr;
    QPoint dragPosition;
    bool mousePressed = false;
    // layout
    QHBoxLayout *m_mainLayout = nullptr;
    // QGSettings
    QGSettings *m_gsTransOpacity = nullptr;
    qreal m_curTransOpacity = 1;
    QGSettings *m_ifSettings = nullptr;
    bool m_isSurportService = false;
    int m_idxService = -1;

    QSettings *m_proSettings = nullptr;
    QSize m_lastWndSize;
    bool m_bIsWndMax;
    QString m_osVersion = "";

    ProcessWidget *m_procWidget = nullptr;
    FileSystemWidget *m_fsWidget = nullptr;
    ServiceWidget *m_svcWidget = nullptr;
    CpuDetailsWidget *mCpuDetailsWidget = nullptr;
    MemDetailsWidget *mMemDetailsWidget = nullptr;
    NetDetailsWidget *mNetDetailsWidget = nullptr;

    QList<WindowId> m_listWinIds;
    int mFirstKey = 0;
};

#endif // MAINWINDOW_H
