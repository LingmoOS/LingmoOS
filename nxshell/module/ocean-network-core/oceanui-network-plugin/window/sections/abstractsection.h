// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ABSTRACTSECTION_H
#define ABSTRACTSECTION_H

#include "widgets/settingshead.h"

namespace DCC_NAMESPACE {
class SettingsGroup;
}

DWIDGET_BEGIN_NAMESPACE
class DStyleOptionButton;
DWIDGET_END_NAMESPACE

class QVBoxLayout;

class AbstractSection : public QFrame
{
    Q_OBJECT

public:
    explicit AbstractSection(QFrame *parent = nullptr);
    explicit AbstractSection(const QString &title, QFrame *parent = nullptr);
    virtual ~AbstractSection();

    virtual bool allInputValid() = 0;
    virtual void saveSettings() = 0;

    void setTitle(const QString &title);
    void appendItem(DCC_NAMESPACE::SettingsItem *item);
    void insertItem(int idx, DCC_NAMESPACE::SettingsItem *item);
    void removeItem(DCC_NAMESPACE::SettingsItem *item);
    void setSettingsHead(DCC_NAMESPACE::SettingsHead *settingsHead);
    int itemIndex(DCC_NAMESPACE::SettingsItem *item);

Q_SIGNALS:
    void requestNextPage(QWidget *const page) const;
    void requestFrameAutoHide(const bool autoHide) const;
    void editClicked();

private:
    DCC_NAMESPACE::SettingsGroup *m_group;
    QVBoxLayout *m_layout;
    DCC_NAMESPACE::SettingsHead *m_head;
};

#endif /* ABSTRACTSECTION_H */
