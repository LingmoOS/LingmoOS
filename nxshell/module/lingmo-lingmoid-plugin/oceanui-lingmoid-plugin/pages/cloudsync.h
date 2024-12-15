// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CLOUDSYNC_H
#define CLOUDSYNC_H


#include "interface/namespace.h"
#include <QWidget>
#include <dtkwidget_global.h>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <DLabel>
#include <widgets/switchwidget.h>
#include "operation/syncmodel.h"
#include "operation/syncworker.h"
#include "userdialog.h"
#include "syncitemwidget.h"

DWIDGET_BEGIN_NAMESPACE
class DListView;
class DLabel;
class DLineEdit;
class DToolButton;
class DGroupBox;
class DFrame;
class DTipLabel;
class DDialog;
class DToolTip;
class DHorizontalLine;
class DCommandLinkButton;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class WarnLabel:public DLabel
{
public:
    explicit WarnLabel(const QString &text, QWidget *parent = nullptr);

    void SetTipText(const QString &tip);
protected:
    void enterEvent(QEvent *event) override;

    void leaveEvent(QEvent *event) override;
private:
    QString m_tipText;
};

class SyncConfigModel:public QStandardItemModel
{
public:
    explicit SyncConfigModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};

class CloudSyncPage:public QWidget
{
    Q_OBJECT
public:
    explicit CloudSyncPage(QWidget *parent = nullptr);

    void setSyncModel(SyncModel *syncModel);

    void setSyncWorker(SyncWorker *worker);

    void onLogin();
Q_SIGNALS:
    void onUserLogout();

    void clearCloudData();

    void requestSetAutoSync(bool enable) const;

    void requestSetModuleState(const QString &syncType, bool state);

    void requestSetUtcloudModuleState(const QString &utcloudType, bool state);
public slots:
    void onModuleStateChanged(std::pair<SyncType, bool> state);

    void onUtcloudModuleStateChanged(const QString& itemKey, bool state);

    void onUserInfoChanged(const QVariantMap &infos);
private slots:
    void expandSysConfig(bool checked);

    void addSwitcherDumpDate(QList<Apps*> &appDate);

    void onAutoSyncChanged(bool state);

    void onLastSyncTimeChanged(const qlonglong lastSyncTime);

    void checkPassword();

    void enableSyncConfig(bool state);
private:
    void initSysConfig();

    void initUI();

    void initConnection();

    void SyncTimeLblVisible(bool isVisible);

    void makeContentDisable(bool enable = false);

    void makeSwitchDisable(const QString &tiptext);

    void initRegisterDialog(RegisterDlg *dlg);

    void initVerifyDialog(VerifyDlg *dlg);
private:
    QVBoxLayout *m_mainlayout;
    DTipLabel *m_autoSyncTips;
    DLabel *m_bindedTips;
    DTipLabel *m_syncTimeTips;
    DCC_NAMESPACE::SwitchWidget *m_autoSyncSwitch;
    SyncItemWidget *m_syncItem;
    DCommandLinkButton *m_clearBtn;
    WarnLabel *m_labelWarn;
    SyncConfigModel *m_configModel;
    QStandardItemModel *m_itemModel;
    QList<QVariantList> m_sysConfig;
    DDialog *m_clearDlg;
    DHorizontalLine *m_btnLine;
    SyncModel *m_syncModel;
    SyncWorker *m_syncWorker;
    QMap<QString, QStandardItem *> m_utcloudItemMap;
    bool m_syncState;
};

#endif // CLOUDSYNC_H
