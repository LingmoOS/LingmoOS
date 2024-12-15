// SPDX-FileCopyrightText: 2011-2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGS_H
#define SETTINGS_H

#include "dsettingsdialog.h"
#include <qsettingbackend.h>
#include <DKeySequenceEdit>
#include <DDialog>
#include <QSettings>
#include <QPointer>
#include <QKeyEvent>
#include <QDebug>
#include <DApplication>
#include <QLabel>
#include <QPushButton>
#include <QMutex>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
DTK_USE_NAMESPACE

class CustemBackend : public DSettingsBackend
{
    Q_OBJECT
public:
    explicit CustemBackend(const QString &filepath, QObject *parent = nullptr);
    ~CustemBackend();

    //获取参数所有的key值
    QStringList keys() const override;
    //根据key获取内容
    QVariant getOption(const QString &key) const;
    //同步数据
    void doSync() override;
    //根据key值设置内容
    void doSetOption(const QString &key, const QVariant &value) override;

    QSettings *m_settings {nullptr};
    QMutex m_writeLock;
};

class Settings : public QObject
{
    Q_OBJECT
public:
    explicit Settings(QWidget *parent = 0);
    ~Settings();

    void dtkThemeWorkaround(QWidget *parent, const QString &theme);
    //static QWidget *createFontComBoBoxHandle(QObject *obj);
    static QPair<QWidget*, QWidget*> createFontComBoBoxHandle(QObject *obj);
    static QPair<QWidget*, QWidget*> createKeySequenceEditHandle(QObject *obj);
    static QWidget* createSavingPathWgt(QObject* objg);
    static Settings* instance();
    void setSettingDialog(DSettingsDialog *settingsDialog);

    int m_iDefaultFontSize = 12;
    int m_iMaxFontSize = 50;
    int m_iMinFontSize = 8;
    DSettings *settings {nullptr};

    void setSavePath(int id,const QString& path);
    QString getSavePath(int id);
    void setSavePathId(int id);
    int getSavePathId();

signals:
    void sigAdjustFont(QString name);
    void sigAdjustFontSize(qreal fontSize);
    void sigAdjustTabSpaceNumber(int number);
    void sigAdjustWordWrap(bool enable);
    void sigAdjustBookmark(bool enable);
    void sigShowCodeFlodFlag(bool enable);
    void sigShowBlankCharacter(bool enable);
    void sigHightLightCurrentLine(bool enable);
    void sigThemeChanged(const QString &theme);
    void sigSetLineNumberShow(bool bIsShow);
    void sigChangeWindowSize(QString mode);

public slots:
    //自定义快捷键 ut002764 2021.6.28
    void slotCustomshortcut(const QString &key, const QVariant &value);

    //添加信号槽 ut002764-07-05
    void slotsigAdjustFont(QVariant value);
    void slotsigAdjustFontSize(QVariant value);
    void slotsigAdjustWordWrap(QVariant value);
    void slotsigSetLineNumberShow(QVariant value);
    void slotsigAdjustBookmark(QVariant value);
    void slotsigShowCodeFlodFlag(QVariant value);
    void slotsigShowBlankCharacter(QVariant value);
    void slotsigHightLightCurrentLine(QVariant value);
    void slotsigAdjustTabSpaceNumber(QVariant value);
    void slotupdateAllKeysWithKeymap(QVariant value);

private:
    void updateAllKeysWithKeymap(QString keymap);
    void copyCustomizeKeysFromKeymap(QString keymap);
    bool checkShortcutValid(const QString &Name, QString Key, QString &Reason, bool &bIsConflicts, QString defaultValue = QString::null);
    bool isShortcutConflict(const QString &Name, const QString &Key);
    DDialog *createDialog(const QString &title, const QString &content, const bool &bIsConflicts);
    void removeLockFiles();

private:
    Dtk::Core::QSettingBackend *m_backend {nullptr};

    bool m_bUserChangeKey = false;
    DSettingsDialog *m_pSettingsDialog;
    static Settings* s_pSetting;
    DDialog *m_pDialog;
};

class KeySequenceEdit : public DKeySequenceEdit
{
public:
    explicit KeySequenceEdit(DTK_CORE_NAMESPACE::DSettingsOption *opt, QWidget *parent = nullptr);
    /**
     * @brief option 获取设置页面操作句柄
     */
    DTK_CORE_NAMESPACE::DSettingsOption *option();
    /**
     * @brief slotDSettingsOptionvalueChanged 设置页面配置修改
     */
    void slotDSettingsOptionvalueChanged(const QVariant &value);

protected:
    /**
     * @brief eventFilter 设置页面快捷键类的事件过滤器
     */
    bool eventFilter(QObject *object, QEvent *event);

private:
    DTK_CORE_NAMESPACE::DSettingsOption *m_pOption = nullptr;
};

#endif // SETTINGS_H
