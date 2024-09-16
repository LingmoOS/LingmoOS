// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KBLAYOUTPROVIDER_H
#define KBLAYOUTPROVIDER_H

#include "abstractosdprovider.h"

#include "org_deepin_dde_inputdevice1_keyboard.h"

using namespace org::deepin::dde::inputdevice1;

/*!
 * \~chinese \class DisplayModeProvider
 * \~chinese \brief 切换键盘布局时的通知类
 */
class KBLayoutProvider : public AbstractOSDProvider
{
    Q_OBJECT
public:
    explicit KBLayoutProvider(QObject *parent = 0);

    bool checkConditions() const override;
    QSize contentSize() const override;
    QMargins contentMargins() const override;

    void highlightCurrent() override;
    void highlightNext() override;

    void sync() override;

    // model
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    int currentIndex() const;

    // delegate
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    KeyboardLayoutList m_database;

    QStringList m_userLayouts;
    QString m_userCurrentLayout;

    Keyboard *m_keyboardInter;

    QString describeLayout(const QString &layout) const;

private slots:
    void currentLayoutChanged(const QString &layout);
    void userLayoutListChanged(const QStringList &layouts);
};

#endif // KBLAYOUTPROVIDER_H
