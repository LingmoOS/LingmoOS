// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KBLAYOUTINDICATOR_H
#define KBLAYOUTINDICATOR_H

#include <QSystemTrayIcon>
#include <QMenu>

#include "org_deepin_dde_inputdevice1_keyboard.h"
using KeyboardInterface = org::deepin::dde::inputdevice1::Keyboard;
/*!
 * \~chinese \class DataWrapper
 * \~chinese \brief 获取键盘布局的信息
 */
class DataWrapper : public QObject
{
    Q_OBJECT
public:
    explicit DataWrapper(KeyboardInterface *kinter, QObject *parent = nullptr);
    virtual ~DataWrapper() {}

    QString currentLayout() const;
    void setCurrentLayout(const QString &currentLayout);

    QStringList layoutList() const;
    void setLayoutList(const QStringList &layoutList);

    QString localizedNameOfLayout(const QString &layout) const;

    QStringList userLayoutList() const;

signals:
    void layoutListChanged(const QStringList &list);
    void currentLayoutChanged(const QString &layout);

private:
    KeyboardInterface *m_keyboardInter;

    QString m_currentLayout;
    QStringList m_layoutList;
    KeyboardLayoutList m_layoutListAll;

    void initLayoutListAll();
};

/*!
 * \~chinese \class KBLayoutIndicator
 * \~chinese \brief dock栏显示键盘布局的控件
 */
class KBLayoutIndicator : public QSystemTrayIcon
{
    Q_OBJECT
public:
    explicit KBLayoutIndicator(QWidget *parent = nullptr);

private:
    KeyboardInterface *m_keyboardInter;

    DataWrapper *m_data;

    QMenu *m_menu;
    QAction *m_addLayoutAction;

    void updateMenu();
    void updateIcon();

    QString duplicateCheck(const QString &kb);
    QPixmap generateIconPixmap(const QString &layout, qreal dpr = 1.0);

    void switchToNextLayout();

    void handleDataChanged();

    void handleActionTriggered(QAction *action);
    void handleActivated(QSystemTrayIcon::ActivationReason reason);
};

#endif // KBLAYOUTINDICATOR_H
