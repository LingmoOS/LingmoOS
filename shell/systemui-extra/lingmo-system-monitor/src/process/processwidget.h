#ifndef PROCESSWIDGET_H
#define PROCESSWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGSettings>
#include <QSettings>
#include <QButtonGroup>

#include "../controls/kgroupbutton.h"
#include "processtableview.h"
#include <lingmosdk/applications/ktabbar.h>

class ProcessWidget : public QWidget
{
    Q_OBJECT
public:
    enum {
      PROCESSTYPE_ACTIVE,
      PROCESSTYPE_BELONGME,
      PROCESSTYPE_ALL,
      PROCESSTYPE_APPLICATION
    };
    explicit ProcessWidget(QSettings *proSettings = nullptr, QWidget *parent = nullptr);
    virtual ~ProcessWidget();

    virtual void onWndClose();

public slots:
    void onSwitchProcType(int nType);

signals:
    void changeProcessItemDialog(int itemIndex);
    void searchSignal(QString);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void initUI();
    void initConnections();
    void initThemeMode();

private:
    QVBoxLayout *m_mainLayout = nullptr;
    QHBoxLayout *m_topBtnsLayout = nullptr;
    QVBoxLayout *m_procTableLayout = nullptr;

    kdk::KTabBar *processTabBar = nullptr;
    int m_nWhichNum = 0;

    ProcessTableView *m_procTabView = nullptr;
    QGSettings *m_ifSettings = nullptr;
    QSettings *m_proSettings = nullptr;
};

#endif // PROCESSWIDGET_H
