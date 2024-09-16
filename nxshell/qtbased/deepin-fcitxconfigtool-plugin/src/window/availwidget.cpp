// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "availwidget.h"
#include "immodel/immodel.h"
#include "widgets/settingshead.h"
#include "widgets/settingsgroup.h"
#include "widgets/imsettingsitem.h"
#include "widgets/contentwidget.h"
#include "publisher/publisherdef.h"
#include "fcitxInterface/global.h"
#include <QVBoxLayout>
#include <KLocalizedString>

using namespace Fcitx;
using namespace dcc_fcitx_configtool::widgets;
bool operator==(const FcitxQtInputMethodItem &item, const FcitxQtInputMethodItem &item2);

// kcm代码 获取语言名称 简体中文 繁体中文 英文等 需要优化
QString languageName(const QString &langCode)
{
    QString languageName("Unknown");
    QLocale locale(langCode);
    if (langCode.isEmpty() || (langCode == "*")||(locale.language() == QLocale::C)) {
        return languageName;
    } else {
        languageName = i18nd("iso_639", QLocale::languageToString(locale.language()).toUtf8());
        return languageName.isEmpty()?QString("Unknown"):languageName;
    }
}

AvailWidget::AvailWidget(QWidget *parent)
    : QWidget(parent)
{
    initUI();
}

AvailWidget::~AvailWidget()
{
    m_allIMGroup->clear();
    m_searchIMGroup->clear();
    DeleteObject_Null(m_allIMGroup);
    DeleteObject_Null(m_searchIMGroup);
    DeleteObject_Null(m_mainLayout);
}

void AvailWidget::initUI()
{
    //界面布局
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
    //滑动窗口
    FcitxContentWidget *scrollArea = new FcitxContentWidget(this);
    scrollArea->setAccessibleName("scrollArea");
    QWidget *scrollAreaWidgetContents = new QWidget(scrollArea);
    scrollAreaWidgetContents->setAccessibleName("scrollAreaWidgetContents");
    QVBoxLayout *scrollAreaLayout = new QVBoxLayout(scrollAreaWidgetContents);
    scrollAreaLayout->setSpacing(0);
    scrollArea->setContent(scrollAreaWidgetContents);
    scrollAreaWidgetContents->setLayout(scrollAreaLayout);
    //搜索输入法列表 可用输入法列表
    m_allIMGroup = new FcitxSettingsGroup;
    m_allIMGroup->setSpacing(10);
    m_allIMGroup->setAccessibleName("allIMGroup");
    m_searchIMGroup = new FcitxSettingsGroup;
    m_searchIMGroup->setAccessibleName("searchIMGroup");
    m_searchIMGroup->setVerticalPolicy();
    m_searchIMGroup->setSpacing(5);
    //控件添加至滑动窗口内
    scrollAreaLayout->addWidget(m_allIMGroup);
    scrollAreaLayout->addSpacing(10);
    scrollAreaLayout->addWidget(m_searchIMGroup);
    scrollAreaLayout->addStretch();
    scrollAreaLayout->setObjectName("scrollAreaLayout");
    //添加至主界面内
    m_mainLayout->addWidget(scrollArea);
}

void AvailWidget::initConnect()
{
    connect(IMModel::instance(), SIGNAL(availIMListChanged(FcitxQtInputMethodItemList)), this, SLOT(onUpdateUI(FcitxQtInputMethodItemList)));
}

void AvailWidget::onUpdateUI()
{
    onUpdateUI(IMModel::instance()->getAvailIMList());
    initConnect();
}

void AvailWidget::onUpdateUI(FcitxQtInputMethodItemList IMlist)
{
    if (!Global::instance()->inputMethodProxy()) {
        m_allIMGroup->clear(); //清空group
        m_searchIMGroup->clear();
        m_allAvaiIMlList.clear();
        emit seleteIM(false);
        return;
    }

    if (m_searchStr.isEmpty()) {
        m_allIMGroup->show();
        m_searchIMGroup->hide();
    } else {
        m_allIMGroup->hide();
        m_searchIMGroup->show();
    }

    if (m_allAvaiIMlList == IMlist) {
        emit seleteIM((m_allAvaiIMlList.indexOf(m_selectItem) != -1));
        return;
    }
    m_allAvaiIMlList.swap(IMlist);

    QList<QString> useLanguage;
    FcitxQtInputMethodItemList CurIMlist = IMModel::instance()->getCurIMList();
    Q_FOREACH (const FcitxQtInputMethodItem &im, CurIMlist) {
        useLanguage.append(languageName(im.langCode()));
    }
    if(useLanguage.isEmpty()) {
        QString lang = QString(getenv("LANG")).split('_').first();
        useLanguage.append(languageName(lang));
    }
    //fcitx原有逻辑 不需要修改 __begin
    QMap<QString, int> languageMap;
    QList<QPair<QString, FcitxQtInputMethodItemList>> filteredIMEntryList;

    for (auto im = m_allAvaiIMlList.begin();im != m_allAvaiIMlList.end();im++ ) {
        if (!im->enabled()) {
            int idx;
            if (!languageMap.contains(im->langCode())) {
                idx = filteredIMEntryList.count();
                languageMap[im->langCode()] = idx;
                filteredIMEntryList.append(QPair<QString, FcitxQtInputMethodItemList>(im->langCode(), FcitxQtInputMethodItemList()));
            } else {
                idx = languageMap[im->langCode()];
            }
            filteredIMEntryList[idx].second.append(*im);
        }
    }

    std::map<QString, FcitxQtInputMethodItemList> tmpIM;
    for (auto it = filteredIMEntryList.begin(); it != filteredIMEntryList.end(); ++it) {
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            QString imcodeName = languageName(it2->langCode());
            if(imcodeName.compare("Unknown") != 0)
            {
                if (tmpIM.find(imcodeName) == tmpIM.end()) {
                    tmpIM.insert(std::pair<QString, FcitxQtInputMethodItemList>(imcodeName, FcitxQtInputMethodItemList()));
                }
                tmpIM[imcodeName].push_back(*it2);
            }
        }
    }

    //fcitx原有逻辑 不需要修改 __end
    auto createIMSttings = [ = ](FcitxSettingsGroup * group, const FcitxQtInputMethodItem & imItem) {
        FcitxIMSettingsItem *item = new FcitxIMSettingsItem();
        connect(item, &FcitxIMSettingsItem::itemClicked, [ = ](FcitxIMSettingsItem * item) {
            m_selectItem = item->m_item;
            emit seleteIM(true);
        });
        item->setParent(group);
        item->setFcitxItem(imItem);
        group->appendItem(item);
        if (group == m_searchIMGroup) {
            item->setFilterStr(m_searchStr);
        }
        if (imItem.name() == m_selectItem.name()
                && imItem.langCode() == m_selectItem.langCode()
                && imItem.uniqueName() == m_selectItem.uniqueName()) {
            item->setItemSelected(true);
            emit seleteIM(true);
        }
    };
    //清空group
    m_allIMGroup->clear();
    m_searchIMGroup->clear();

    for (auto it = tmpIM.rbegin(); it != tmpIM.rend(); ++it) {
        if(!useLanguage.contains(it->first)){
           continue;
        }
        FcitxSettingsHead *head = new FcitxSettingsHead();
        head->setEditEnable(false);
        head->setTitle(it->first);
        head->layout()->setContentsMargins(10, 0, 0, 0);
        m_allIMGroup->appendItem(head, FcitxSettingsGroup::NoneBackground);
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            createIMSttings(m_allIMGroup, *it2);
            createIMSttings(m_searchIMGroup, *it2);
        }
        break;
    }
    //添加item
    for (auto it = tmpIM.rbegin(); it != tmpIM.rend(); ++it) {
        if(useLanguage.contains(it->first)){
           continue;
        }
        FcitxSettingsHead *head = new FcitxSettingsHead();
        head->setEditEnable(false);
        head->setTitle(it->first);
        head->layout()->setContentsMargins(10, 0, 0, 0);
        m_allIMGroup->appendItem(head, FcitxSettingsGroup::NoneBackground);
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            createIMSttings(m_allIMGroup, *it2);
            createIMSttings(m_searchIMGroup, *it2);
        }
    }
    disconnect(IMModel::instance(), SIGNAL(availIMListChanged(FcitxQtInputMethodItemList)), this, SLOT(onUpdateUI(FcitxQtInputMethodItemList)));
}

void AvailWidget::clearItemStatus()
{
    m_selectItem = FcitxQtInputMethodItem();
    clearItemStatusAndFilter(m_allIMGroup, true);
}

void AvailWidget::removeSeleteIm(const FcitxQtInputMethodItem & temp)
{ 
    FcitxQtInputMethodItemList newTempList;
    newTempList.append(m_allAvaiIMlList);
    newTempList.removeOne(m_selectItem);
    onUpdateUI(newTempList);
    clearItemStatus();
}

void AvailWidget::addSeleteIm(const FcitxQtInputMethodItem & temp)
{
    FcitxQtInputMethodItemList newTempList;
    newTempList.append(m_allAvaiIMlList);
    newTempList.append(temp);
    onUpdateUI(newTempList);
    clearItemStatus();
}

void AvailWidget::clearItemStatusAndFilter(FcitxSettingsGroup *group, const bool &flag)
{
    if (!group) {
        return;
    }

    for (int i = 0; i < group->itemCount(); ++i) {
        FcitxIMSettingsItem *item = dynamic_cast<FcitxIMSettingsItem *>(group->getItem(i));
        if (item) {
            item->setItemSelected(false);
            if (flag) {
                item->setFilterStr(m_searchStr);
            }
        }
    }
}

void AvailWidget::onSearchIM(const QString &str)
{
    m_searchStr = str;
    m_selectItem = FcitxQtInputMethodItem();
    emit seleteIM(false);

    if (m_searchStr.isEmpty()) {
        m_allIMGroup->show();
        m_allIMGroup->setVisible(true);
        m_searchIMGroup->hide();
        m_searchIMGroup->setVisible(false);
        clearItemStatusAndFilter(m_allIMGroup, false);
    } else {
        m_searchIMGroup->show();
        m_searchIMGroup->setVisible(true);
        m_allIMGroup->hide();
        m_allIMGroup->setVisible(false);
        clearItemStatusAndFilter(m_searchIMGroup, true);
    }
}
