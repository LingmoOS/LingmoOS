// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scopewidget.h"
#include "global/builtinsearch.h"
#include "global/searchconfigdefine.h"
#include "business/config/searchconfig.h"
#include "switchwidget/switchwidget.h"
#include "gui/datadefine.h"
#include "utils/utils.h"

#include <DFontSizeManager>
#include <DGuiApplicationHelper>

#include <QLabel>
#include <QFont>
#include <QDir>
#include <QSettings>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

ScopeWidget::ScopeWidget(QWidget *parent)
    : DWidget(parent)
{
    m_groupLabel = new QLabel(tr("Search contents"), this);
    DFontSizeManager::instance()->bind(m_groupLabel, DFontSizeManager::T5, QFont::Bold);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    m_mainLayout->addWidget(m_groupLabel);

    m_groupName = { {GRANDSEARCH_GROUP_APP, GroupName_App}
                    , {GRANDSEARCH_GROUP_SETTING, GroupName_Setting}
                    , {GRANDSEARCH_GROUP_FILE_VIDEO, GroupName_Video}
                    , {GRANDSEARCH_GROUP_FILE_AUDIO, GroupName_Audio}
                    , {GRANDSEARCH_GROUP_FILE_PICTURE, GroupName_Picture}
                    , {GRANDSEARCH_GROUP_FILE_DOCUMNET, GroupName_Document}
                    , {GRANDSEARCH_GROUP_FOLDER, GroupName_Folder}
                    , {GRANDSEARCH_GROUP_FILE, GroupName_File}
                    , {GRANDSEARCH_GROUP_WEB, GroupName_Web}
                  };

    QStringList displayOrder;
    displayOrder << GRANDSEARCH_GROUP_FILE << GRANDSEARCH_GROUP_FOLDER << GRANDSEARCH_GROUP_FILE_DOCUMNET
                 << GRANDSEARCH_GROUP_APP << GRANDSEARCH_GROUP_SETTING << GRANDSEARCH_GROUP_FILE_VIDEO
                 << GRANDSEARCH_GROUP_FILE_AUDIO << GRANDSEARCH_GROUP_FILE_PICTURE << GRANDSEARCH_GROUP_WEB;

    m_displayIcons = QStringList{"filesearch", "foldersearch", "documentsearch", "appsearch", "settingsearch"
                             , "videosearch", "audiosearch", "picturesearch", "websearch"};

    // 如果控制中心提供的设置搜索插件不存在，则从配置界面删除
    if (!isValid(GRANDSEARCH_CLASS_SETTING_CONTROLCENTER)) {
        m_groupName.remove(GRANDSEARCH_GROUP_SETTING);
        displayOrder.removeOne(GRANDSEARCH_GROUP_SETTING);
        m_displayIcons.removeOne("settingsearch");
    }

    Q_ASSERT(displayOrder.count() == m_displayIcons.count());

    for (int i = 0; i < displayOrder.count(); ++i) {
        const QString &serachGroupy = displayOrder.at(i);
        bool enable = SearchConfig::instance()->getConfig(GRANDSEARCH_SEARCH_GROUP, serachGroupy, true).toBool();
        SwitchWidget *switchWidget = new SwitchWidget(this);
        switchWidget->setFixedSize(SWITCHWIDGETWIDTH, SWITCHWIDGETHEIGHT);
        switchWidget->setEnableBackground(true);
        switchWidget->setTitle(m_groupName.value(serachGroupy));
        switchWidget->setChecked(enable);
        switchWidget->setProperty(GRANDSEARCH_SEARCH_GROUP, serachGroupy);

        m_mainLayout->addWidget(switchWidget);
        m_switchWidgets.append(switchWidget);

        connect(switchWidget, &SwitchWidget::checkedChanged, this, &ScopeWidget::onSwitchStateChanged);
    }

    setLayout(m_mainLayout);
    updateIcons();

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &ScopeWidget::updateIcons);
}

ScopeWidget::~ScopeWidget()
{
    m_switchWidgets.clear();
}

void ScopeWidget::onSwitchStateChanged(const bool checked)
{
    QObject *obj = sender();
    SwitchWidget *switchWidget = static_cast<SwitchWidget *>(obj);

    if (switchWidget) {
        QString group = switchWidget->property(GRANDSEARCH_SEARCH_GROUP).toString();
        SearchConfig::instance()->setConfig(GRANDSEARCH_SEARCH_GROUP, group, checked);
    }
}

void ScopeWidget::updateIcons()
{
    Q_ASSERT(m_switchWidgets.count() == m_displayIcons.count());

    QString suffix = Utils::iconThemeSuffix();

    for (int i = 0; i < m_switchWidgets.count(); ++i) {

        QString iconName = m_displayIcons.at(i);
        QIcon icon = QIcon(QString(":/icons/%1%2.svg").arg(iconName).arg(suffix));

        auto switchWidget = m_switchWidgets.at(i);
        Q_ASSERT(switchWidget);

        switchWidget->setIcon(icon, QSize(SWITCHWIDGETICONSIZE, SWITCHWIDGETICONSIZE));
    }
    update();
}

bool ScopeWidget::isValid(const QString &item)
{
    if (item.isEmpty())
        return false;

    //默认路径
#ifdef QT_DEBUG
    char path[PATH_MAX] = {0};
    const char *defaultPath = realpath("../grand-search-daemon/", path);
#else
    auto defaultPath = PLUGIN_SEARCHER_DIR;
#endif
    static_assert(std::is_same<decltype(defaultPath), const char *>::value, "PLUGIN_SEARCHER_DIR is not a string.");

    QDir dir(defaultPath);

    auto entrys = dir.entryInfoList({"*.conf"}, QDir::Files, QDir::Name);
    for (const QFileInfo &entry : entrys) {
        QSettings conf(entry.absoluteFilePath(), QSettings::IniFormat);
        //插件名称
        QString name = conf.value("Grand Search/Name", "").toString();
        if (item == name)
            return true;
    }

    return false;
}
