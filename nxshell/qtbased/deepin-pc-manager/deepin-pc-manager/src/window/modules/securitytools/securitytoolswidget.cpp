// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "securitytoolswidget.h"

#include "defsecuritytoolinfo.h"
#include "defsecuritytoolsfrontunit.h"
#include "securitytoolsmodel.h"
#include "window/modules/securitytools/defsecuritytoollistwidget.h"

#include <DFontSizeManager>
#include <DListView>
#include <DPalette>

#include <QDBusConnection>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QScroller>
#include <QSpacerItem>
#include <QStandardItem>
#include <QVBoxLayout>

#define PACKAGEKEY Qt::UserRole + 1

enum TOOLMSGPANE {
    DATA, // 有数据
    EMPTY // 无数据
};

SecurityToolsWidget::SecurityToolsWidget(SecurityToolsModel *model, DWidget *parent)
    : DWidget(parent)
    , m_model(model)
    , m_scrollAreaWidgetContents(nullptr)
    , m_pSecurityButtonBox(nullptr)
    , m_pMsgLabel(nullptr)
    , m_toolStatus(TOOLSSTATUS::ALL)
    , m_iClassify(TOOLCLASSIFY::ALLTOOLS)
    , m_pStackedWidget(nullptr)
{
    // 注册信号类型
    registerDefSecurityToolInfo();
    registerDefSecurityToolInfoList();
    this->setAccessibleName("rightWidget_securityToolsWidget");

    // 初始化UI
    initUI();
    initConnection();
    m_model->updateToolsInfo();
}

SecurityToolsWidget::~SecurityToolsWidget() { }

// 初始化界面
void SecurityToolsWidget::initUI()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    pMainLayout->setContentsMargins(10, 10, 10, 10);

    QHBoxLayout *phlayout = new QHBoxLayout;
    phlayout->setContentsMargins(22, 0, 20, 0);
    m_pSecurityButtonBox = new DButtonBox(this);
    m_pSecurityButtonBox->setOrientation(Qt::Horizontal);
    QList<DButtonBoxButton *> btnlist;
    DButtonBoxButton *pBtn = createBoxButton(tr("All"), m_pSecurityButtonBox);
    pBtn->setAccessibleName("securitytoolswidget_AllBtn");
    btnlist.append(pBtn);

    pBtn = createBoxButton(tr("System"), m_pSecurityButtonBox);
    pBtn->setAccessibleName("securitytoolswidget_SystemBtn");
    btnlist.append(pBtn);

    pBtn = createBoxButton(tr("Network"), m_pSecurityButtonBox);
    pBtn->setAccessibleName("securitytoolswidget_NetworkBtn");
    btnlist.append(pBtn);

    pBtn = createBoxButton(tr("Data"), m_pSecurityButtonBox);
    pBtn->setAccessibleName("securitytoolswidget_DataBtn");
    btnlist.append(pBtn);

    pBtn = createBoxButton(tr("Apps"), m_pSecurityButtonBox);
    pBtn->setAccessibleName("securitytoolswidget_AppsBtn");
    btnlist.append(pBtn);

    pBtn = createBoxButton(tr("Devices"), m_pSecurityButtonBox);
    pBtn->setAccessibleName("securitytoolswidget_DeviceBtn");
    btnlist.append(pBtn);
    m_pSecurityButtonBox->setButtonList(btnlist, true);
    btnlist.at(0)->setChecked(true);

    for (int i = 0; i < btnlist.size(); i++) {
        m_pSecurityButtonBox->setId(btnlist.at(i), TOOLCLASSIFY(i));
    }

    m_pDropDown = new DDropdown;
    phlayout->addWidget(m_pSecurityButtonBox);
    phlayout->addStretch();
    phlayout->addWidget(m_pDropDown);
    m_pDropDown->setAccessibleName("securitytoolswidget_Drop");

    DefSecurityToolListWidget *pSecurityList = new DefSecurityToolListWidget;
    pSecurityList->setAccessibleName("securitytoolswidget_SecurityView");
    pSecurityList->setSpacing(10);
    pSecurityList->setContentsMargins(0, 0, 0, 0);
    DWidget *pEmptyWidget = createEmptyDataPane();
    pEmptyWidget->setAccessibleName("securitytoolswidget_EmptyWidget");

    m_pStackedWidget = new DStackedWidget;
    m_pStackedWidget->setAccessibleName("securitytoolswidget_StackedWidget");
    m_pStackedWidget->setFixedSize(740, 500);
    m_pStackedWidget->insertWidget(TOOLMSGPANE::DATA, pSecurityList);
    m_pStackedWidget->insertWidget(TOOLMSGPANE::EMPTY, pEmptyWidget);
    pMainLayout->addLayout(phlayout);
    pMainLayout->addWidget(m_pStackedWidget, 0, Qt::AlignHCenter);
    pMainLayout->addStretch();
    setLayout(pMainLayout);
}

void SecurityToolsWidget::initConnection()
{
    QObject::connect(m_model, &SecurityToolsModel::sUpdateToolsInfo, this, [&] {
        clearToolInfoList();
        DEFSECURITYTOOLINFOLIST infos = m_model->getToolsInfo(m_toolStatus);
        DEFSECURITYTOOLINFOLIST showInfos = getToolInfoByClassify(infos);

        QStringList sInstallingApps = m_model->getInstallingApps();
        QStringList sUpdatingApps = m_model->getUpdatingApps();
        QStringList sUninstallingApps = m_model->getUninstallingApps();

        if (showInfos.empty()) {
            m_pStackedWidget->setCurrentIndex(TOOLMSGPANE::EMPTY);
        } else {
            m_pStackedWidget->setCurrentIndex(TOOLMSGPANE::DATA);
            DWidget *pWidget = m_pStackedWidget->widget(TOOLMSGPANE::DATA);
            DefSecurityToolListWidget *pSecurityList =
                qobject_cast<DefSecurityToolListWidget *>(pWidget);

            foreach (auto info, showInfos) {
                DefSecurityToolsFrontUnit *pUnit = new DefSecurityToolsFrontUnit(pSecurityList);
                pUnit->setAccessibleName("securitytoolswidget_Unit" + info.getPackageKey());
                pUnit->setToolsInfo(info);
                pUnit->setDataModel(m_model);
                QListWidgetItem *pItem = new QListWidgetItem;
                QString packageKey = info.getPackageKey();
                pItem->setData(PACKAGEKEY, packageKey);
                pItem->setSizeHint(QSize(UNITWIDTH, UNITHEIGHT));
                pSecurityList->addItem(pItem);
                pSecurityList->setItemWidget(pItem, pUnit);

                if (sInstallingApps.contains(packageKey)) {
                    pUnit->setAppStatus(DEFSECURITYTOOLSTATUS::INSTALLING);
                } else if (sUpdatingApps.contains(packageKey)) {
                    pUnit->setAppStatus(DEFSECURITYTOOLSTATUS::UPDATING);
                } else if (sUninstallingApps.contains(packageKey)) {
                    pUnit->setAppStatus(DEFSECURITYTOOLSTATUS::UNINSTALLING);
                }
            }

            pSecurityList->setSpacing(10);
        }
    });

    QObject::connect(m_model,
                     &SecurityToolsModel::notityAppStatusChanged,
                     this,
                     [&](const QString &strPackageKey, int status) {
                         DWidget *pWidget = m_pStackedWidget->widget(TOOLMSGPANE::DATA);
                         DefSecurityToolListWidget *pSecurityList =
                             qobject_cast<DefSecurityToolListWidget *>(pWidget);
                         int iCount = pSecurityList->count();

                         for (int i = 0; i < iCount; i++) {
                             QListWidgetItem *pItem = pSecurityList->item(i);
                             QString strPackageData = pItem->data(PACKAGEKEY).toString();

                             if (strPackageData == strPackageKey) {
                                 QWidget *pTempWidget = pSecurityList->itemWidget(pItem);
                                 DefSecurityToolsFrontUnit *pUnit =
                                     qobject_cast<DefSecurityToolsFrontUnit *>(pTempWidget);
                                 pUnit->setAppStatus(DEFSECURITYTOOLSTATUS(status));
                             }
                         }
                     });

    QObject::connect(m_pDropDown, &DDropdown::triggeredAll, this, [&]() {
        updateToolsMsgByStatus(TOOLSSTATUS::ALL);
    });

    QObject::connect(m_pDropDown, &DDropdown::triggeredInstalled, this, [&]() {
        updateToolsMsgByStatus(TOOLSSTATUS::INSTALLEDTOOL);
    });

    QObject::connect(m_pDropDown, &DDropdown::triggeredUnInstalled, this, [&]() {
        updateToolsMsgByStatus(TOOLSSTATUS::UNINSTALLEDTOOL);
    });

    QObject::connect(m_pSecurityButtonBox,
                     &DButtonBox::buttonClicked,
                     this,
                     [&](QAbstractButton *pBtn) {
                         int iID = m_pSecurityButtonBox->id(pBtn);
                         updateToolsMsgByClassify(TOOLCLASSIFY(iID));
                     });
}

DButtonBoxButton *SecurityToolsWidget::createBoxButton(const QString &strText, QWidget *pParent)
{
    Q_ASSERT(!strText.isEmpty() && "string can't be empty!");
    QFont font;
    font.setFamily("SourceHanSansSC");
    font.setWeight(QFont::Medium);
    font.setPixelSize(14);
    DButtonBoxButton *pBtn = new DButtonBoxButton(strText, pParent);
    pBtn->setAccessibleName(QString("SecurityToolsWidget_%1").arg(strText.at(0).toLower()));
    pBtn->setFixedSize(QSize(72, 30));
    pBtn->setFont(font);
    return pBtn;
}

void SecurityToolsWidget::clearToolInfoList()
{
    DWidget *pWidget = m_pStackedWidget->widget(TOOLMSGPANE::DATA);
    DefSecurityToolListWidget *pSecurityList = qobject_cast<DefSecurityToolListWidget *>(pWidget);
    int iCount = pSecurityList->count();

    for (int i = 0; i < iCount; i++) {
        QListWidgetItem *pItem = pSecurityList->item(i);
        QWidget *pUnitWidget = pSecurityList->itemWidget(pItem);
        pUnitWidget->deleteLater();
        pUnitWidget = nullptr;
    }

    pSecurityList->clear();
}

DEFSECURITYTOOLINFOLIST
SecurityToolsWidget::getToolInfoByClassify(const DEFSECURITYTOOLINFOLIST &infolist) const
{
    DEFSECURITYTOOLINFOLIST desInfolist;

    if (m_iClassify == TOOLCLASSIFY::ALLTOOLS) {
        desInfolist = infolist;
    } else {
        foreach (DEFSECURITYTOOLINFO info, infolist) {
            if (m_iClassify == info.iClassify) {
                desInfolist.push_back(info);
            }
        }
    }

    return desInfolist;
}

DWidget *SecurityToolsWidget::createEmptyDataPane()
{
    DWidget *pEmptyWidget = new DWidget;
    DLabel *pImageLabel = new DLabel;
    QPixmap pixmap = QIcon::fromTheme("No_related_tool_is_found").pixmap(96, 96);
    pImageLabel->setPixmap(pixmap);
    DLabel *pEmptyLabel = new DLabel;
    pEmptyLabel->setText(tr("No tools found"));
    QFont font;
    font.setFamily("SourceHanSansSC");
    font.setWeight(QFont::Medium);
    font.setPixelSize(12);
    pEmptyLabel->setFont(font);
    pEmptyLabel->setAlignment(Qt::AlignCenter);
    QVBoxLayout *pVlayout = new QVBoxLayout;
    pVlayout->setSpacing(20);
    pVlayout->addStretch();
    pVlayout->addWidget(pImageLabel, 0, Qt::AlignCenter);
    pVlayout->addWidget(pEmptyLabel);
    pVlayout->addStretch();
    pEmptyWidget->setLayout(pVlayout);
    return pEmptyWidget;
}

// 设置因主题改变图标
void SecurityToolsWidget::changeThemeType(ColorType themeType)
{
    Q_UNUSED(themeType);
}

void SecurityToolsWidget::updateToolsMsgByStatus(TOOLSSTATUS status)
{
    m_toolStatus = status;
    m_model->updateToolsInfo();
}

void SecurityToolsWidget::updateToolsMsgByClassify(TOOLCLASSIFY iClassify)
{
    m_iClassify = iClassify;
    m_model->updateToolsInfo();
}
