/*
 *
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "panel-ukcc-plugin.h"
#include <QTranslator>
#include <QApplication>
#include "general-config-define.h"
#include "switch-button.h"

PanelUkccPlugin::PanelUkccPlugin(QObject *parent): QObject(parent)
{
    auto translator = new QTranslator(this);
    if(!translator->load(TRANSLATIONS_INSTALL_PATH + QLocale::system().name())) {
        qWarning() << TRANSLATIONS_INSTALL_PATH + QLocale::system().name() << "load failed";
    }
    QApplication::installTranslator(translator);
    initUI();
    connectToSource();
}

QString PanelUkccPlugin::plugini18nName()
{
    return tr("Panel");
}

int PanelUkccPlugin::pluginTypes()
{
    return PERSONALIZED;
}

QWidget *PanelUkccPlugin::pluginUi()
{
    return m_widget;
}

const QString PanelUkccPlugin::name() const
{
    // 不需要翻译，作为key，用于在控制面板激活并显示插件
    return "Panel";
}

QString PanelUkccPlugin::translationPath() const
{
    return QStringLiteral("/usr/share/lingmo-panel/panel-ukcc-plugin/translations/%1.ts");;
}

bool PanelUkccPlugin::isShowOnHomePage() const
{
    return false;
}

QIcon PanelUkccPlugin::icon() const
{
    return QIcon::fromTheme("lingmo-panel-symbolic");
}

bool PanelUkccPlugin::isEnable() const
{
    return true;
}

void PanelUkccPlugin::connectToSource()
{
    QString displayEnv = (qgetenv("XDG_SESSION_TYPE") == "wayland") ? QLatin1String("WAYLAND_DISPLAY") : QLatin1String("DISPLAY");
    QString display(qgetenv(displayEnv.toUtf8().data()));
    QUrl address(QStringLiteral("local:lingmo-panel-config-")  + QString(qgetenv("USER")) + display);

    qDebug() << "panel ukcc plugin connect to source:" << m_qroNode.connectToNode(address);
    m_configReplica = m_qroNode.acquire<RemoteConfigReplica>();
    connect(m_configReplica, &QRemoteObjectReplica::initialized, this, &PanelUkccPlugin::initData);
    connect(m_configReplica, &QRemoteObjectReplica::stateChanged, this, &PanelUkccPlugin::sourceStateChanged);
}

void PanelUkccPlugin::initUI()
{
    m_widget = new QWidget;
    // main page
    m_mainLayout = new QVBoxLayout(m_widget);
    m_mainLayout->setSpacing(8);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    // panel
    m_titleLabel1 = new TitleLabel(m_widget);
    //~ contents_path /Panel/Panel
    m_titleLabel1->setText(tr("Panel"));
    m_mainLayout->addWidget(m_titleLabel1);

    m_frame1 = new QFrame(m_widget);
    m_frame1->setFrameShape(QFrame::Shape::Box);
    m_vLayout1 = new QVBoxLayout(m_frame1);
    m_vLayout1->setContentsMargins(0, 0, 0, 0);
    m_vLayout1->setSpacing(0);

    //merge icons
    m_mergeIconsFrame = new QFrame(m_frame1);
    m_mergeIconsFrame->setFixedHeight(56);
    m_mergeIconsLayout = new QHBoxLayout(m_mergeIconsFrame);
    m_mergeIconsLayout->setContentsMargins(16, 0, 16, 0);

    m_mergeIconsLabel = new QLabel(m_mergeIconsFrame);
    m_mergeIconsLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //~ contents_path /Panel/Merge icons on the taskbar
    m_mergeIconsLabel->setText(tr("Merge icons on the taskbar"));
    m_mergeIcons = new QComboBox(m_mergeIconsFrame);
    m_mergeIcons->setEnabled(false);
    m_mergeIcons->setMinimumWidth(320);
    m_mergeIcons->addItem(tr("Always"), GeneralConfigDefine::MergeIcons::Always);
    m_mergeIcons->addItem(tr("Never"), GeneralConfigDefine::MergeIcons::Never);
    m_mergeIconsLayout->addWidget(m_mergeIconsLabel);
    m_mergeIconsLayout->addWidget(m_mergeIcons);
    m_vLayout1->addWidget(m_mergeIconsFrame);
    m_vLayout1->addWidget(setLine(m_frame1));

    //panel location
    m_panelLocationFrame = new QFrame(m_frame1);
    m_panelLocationFrame->setFixedHeight(56);
    m_panelLocationLayout = new QHBoxLayout(m_panelLocationFrame);
    m_panelLocationLayout->setContentsMargins(16, 0, 16, 0);

    m_panelLocationLabel = new QLabel(m_panelLocationFrame);
    m_panelLocationLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //~ contents_path /Panel/Panel location
    m_panelLocationLabel->setText(tr("Panel location"));
    m_panelLocation = new QComboBox(m_panelLocationFrame);
    m_panelLocation->setEnabled(false);
    m_panelLocation->setMinimumWidth(320);
    m_panelLocation->addItem(tr("Bottom"), GeneralConfigDefine::PanelLocation::Bottom);
    m_panelLocation->addItem(tr("Left"), GeneralConfigDefine::PanelLocation::Left);
    m_panelLocation->addItem(tr("Top"), GeneralConfigDefine::PanelLocation::Top);
    m_panelLocation->addItem(tr("Right"), GeneralConfigDefine::PanelLocation::Right);
    m_panelLocationLayout->addWidget(m_panelLocationLabel);
    m_panelLocationLayout->addWidget(m_panelLocation);
    m_vLayout1->addWidget(m_panelLocationFrame);
    m_vLayout1->addWidget(setLine(m_frame1));

    //panel size
    m_panelSizePolicyFrame = new QFrame(m_frame1);
    m_panelSizePolicyFrame->setFixedHeight(56);
    m_panelSizePolicyLayout = new QHBoxLayout(m_panelSizePolicyFrame);
    m_panelSizePolicyLayout->setContentsMargins(16, 0, 16, 0);

    m_panelSizePolicyLabel = new QLabel(m_panelSizePolicyFrame);
    m_panelSizePolicyLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //~ contents_path /Panel/Panel size
    m_panelSizePolicyLabel->setText(tr("Panel size"));
    m_panelSizePolicy = new QComboBox(m_panelSizePolicyFrame);
    m_panelSizePolicy->setEnabled(false);
    m_panelSizePolicy->setMinimumWidth(320);
    m_panelSizePolicy->addItem(tr("Small"), GeneralConfigDefine::PanelSizePolicy::Small);
    m_panelSizePolicy->addItem(tr("Medium"), GeneralConfigDefine::PanelSizePolicy::Medium);
    m_panelSizePolicy->addItem(tr("Large"), GeneralConfigDefine::PanelSizePolicy::Large);
    m_panelSizePolicy->addItem(tr("Custom"), GeneralConfigDefine::PanelSizePolicy::Custom);
    m_panelSizePolicyLayout->addWidget(m_panelSizePolicyLabel);
    m_panelSizePolicyLayout->addWidget(m_panelSizePolicy);
    m_vLayout1->addWidget(m_panelSizePolicyFrame);
    m_vLayout1->addWidget(setLine(m_frame1));

    //panel auto hide
    m_panelAutoHideFrame = new QFrame(m_frame1);
    m_panelAutoHideFrame->setFixedHeight(56);
    m_panelAutoHideLayout = new QHBoxLayout(m_panelAutoHideFrame);
    m_panelAutoHideLayout->setContentsMargins(16, 0, 16, 0);
    m_panelAutoHideLabel = new QLabel(m_panelAutoHideFrame);
    m_panelAutoHideLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //~ contents_path /Panel/Panel auto hide
    m_panelAutoHideLabel->setText(tr("Panel auto hide"));
    m_panelAutoHide = new kdk::KSwitchButton(m_panelAutoHideFrame);
    m_panelAutoHide->setFixedSize(48,24);
//    m_panelAutoHide->setCheckable(false);
    m_panelAutoHide->setEnabled(false);
    m_panelAutoHideLayout->addWidget(m_panelAutoHideLabel);
    m_panelAutoHideLayout->addWidget(m_panelAutoHide);
    m_vLayout1->addWidget(m_panelAutoHideFrame);
    m_vLayout1->addWidget(setLine(m_frame1));

    //panel lock
    m_panelLockFrame = new QFrame(m_frame1);
    m_panelLockFrame->setFixedHeight(56);
    m_panelLockLayout = new QHBoxLayout(m_panelLockFrame);
    m_panelLockLayout->setContentsMargins(16, 0, 16, 0);
    m_panelLockLabel = new QLabel(m_panelLockFrame);
    m_panelLockLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //~ contents_path /Panel/Panel lock
    m_panelLockLabel->setText(tr("Panel lock"));
    m_panelLock = new kdk::KSwitchButton(m_panelLockFrame);
    m_panelLock->setFixedSize(48,24);
//    m_panelLock->setCheckable(false);
    m_panelLock->setEnabled(false);
    m_panelLockLayout->addWidget(m_panelLockLabel);
    m_panelLockLayout->addWidget(m_panelLock);
    m_vLayout1->addWidget(m_panelLockFrame);
    m_vLayout1->addWidget(setLine(m_frame1));

    //show panel on all screens
    m_showPanelOnAllScreensFrame = new QFrame(m_frame1);
    m_showPanelOnAllScreensFrame->setFixedHeight(56);
    m_showPanelOnAllScreensLayout = new QHBoxLayout(m_showPanelOnAllScreensFrame);
    m_showPanelOnAllScreensLayout->setContentsMargins(16, 0, 16, 0);
    m_showPanelOnAllScreensLabel = new QLabel(m_showPanelOnAllScreensFrame);
    m_showPanelOnAllScreensLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //~ contents_path /Panel/Show panel on all screens
    m_showPanelOnAllScreensLabel->setText(tr("Show panel on all screens"));
    m_showPanelOnAllScreens = new kdk::KSwitchButton(m_showPanelOnAllScreensFrame);
    m_showPanelOnAllScreens->setFixedSize(48,24);
    m_showPanelOnAllScreens->setEnabled(false);
    m_showPanelOnAllScreensLayout->addWidget(m_showPanelOnAllScreensLabel);
    m_showPanelOnAllScreensLayout->addWidget(m_showPanelOnAllScreens);
    m_vLayout1->addWidget(m_showPanelOnAllScreensFrame);
    m_vLayout1->addWidget(setLine(m_frame1));

    //show system tray on all panel
    m_showSystemTrayOnAllPanelsFrame = new QFrame(m_frame1);
    m_showSystemTrayOnAllPanelsFrame->setFixedHeight(56);
    m_showSystemTrayOnAllPanelsLayout = new QHBoxLayout(m_showSystemTrayOnAllPanelsFrame);
    m_showSystemTrayOnAllPanelsLayout->setContentsMargins(16, 0, 16, 0);
    m_showSystemTrayOnAllPanelsLabel = new QLabel(m_showSystemTrayOnAllPanelsFrame);
    m_showSystemTrayOnAllPanelsLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //~ contents_path /Panel/Show system tray area on all panels
    m_showSystemTrayOnAllPanelsLabel->setText(tr("Show system tray area on all panels "));
    m_showSystemTrayOnAllPanels = new kdk::KSwitchButton(m_showSystemTrayOnAllPanelsFrame);
    m_showSystemTrayOnAllPanels->setFixedSize(48,24);
    m_showSystemTrayOnAllPanels->setEnabled(false);
    m_showSystemTrayOnAllPanelsLayout->addWidget(m_showSystemTrayOnAllPanelsLabel);
    m_showSystemTrayOnAllPanelsLayout->addWidget(m_showSystemTrayOnAllPanels);
    m_vLayout1->addWidget(m_showSystemTrayOnAllPanelsFrame);
    m_vLayout1->addWidget(setLine(m_frame1));

    //taskmanager icons showed on
    m_iconsShowedOnFrame = new QFrame(m_frame1);
    m_iconsShowedOnFrame->setFixedHeight(56);
    m_iconsShowedOnLayout = new QHBoxLayout(m_iconsShowedOnFrame);
    m_iconsShowedOnLayout->setContentsMargins(16, 0, 16, 0);
    m_iconsShowedOnLabel = new QLabel(m_iconsShowedOnFrame);
    m_iconsShowedOnLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //~ contents_path /Panel/When existing multiple panels, show window icons on
    m_iconsShowedOnLabel->setText(tr("When existing multiple panels, show window icons on"));
    m_iconsShowedOn = new QComboBox(m_iconsShowedOnFrame);
    m_iconsShowedOn->setEnabled(false);
    m_iconsShowedOn->setMinimumWidth(320);
    m_iconsShowedOn->addItem(tr("All panels"), GeneralConfigDefine::TaskBarIconsShowedOn::AllPanels);
    m_iconsShowedOn->addItem(tr("Primary screen panel and panel where window is open"), GeneralConfigDefine::TaskBarIconsShowedOn::PrimaryScreenPanelAndPanelWhereWindowIsOpen);
    m_iconsShowedOn->addItem(tr("Panel where window is open"), GeneralConfigDefine::TaskBarIconsShowedOn::PanelWhereWindowIsOpen);
    m_iconsShowedOnLayout->addWidget(m_iconsShowedOnLabel);
    m_iconsShowedOnLayout->addWidget(m_iconsShowedOn);
    m_vLayout1->addWidget(m_iconsShowedOnFrame);
    m_vLayout1->addWidget(setLine(m_frame1));

    m_mainLayout->addWidget(m_frame1);
    m_mainLayout->addSpacerItem(new QSpacerItem(40, 40, QSizePolicy::Fixed));

    //widgets showed on panel
    m_titleLabel2 = new TitleLabel(m_widget);
    //~ contents_path /Panel/Widgets always showed on panel
    m_titleLabel2->setText(tr("Widgets always showed on panel"));
    m_mainLayout->addWidget(m_titleLabel2);

    m_frame2 = new QFrame(m_widget);
    m_frame2->setFrameShape(QFrame::Box);
    m_vLayout2 = new QVBoxLayout(m_frame2);
    m_vLayout2->setContentsMargins(0, 0, 0, 0);
    m_vLayout2->setSpacing(0);

    //task view
    m_showTaskViewFrame = new QFrame(m_frame2);
    m_showTaskViewFrame->setFixedHeight(56);
    m_showTaskViewLayout = new QHBoxLayout(m_showTaskViewFrame);
    m_showTaskViewLayout->setContentsMargins(16, 0, 16, 0);
    m_showTaskViewLabel = new QLabel(m_showTaskViewFrame);
    m_showTaskViewLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_showTaskViewLabel->setText(tr("Task view"));
    m_showTaskView = new kdk::KSwitchButton(m_showTaskViewFrame);
    m_showTaskView->setFixedSize(48,24);
//    m_showTaskView->setCheckable(false);
    m_showTaskView->setEnabled(false);
    m_showTaskViewLayout->addWidget(m_showTaskViewLabel);
    m_showTaskViewLayout->addWidget(m_showTaskView);
    m_vLayout2->addWidget(m_showTaskViewFrame);
    m_vLayout2->addWidget(setLine(m_frame2));

    //show search
    m_showSearchFrame = new QFrame(m_frame2);
    m_showSearchFrame->setFixedHeight(56);
    m_showSearchLayout = new QHBoxLayout(m_showSearchFrame);
    m_showSearchLayout->setContentsMargins(16, 0, 16, 0);
    m_showSearchLabel = new QLabel(m_showSearchFrame);
    m_showSearchLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_showSearchLabel->setText(tr("Search"));
    m_showSearch = new kdk::KSwitchButton(m_showSearchFrame);
    m_showSearch->setFixedSize(48,24);
    m_showSearch->setEnabled(false);
    m_showSearchLayout->addWidget(m_showSearchLabel);
    m_showSearchLayout->addWidget(m_showSearch);
    m_vLayout2->addWidget(m_showSearchFrame);

    m_mainLayout->addWidget(m_frame2);
    m_mainLayout->addSpacerItem(new QSpacerItem(40, 40, QSizePolicy::Fixed));

    //Icons showed on system tray
    m_titleLabel3 = new TitleLabel(m_widget);
    //~ contents_path /Panel/Icons showed on system tray
    m_titleLabel3->setText(tr("Icons showed on system tray"));
    m_mainLayout->addWidget(m_titleLabel3);

    m_frame3 = new QFrame(m_widget);
//    m_frame3->setFixedHeight(56);
    m_frame3->setFrameShape(QFrame::Box);
    m_vLayout3 = new QVBoxLayout(m_frame3);
    m_vLayout3->setContentsMargins(0, 0, 0, 0);
    m_vLayout3->setSpacing(0);

   m_systemTrayIconIdName = {
        {"lingmo-power-manager-tray",tr("Power")},
        {"lingmo-volume-control-applet-qt", tr("Volume")},
        {"lingmo-nm", tr("Network")},
        {"lingmo-bluetooth", tr("Bluetooth")}
    };

    for(const QString &id : m_systemTrayIconId) {
        auto button = new SwitchButton(m_frame3, m_systemTrayIconIdName.value(id));
        button->setObjectName(id);
        button->setEnabled(false);
        m_vLayout3->addWidget(button);
        if(m_systemTrayIconId.indexOf(id) < m_systemTrayIconId.size()) {
            m_vLayout3->addWidget(setLine(m_frame3));
        }
    }
    m_mainLayout->addWidget(m_frame3);
    m_mainLayout->addStretch();
}

QFrame *PanelUkccPlugin::setLine(QFrame *parent)
{
    auto line = new QFrame(parent);
    line->setFixedHeight(1);
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    return line;
}

void PanelUkccPlugin::initData()
{
    m_mergeIcons->setCurrentIndex(m_configReplica->mergeIcons());
    connect(m_mergeIcons, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int index){
        m_configReplica->setMergeIcons(static_cast<GeneralConfigDefine::MergeIcons>(index));
    });
    connect(m_configReplica, &RemoteConfigReplica::mergeIconsChanged, m_mergeIcons, &QComboBox::setCurrentIndex, Qt::UniqueConnection);
    m_mergeIcons->setEnabled(true);

    m_panelLocation->setCurrentIndex(m_configReplica->panelLocation());
    connect(m_panelLocation, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int index){
        m_configReplica->setPanelLocation(static_cast<GeneralConfigDefine::PanelLocation>(index));
    });
    connect(m_configReplica, &RemoteConfigReplica::panelLocationChanged, m_panelLocation, &QComboBox::setCurrentIndex, Qt::UniqueConnection);
    m_panelLocation->setEnabled(!m_configReplica->panelLock());

    m_panelSizePolicy->setCurrentIndex(m_configReplica->panelSizePolicy());
    connect(m_panelSizePolicy, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int index){
        m_configReplica->setPanelSizePolicy(static_cast<GeneralConfigDefine::PanelSizePolicy>(index));
    });
    connect(m_configReplica, &RemoteConfigReplica::panelSizePolicyChanged, m_panelSizePolicy, &QComboBox::setCurrentIndex, Qt::UniqueConnection);
    m_panelSizePolicy->setEnabled(!m_configReplica->panelLock());

    m_panelAutoHide->setChecked(m_configReplica->panelAutoHide());
    connect(m_panelAutoHide, &kdk::KSwitchButton::stateChanged, m_configReplica, &RemoteConfigReplica::setPanelAutoHide);
    connect(m_configReplica, &RemoteConfigReplica::panelAutoHideChanged, m_panelAutoHide, &kdk::KSwitchButton::setChecked, Qt::UniqueConnection);
    m_panelAutoHide->setEnabled(true);

    m_panelLock->setChecked(m_configReplica->panelLock());
    connect(m_panelLock, &kdk::KSwitchButton::stateChanged, m_configReplica, &RemoteConfigReplica::setPanelLock);
    connect(m_configReplica, &RemoteConfigReplica::panelLockChanged, this, [&](bool panelLock) {
        m_panelLock->setChecked(panelLock);
        m_panelLocation->setEnabled(!panelLock);
        m_panelSizePolicy->setEnabled(!panelLock);
    }, Qt::UniqueConnection);
    m_panelLock->setEnabled(true);

    m_iconsShowedOn->setCurrentIndex(m_configReplica->taskBarIconsShowedOn());
    connect(m_iconsShowedOn, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int index){
        m_configReplica->setTaskBarIconsShowedOn(static_cast<GeneralConfigDefine::TaskBarIconsShowedOn>(index));
    });
    connect(m_configReplica, &RemoteConfigReplica::taskBarIconsShowedOnChanged, m_iconsShowedOn, &QComboBox::setCurrentIndex, Qt::UniqueConnection);
    m_iconsShowedOn->setEnabled(true);

    //show panel on all screens
    m_showPanelOnAllScreens->setChecked(m_configReplica->showPanelOnAllScreens());
    connect(m_showPanelOnAllScreens, &kdk::KSwitchButton::stateChanged, m_configReplica, &RemoteConfigReplica::setShowPanelOnAllScreens);
    connect(m_configReplica, &RemoteConfigReplica::showPanelOnAllScreensChanged, this, [&](bool showPanelOnAllScreens) {
        m_showPanelOnAllScreens->setChecked(showPanelOnAllScreens);
        //不在所有屏幕显示任务栏时，将任务栏图标选项设置为在主屏和打开了窗口的任务栏显示
        if(!showPanelOnAllScreens) {
            if(m_iconsShowedOn->currentIndex() == GeneralConfigDefine::TaskBarIconsShowedOn::PanelWhereWindowIsOpen) {
                m_iconsShowedOn->setCurrentIndex(GeneralConfigDefine::TaskBarIconsShowedOn::AllPanels);
            }
        }
        m_iconsShowedOn->setEnabled(showPanelOnAllScreens);
    }, Qt::UniqueConnection);
    m_showPanelOnAllScreens->setEnabled(true);

    m_showSystemTrayOnAllPanels->setChecked(m_configReplica->showSystemTrayOnAllPanels());
    connect(m_showSystemTrayOnAllPanels, &kdk::KSwitchButton::stateChanged, m_configReplica, &RemoteConfigReplica::setShowSystemTrayOnAllPanels);
    connect(m_configReplica, &RemoteConfigReplica::showSystemTrayOnAllPanelsChanged, m_showSystemTrayOnAllPanels, &kdk::KSwitchButton::setChecked, Qt::UniqueConnection);
    m_showSystemTrayOnAllPanels->setEnabled(true);

    m_showTaskView->setChecked(!m_configReplica->disabledWidgets().contains(QStringLiteral("org.lingmo.panel.taskView")));
    connect(m_configReplica, &RemoteConfigReplica::disabledWidgetsChanged, m_showTaskView, [this] (const QStringList& list) {
        m_showTaskView->setChecked(!list.contains(QStringLiteral("org.lingmo.panel.taskView")));
    }, Qt::UniqueConnection);
    connect(m_showTaskView, &kdk::KSwitchButton::stateChanged, this, [&](bool show){
        m_configReplica->disableWidget(QStringLiteral("org.lingmo.panel.taskView"), !show);
    });
    m_showTaskView->setEnabled(true);

    //是否始终显示搜索
    m_showSearch->setChecked(!m_configReplica->disabledWidgets().contains(QStringLiteral("org.lingmo.panel.search")));
    connect(m_configReplica, &RemoteConfigReplica::disabledWidgetsChanged, m_showSearch, [this] (const QStringList& list) {
        m_showSearch->setChecked(!list.contains(QStringLiteral("org.lingmo.panel.search")));
    }, Qt::UniqueConnection);
    connect(m_showSearch, &kdk::KSwitchButton::stateChanged, this, [&](bool show){
        m_configReplica->disableWidget(QStringLiteral("org.lingmo.panel.search"), !show);
    });
    m_showSearch->setEnabled(true);

    for(const QString &id : m_systemTrayIconId) {
        auto button = m_frame3->findChild<SwitchButton *>(id, Qt::FindDirectChildrenOnly);
        if(button) {
            button->setChecked(!m_configReplica->trayIconsInhibited().contains(id));
            connect(button, &SwitchButton::stateChanged, this, [&, button](bool show){
                QStringList icons = m_configReplica->trayIconsInhibited();
                if(!show && !icons.contains(button->objectName())) {
                    m_configReplica->setTrayIconsInhibited(icons << button->objectName());
                } else if (show && icons.contains(button->objectName())) {
                    icons.removeAll(button->objectName());
                    m_configReplica->setTrayIconsInhibited(icons);
                }
            });
            button->setEnabled(true);
        }
    }
}

void PanelUkccPlugin::sourceStateChanged(QRemoteObjectReplica::State state, QRemoteObjectReplica::State oldState)
{
    qDebug() << "PanelUkccPlugin sourceStateChanged" << state;

    if(state == QRemoteObjectReplica::State::Suspect && oldState == QRemoteObjectReplica::State::Valid) {
        qWarning() << "Error  occurs after remote object initialized";
        m_mergeIcons->setEnabled(false);
//        m_mergeIcons->disconnect(); //fk ky-sdk
        m_mergeIcons->disconnect(this);
        m_panelLocation->setEnabled(false);
        m_panelLocation->disconnect();
        m_panelSizePolicy->setEnabled(false);
        m_panelSizePolicy->disconnect(this);
        m_panelAutoHide->setEnabled(false);
        m_panelAutoHide->disconnect(this);
        m_panelAutoHide->disconnect(m_configReplica);
        m_panelLock->setEnabled(false);
        m_panelLock->disconnect(this);
        m_panelLock->disconnect(m_configReplica);
        m_showTaskView->setEnabled(false);
        m_showTaskView->disconnect(this);
        m_showPanelOnAllScreens->disconnect(m_configReplica);
        m_showPanelOnAllScreens->disconnect(this);
        m_showPanelOnAllScreens->setEnabled(false);
        m_showSystemTrayOnAllPanels->disconnect(m_configReplica);
        m_showSystemTrayOnAllPanels->disconnect(this);
        m_showSystemTrayOnAllPanels->setEnabled(false);
        for(const QString &id : m_systemTrayIconId) {
            auto button = m_frame3->findChild<SwitchButton *>(id, Qt::FindDirectChildrenOnly);
            if(button) {
                button->setEnabled(false);
                button->disconnect(this);
            }
        }
    }
}
