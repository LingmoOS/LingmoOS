/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef PROXY_H
#define PROXY_H


#include <QObject>
#include <QtPlugin>
#include <QFileSystemWatcher>

#include <QDialog>
#include <QLineEdit>
#include "certificationdialog.h"

#include <QGSettings>
#include <QRadioButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QTextEdit>
#include <QButtonGroup>
#include <QComboBox>
#include <QListWidget>
#include <QFormLayout>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusReply>
#include <QMessageBox>
#include <QtDBus/QDBusMetaType>

#include "interface.h"
#include "titlelabel.h"
#include "hoverwidget.h"
#include "applistwidget.h"

#include "kswitchbutton.h"
#include "kpasswordedit.h"
#include "ukcccommon.h"

using namespace kdk;
using namespace ukcc;

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

#include <glib.h>
#include <gio/gio.h>

struct GSData
{
    QString key;
    QString schema;
};

typedef enum{
    NONE,
    MANUAL,
    AUTO
}ProxyMode;

//自定义类型使用QVariant需要使用 Q_DECLARE_METATYPE 注册
Q_DECLARE_METATYPE(ProxyMode)
Q_DECLARE_METATYPE(GSData)

namespace Ui {
class Proxy;
}

class Proxy : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Proxy();
    ~Proxy();

    QString plugini18nName() Q_DECL_OVERRIDE;
    int pluginTypes() Q_DECL_OVERRIDE;
    QWidget * pluginUi() Q_DECL_OVERRIDE;
    void plugin_leave()Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;
    QString translationPath() const  Q_DECL_OVERRIDE;

public:
    void initUi(QWidget *widget);
    void initSearchText();
    void retranslateUi();
    void setupComponent();
    void setupConnect();
    void initProxyModeStatus();
    void initAutoProxyStatus();
    void initManualProxyStatus();
    void initIgnoreHostStatus();
    void initDbus();
    void initAppProxyStatus();

    void manualProxyTextChanged(QString txt);
    int _getCurrentProxyMode();
    void _setSensitivity();
    bool getAptProxyInfo(bool status);
    static void setAptProxy(QString host ,QString port ,bool status); //  apt代理对应的配置文件的写入或删除
    static QHash<QString, QVariant> getAptProxy();
    void setAptInfo();
    void reboot(); // 调用重启接口

    void setFrame_Noframe(QFrame *frame);
    QFrame *setLine(QFrame *frame);

    bool getAppProxyState(); //获取应用代理开启状态--调用Dbus
    void setAppProxyState(bool state); //设置应用代理开启状态--调用Dbus
    static QStringList getAppProxyConf(); //获取应用代理配置信息--调用Dbus
    void setAppProxyConf(QStringList list); //设置应用代理配置信息--调用Dbus
    static QMap<QString, QStringList> getAppListProxy();
//    bool checkIsChanged(QStringList info);
    void setUkccProxySettings();  // 设置控制面板代理模块显示/隐藏
private:
    void setAppProxyFrameUi(QWidget *widget);
    void setAppListFrameUi(QWidget *widget);
    void appProxyInfoPadding();
    void appListPadding();
    bool getipEditState(QString text);
    void setSystemProxyFrameHidden(bool state);
    void setAppProxyFrameHidden(bool state);
    void setAPTProxyFrameHidden(bool state);

    QFrame *m_sysSpacerFrame;
    QFrame *m_appListSpacerFrame;
    QFrame *m_appSpacerFrame;

    QString pluginName;
    int pluginType;
    QWidget * pluginWidget;

    TitleLabel *mTitleLabel;
    TitleLabel *m_appProxyLabel;
    TitleLabel *mAptProxyLabel;
    QLabel *mUrlLabel;
    QLabel *mHTTPLabel;
    QLabel *mHTTPPortLabel;
    QLabel *mHTTPSLabel;
    QLabel *mHTTPSPortLabel;
    QLabel *mFTPLabel;
    QLabel *mFTPPortLabel;
    QLabel *mSOCKSLabel;
    QLabel *mSOCKSPortLabel;
    QLabel *mIgnoreLabel;
    QLabel *mAptLabel;
    QLabel *mAPTHostLabel_1;
    QLabel *mAPTHostLabel_2;
    QLabel *mAPTPortLabel_1;
    QLabel *mAPTPortLabel_2;

    QLabel *mCertificationLabel;
    QLabel *mUserNameLabel;
    QLabel *mPwdLabel;

    QLabel *m_appEnableLabel;
    QLabel *m_proxyTypeLabel;
    QLabel *m_ipAddressLabel;
    QLabel *m_ipHintsLabel;
    QLabel *m_portLabel;
    QLabel *m_userNameLabel;
    QLabel *m_pwdLabel;
    QLabel *m_allowAppProxyLabel;

    QFrame *mProxyFrame;
    QFrame *mEnableFrame;
    QFrame *mSelectFrame;
    QFrame *mUrlFrame;

    QFrame *mHTTPFrame;
    QFrame *mHTTPSFrame;
    QFrame *mFTPFrame;
    QFrame *mSOCKSFrame;
    QFrame *mIgnoreFrame;
    QFrame *mCertificationFrame_1;

    QFrame *m_appEnableFrame;
    QFrame *m_appProxyFrame;
    QFrame *m_proxyTypeFrame;
    QFrame *m_ipAddressFrame;
    QFrame *m_portFrame;
    QFrame *m_userNameFrame;
    QFrame *m_pwdFrame;
//    QFrame *m_appBtnFrame;
    QFrame *m_appListFrame;

    QFrame *mAPTFrame;
    QFrame *mAPTFrame_1;
    QFrame *mAPTFrame_2;

    QFrame *line_1;
    QFrame *line_2;
    QFrame *line_3;
    QFrame *line_4;
    QFrame *line_5;
    QFrame *line_6;
    QFrame *line_7;
    QFrame *line_8;
    QFrame *m_appLine1;
    QFrame *m_appLine2;
    QFrame *m_appLine3;
    QFrame *m_appLine4;
    QFrame *m_appLine5;

    QRadioButton *mAutoBtn;
    QRadioButton *mManualBtn;
    KSwitchButton *mEnableBtn;
    KSwitchButton *mAptBtn;
    QPushButton *mEditBtn;
    QCheckBox *mCertificationBtn;

    QButtonGroup *mProxyBtnGroup;

    QLineEdit *mUrlLineEdit;
    QLineEdit *mHTTPLineEdit_1;
    QLineEdit *mHTTPLineEdit_2;
    QLineEdit *mHTTPSLineEdit_1;
    QLineEdit *mHTTPSLineEdit_2;
    QLineEdit *mFTPLineEdit_1;
    QLineEdit *mFTPLineEdit_2;
    QLineEdit *mSOCKSLineEdit_1;
    QLineEdit *mSOCKSLineEdit_2;
    QLineEdit *mUserNameLineEdit;
    QLineEdit *mPwdLineEdit;
    QLineEdit *m_ipAddressLineEdit;
    QLineEdit *m_portLineEdit;
    QLineEdit *m_userNameLineEdit;

    KSwitchButton *m_appEnableBtn;
    QComboBox *m_proxyTypeComboBox;
    KPasswordEdit *m_pwdLineEdit = nullptr;
//    QPushButton *m_cancelBtn;
//    QPushButton *m_saveBtn;
    QListWidget *m_appListWidget = nullptr;
    QWidget *m_appProxyInfoWidget;

    QTextEdit *mIgnoreLineEdit;

    QGSettings * proxysettings;
    QGSettings * httpsettings;
    QGSettings * securesettings;
    QGSettings * ftpsettings;
    QGSettings * sockssettings;
    QGSettings * aptsettings;

    QFileSystemWatcher *mfileWatch_1;
    QFileSystemWatcher *mfileWatch_2;

//    QDBusInterface *mAptproxyDbus;
    QDBusInterface *m_appProxyDbus = nullptr;

    bool isExistSettings = false;
    bool settingsCreate;
    bool mFirstLoad;
    QStringList m_appProxyInfo;
    QStringList m_appCheckedList;

private slots:
    void setAptProxySlot();  //处理apt代理前端交互逻辑
    void setAppProxyUiEnable(bool enable);
    void onipEditStateChanged(); //IP地址无效提示
    void onAppProxyConfChanged(); //应用代理配置信息变化
    void onAppProxyConfEditFinished();
    void onPaletteChanged();
//    void onCancelBtnClicked();
//    void onSaveBtnClicked();
//    void setBtnEnable();
};

#endif // PROXY_H
