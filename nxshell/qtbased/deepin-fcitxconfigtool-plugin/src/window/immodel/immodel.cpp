// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "immodel.h"
#include "fcitxInterface/global.h"
#include "publisher/publisherdef.h"
#include "widgets/imactivityitem.h"
#include "imconfig.h"
#include "widgets/utils.h"
#include <QApplication>
#include <QProcess>
#include <com_deepin_daemon_inputdevice_keyboard.h>

using namespace Fcitx;
using namespace Dtk::Widget;

IMModel *IMModel::m_ins {nullptr};

bool operator==(const FcitxQtInputMethodItem &item, const FcitxQtInputMethodItem &item2)
{
    return item.name() == item2.name()
           && item.langCode() == item2.langCode()
           && item.uniqueName() == item2.uniqueName() && item.enabled() == item2.enabled();
}

IMModel::IMModel()
    : QObject(nullptr)
{
    onUpdateIMList();
    connect(Global::instance(), &Global::connectStatusChanged, this, &IMModel::onUpdateIMList);
}

IMModel::~IMModel()
{
}

IMModel *IMModel::instance()
{
    return m_ins == nullptr ? (m_ins = new IMModel()) : m_ins;
}

void IMModel::deleteIMModel()
{
    m_ins->IMListSave();
    DeleteObject_Null(m_ins);
}

void IMModel::setEdit(bool flag)
{
    m_isEdit = flag;
    if (!m_isEdit) {
        IMListSave();
        QTimer::singleShot(200, this, SLOT(addIMItem()));
    }
}

int IMModel::getIMIndex(const QString &IM) const
{
    if (IM.isEmpty()) {
        return -1;
    }

    for (int i = 0; i < m_curIMList.count(); ++i) {
        if (m_curIMList[i].name() == IM
                || m_curIMList[i].uniqueName().indexOf(IM, Qt::CaseInsensitive) != -1
                || m_curIMList[i].langCode().indexOf(IM, Qt::CaseInsensitive) != -1) {
            return i;
        }
    }
    return -1;
}

int IMModel::getIMIndex(const FcitxQtInputMethodItem &IM) const
{
    return getIMIndex(IM.name());
}

FcitxQtInputMethodItem IMModel::getIM(const int &index) const
{
    if (index > m_curIMList.count() || index < 0)
        return FcitxQtInputMethodItem();
    return m_curIMList[index];
}

void IMModel::onUpdateIMList()
{
    if (Global::instance()->inputMethodProxy()) {
        FcitxQtInputMethodItemList &&list = Global::instance()->inputMethodProxy()->iMList();
        FcitxQtInputMethodItemList curList, availList;

        Q_FOREACH (const FcitxQtInputMethodItem &im, list) {
            im.enabled() ? curList.append(im) : availList.append(im);
        }

        if (curList != m_curIMList) {
            m_curIMList.swap(curList);
            emit curIMListChanaged(m_curIMList);
        }

        if (availList != m_availeIMList) {
            m_availeIMList.swap(availList);
            emit availIMListChanged(m_availeIMList);
        }

    } else {
        m_availeIMList.clear();
        m_curIMList.clear();
        emit curIMListChanaged(m_curIMList);
        emit availIMListChanged(m_availeIMList);
    }
}

void IMModel::onAddIMItem(FcitxQtInputMethodItem item)
{
    if (item.name().isEmpty() || item.uniqueName().isEmpty())
        return;

    m_availeIMList.removeAll(item);
    item.setEnabled(true);
    m_curIMList.insert(1, item);
    IMListSave();
    emit curIMListChanaged(m_curIMList);
    modifySystemLayout(item, true);
    QTimer::singleShot(200, this, SLOT(addIMItem()));

}

void IMModel::addIMItem()
{
    emit availIMListChanged(m_availeIMList);
}

void IMModel::onDeleteItem(FcitxQtInputMethodItem item)
{
    m_curIMList.removeAll(item);
    item.setEnabled(false);
    m_availeIMList.append(item);
    IMListSave();
    modifySystemLayout(item, false);
}

void IMModel::onItemUp(FcitxQtInputMethodItem item)
{
    int row = getIMIndex(item);
    m_curIMList.swap(row, row - 1);
    IMListSave();
    emit IMItemSawp(row, row - 1);
}

void IMModel::onItemDown(FcitxQtInputMethodItem item)
{
    int row = getIMIndex(item);

    if (row == m_curIMList.count() - 1) {
        return;
    }

    m_curIMList.swap(row, row + 1);
    IMListSave();
    emit IMItemSawp(row, row + 1);
}

void IMModel::switchPoistion(FcitxQtInputMethodItem item, int dest)
{
    m_curIMList.removeOne(item);
    m_curIMList.insert(dest, item);
    IMListSave();
}

void IMModel::onConfigShow(const FcitxQtInputMethodItem &item)
{
    QString imName = item.name();
    QString imLangCode = item.langCode();
    QString imUniqueName = item.uniqueName();

    QStringList closeSrcImList {
        "chineseime", "iflyime", "sogoupinyin", "baidupinyin"};

    if (closeSrcImList.contains(imUniqueName)) {
        QProcess::startDetached("sh -c " + IMConfig::IMPluginKey(imUniqueName));
    } else if (imUniqueName.compare("huayupy") == 0) {
        QProcess::startDetached("sh -c \"" + IMConfig::IMPluginKey(imUniqueName) + " " + IMConfig::IMPluginPar(imUniqueName)+"\"");
    } else {
        QDBusPendingReply<QString>
        result = Global::instance()->inputMethodProxy()->GetIMAddon(imUniqueName);
        result.waitForFinished();
        if (result.isValid()) {
            QProcess::startDetached("sh -c \"fcitx-config-gtk3 " + result.value()+"\"");
        }
    }
}

void IMModel::IMListSave()
{
    if (Global::instance()->inputMethodProxy()) {
        FcitxQtInputMethodItemList &&list = (m_curIMList + m_availeIMList);
        if (list != Global::instance()->inputMethodProxy()->iMList()) {
            if (Global::instance()->inputMethodProxy()) {
                Global::instance()->inputMethodProxy()->setIMList(list);
                Global::instance()->inputMethodProxy()->ReloadConfig();
            }
        }
    }
}

// add or delete system layout
void IMModel::modifySystemLayout(FcitxQtInputMethodItem item, bool add)
{
    if (!dcc_fcitx_configtool::IsCommunitySystem) return;
    QString fcitxUniqueName = "fcitx-keyboard-";
    QString uniqueName = item.uniqueName();
    __Keyboard keyboard("com.deepin.daemon.InputDevices",
            "/com/deepin/daemon/InputDevice/Keyboard",
            QDBusConnection::sessionBus());

    QString layout;
    if (uniqueName.startsWith(fcitxUniqueName)) {
        layout = uniqueName.replace(fcitxUniqueName, "");
        int splitLoc = layout.indexOf('-');
        if (splitLoc > 0) {
            layout =  layout.replace(splitLoc,1,';');
        } else {
            layout = layout.append(';');
        }
    } else {
        if (0 == QString::compare("zh_CN", item.langCode())) {
            layout = "cn;";
        } else {
            layout = item.langCode();
            layout = layout.append(';');
        }
    }
    if (add) {
        keyboard.AddUserLayout(layout);
    } else {
        keyboard.DeleteUserLayout(layout);
    }
}
