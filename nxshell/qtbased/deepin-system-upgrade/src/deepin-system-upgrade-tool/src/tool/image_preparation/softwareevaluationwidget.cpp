    // SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DGuiApplicationHelper>

#include <QDir>
#include <QFile>

#include <DCheckBox>


#include "../../core/constants.h"
#include "../../core/dbusworker.h"
#include "../../core/utils.h"
#include "softwareevaluationwidget.h"
#include "../../widgets/iconlabel.h"

AppInfo readDesktopFile(const QString &filePath)
{
    QFile desktopFile(filePath);
    AppInfo info;
    info.noDisplay = false;
    QString name = "";
    QString fallbackName = "";
    if (desktopFile.open(QFile::ReadOnly | QFile::Text))
    {
        while (!desktopFile.atEnd())
        {
            QString line = desktopFile.readLine();
            if (line.startsWith("Icon"))
            {
                info.iconName = line.split("=")[1].trimmed();
            }
            else if (line.startsWith(QString("Name[%1]").arg(kCurrentLocale)) || line.startsWith("Name[zh]"))
            {
                name = line.split("=")[1].trimmed();
            }
            else if (line.startsWith("Name="))
            {
                fallbackName = line.split("=")[1].trimmed();
            }
            else if (line.startsWith("NoDisplay="))
            {
                info.noDisplay = line.split("=")[1].trimmed() == "true";
            }
        }
        desktopFile.close();
        info.name = name.length() > 0 ? name : fallbackName;
    }
    else
    {
        info.name = filePath;
    }
    return info;
}

SoftwareEvaluationWidget::SoftwareEvaluationWidget(QWidget *parent)
    :   DWidget(parent)
    ,   m_pkgDesktopMap(new QMap<QString, QStringList>())
    ,   m_powerTipLabel(new SimpleLabel)
    ,   m_mainLayout(new QVBoxLayout(this))
    ,   m_mainWidget(new QWidget(this))
    ,   m_softwareChangeTitleLabel(new DLabel(this))
    ,   m_softwareChangeTableWidget(new SoftwareTableWidget(this))
{
    initUI();
    initConnections();
}

void SoftwareEvaluationWidget::initConnections()
{
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, [this] {
        QPalette palette = this->palette();
        if (isDarkMode())
        {
            palette.setColor(QPalette::Base, "#FF232323");
        }
        else
        {
            palette.setColor(QPalette::Base, "#FFFFFFFF");
        }
        this->setPalette(palette);
    });
    connect(DBusWorker::getInstance(), &DBusWorker::AppsAvailable, [this] (const QMap<QString, QStringList> appSet){
        *m_pkgDesktopMap = appSet;
        qDebug() << "m_pkgDesktopMap:" << *m_pkgDesktopMap;
        qDebug() << "m_pkgDesktopMap size:" << m_pkgDesktopMap->size();
    });
    connect(DBusWorker::getInstance(), &DBusWorker::MigrateStatus, this, &SoftwareEvaluationWidget::updateAppInfos);
}

void SoftwareEvaluationWidget::initUI()
{
    QPalette palette = this->palette();

    if (isDarkMode())
    {
        palette.setColor(QPalette::Base, "#FF232323");
    }
    else
    {
        palette.setColor(QPalette::Base, "#FFFFFFFF");
    }
    this->setPalette(palette);

    m_softwareChangeTitleLabel->setText(tr("Evaluation Result"));
    m_softwareChangeTitleLabel->setForegroundRole(DPalette::TextTitle);
    DFontSizeManager::instance()->bind(m_softwareChangeTitleLabel, DFontSizeManager::T5, QFont::Bold);

    const QStringList softwareChangeHeaders = {tr("Compatible Apps"), tr("Incompatible Apps")};
    m_softwareChangeTableWidget->setHorizontalHeaderLabels(softwareChangeHeaders);
    m_softwareChangeTableWidget->horizontalHeader()->setForegroundRole(QPalette::Text);
    m_softwareChangeTableWidget->horizontalHeader()->setVisible(true);
    m_softwareChangeTableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    m_softwareChangeTableWidget->horizontalHeader()->setFixedHeight(36);
    m_softwareChangeTableWidget->setCornerButtonEnabled(true);
    m_softwareChangeTableWidget->setFixedSize(572, 200);

    QWidget *tipsMainWidget = new QWidget;
    QVBoxLayout * tipsMainLayout = new QVBoxLayout;
    QHBoxLayout * dataLabelLayout = new QHBoxLayout;
    QHBoxLayout * appLabelLayout = new QHBoxLayout;

    DLabel *tipsIconLabel = new DLabel;
    tipsIconLabel->setPixmap(QIcon::fromTheme("dialog-information").pixmap(QSize(24, 24)));
    tipsIconLabel->setFixedSize(24,24);
    DLabel *dataLabel = new DLabel(tr("The system upgrade will ensure the integrity of user data, and we will not delete user data"));
    DFontSizeManager::instance()->bind(dataLabel, DFontSizeManager::T8, QFont::Normal);
    DLabel *appLabel = new DLabel(tr("The cross-version upgrade does not migrate the application. After the upgrade is completed, it supports automatic installation of the selected compatible application"));
    DFontSizeManager::instance()->bind(appLabel, DFontSizeManager::T8, QFont::Normal);

    dataLabelLayout->addWidget(tipsIconLabel, Qt::AlignLeft);
    dataLabelLayout->addSpacing(0);
    dataLabelLayout->addWidget(dataLabel, Qt::AlignLeft);

    appLabelLayout->addWidget(appLabel);
    appLabelLayout->setContentsMargins(30, 0, 0, 0);

    tipsMainLayout->addLayout(dataLabelLayout);
    tipsMainLayout->addLayout(appLabelLayout);
    tipsMainLayout->setContentsMargins(0,0,0,0);

    tipsMainWidget->setLayout(tipsMainLayout);
    tipsMainWidget->setFixedSize(572, 50);

    DFontSizeManager::instance()->bind(m_powerTipLabel, DFontSizeManager::T8, QFont::Normal);
    m_powerTipLabel->setForegroundRole(DPalette::TextTips);
    m_powerTipLabel->setText(tr("The upgrade may take a long time. Please plug in to avoid interruption."));

    m_mainLayout->setContentsMargins(74, 0, 74, 0);
    m_mainLayout->addStretch(6);
    m_mainLayout->addWidget(m_softwareChangeTitleLabel, 0, Qt::AlignCenter);
    m_mainLayout->addStretch(2);
    m_mainLayout->addWidget(m_softwareChangeTableWidget, Qt::AlignCenter);
    m_mainLayout->addStretch(1);
    m_mainLayout->addWidget(tipsMainWidget);
    m_mainLayout->addStretch(2);
    m_mainLayout->addWidget(m_powerTipLabel, 0, Qt::AlignCenter);
    m_mainLayout->addStretch(2);
    setLayout(m_mainLayout);

    // It must be set to the same role as base table items, or, the table background will display the widgets at the bottom.
    setBackgroundRole(QPalette::Base);
}

IconLabel* generateCellWidget(const QString name, const QString iconPath)
{
    IconLabel *cellLabel = new IconLabel(nullptr);
    DLabel *nameLabel = new DLabel(name);
    DFontSizeManager::instance()->bind(nameLabel, DFontSizeManager::T8, QFont::Medium);
    nameLabel->setForegroundRole(DPalette::TextTitle);
    // Absolute icon file path
    if (iconPath.startsWith('/'))
    {
        cellLabel->setIcon(QIcon(iconPath).pixmap(24, 24));
    }
    // Icon name selected from themes
    else
    {
        cellLabel->setIcon(QIcon::fromTheme(iconPath, QIcon::fromTheme("application-x-executable")).pixmap(24, 24));
    }
    cellLabel->setIconSize({24, 24});
    cellLabel->addSpacing(10);
    cellLabel->addWidget(nameLabel);
    cellLabel->addStretch(1);
    return cellLabel;
}

void SoftwareEvaluationWidget::fillTable()
{
    m_softwareChangeTableWidget->setRowCount(m_compatAppInfos.size() > m_incompatAppInfos.size() ? m_compatAppInfos.size() : m_incompatAppInfos.size());
    for (unsigned long i = 0; i < m_compatAppInfos.size(); ++i) {
        AppInfo info = m_compatAppInfos[i];
        IconLabel *cellLabel = generateCellWidget(info.name, info.iconName);
        DCheckBox * checkBox = new DCheckBox(this);
        checkBox->setMaximumWidth(24);
        QWidget * tableItemWidget = new QWidget(this);
        QHBoxLayout * tableItemlayout = new QHBoxLayout;
        tableItemlayout->addWidget(checkBox, Qt::AlignCenter);
        tableItemlayout->addSpacing(0);
        tableItemlayout->addWidget(cellLabel, Qt::AlignCenter);
        tableItemlayout->setMargin(0);
        tableItemWidget->setLayout(tableItemlayout);
        m_softwareChangeTableWidget->setCellWidget(i, 0, tableItemWidget);
        connect(m_softwareChangeTableWidget->getHeaderView(), &BorderRadiusHeaderView::selectAllButtonClicked, this, [ckBox = checkBox] (bool clicked) {
               ckBox->setChecked(clicked);
        });

        connect(checkBox, &DCheckBox::stateChanged, this,[this, name = info.pkgName](int state) {
               onItemCheckboxClicked(state, name);
        });
        qDebug() << "compat app:" << info.name;
    }
    for (unsigned long i = 0; i < m_incompatAppInfos.size(); ++i) {
        AppInfo info = m_incompatAppInfos[i];
        IconLabel *cellLabel = generateCellWidget(info.name, info.iconName);
        m_softwareChangeTableWidget->setCellWidget(i, 1, cellLabel);
        qDebug() << "incompat app:" << info.name;
    }
}

void SoftwareEvaluationWidget::clearAppInfos()
{
    m_compatAppInfos.clear();
    m_incompatAppInfos.clear();
}

void SoftwareEvaluationWidget::updateAppInfos(const QString &pkg, int status)
{
    QDir usrShareDir = QDir("/usr/share");
    QDir iconDir = QDir(usrShareDir);
    iconDir.cd("icons");
    QDir appDir = QDir(usrShareDir);
    appDir.cd("applications");

    QIcon::setThemeSearchPaths(QStringList(iconDir.absolutePath()));
    if (m_pkgDesktopMap->contains(pkg))
    {
        for (QString desktopFilename: m_pkgDesktopMap->value(pkg))
        {
            AppInfo info = readDesktopFile(appDir.absoluteFilePath(desktopFilename));
            info.pkgName = pkg;
            qDebug() << "Adding desktop entry" << desktopFilename << "for" << pkg;
            if (status == 1)
                m_compatAppInfos.push_back(info);
            else
                m_incompatAppInfos.push_back(info);
        }
    }
}

void SoftwareEvaluationWidget::onItemCheckboxClicked(int state, const QString &infoName)
{
    if (state == Qt::Checked)
    {
        m_migrateLists.append(infoName);
    }
    else
    {
        m_migrateLists.removeOne(infoName);
    }
}

QStringList SoftwareEvaluationWidget::getMigratelist()
{
    return m_migrateLists;
}
