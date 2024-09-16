// SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DCC_MODULE_IMPAGE_H
#define DCC_MODULE_IMPAGE_H

#include "dbusprovider.h"
#include "imconfig.h"
#include "layoutwidget.h"

#include <fcitxqtdbustypes.h>
#include <widgets/buttontuple.h>

#include <DAbstractDialog>
#include <DCommandLinkButton>
#include <DListView>
#include <DSearchEdit>
#include <DWidget>

#include <QDBusPendingCallWatcher>
#include <QWidget>

class QLabel;
class QPushButton;

class AddIMPage : public DTK_WIDGET_NAMESPACE::DAbstractDialog
{
    Q_OBJECT
public:
    AddIMPage(fcitx::kcm::DBusProvider *dbus, fcitx::kcm::IMConfig *config, QWidget *parent);
    ~AddIMPage();

Q_SIGNALS:
    void changed();
    void closeAddIMWindow();

public Q_SLOTS:
    void save();
    void load();
    void defaults();

private Q_SLOTS:
    void availIMCurrentChanged(const QModelIndex &index);
    void childIMSelectionChanged();

    void clickedFindMoreButton();

    void clickedCloseButton();
    void clickedAddButton();

private:
    fcitx::kcm::DBusProvider *m_dbus;
    fcitx::kcm::IMConfig *m_config;

    QHBoxLayout *m_contentLayout;
    QVBoxLayout *m_leftLayout;
    Dtk::Widget::DSearchEdit *m_searchEdit;
    QFrame *line;
    Dtk::Widget::DListView *m_availIMList;
    QVBoxLayout *m_rightLayout;
    Dtk::Widget::DListView *m_childIMList;
    LayoutWidget *m_laSelector;
    Dtk::Widget::DCommandLinkButton *m_findMoreLabel;
    DCC_NAMESPACE::ButtonTuple *m_buttonTuple;
};

#endif // !DCC_MODULE_IMPAGE_H
