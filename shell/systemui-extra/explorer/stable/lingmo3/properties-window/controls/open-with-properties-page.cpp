/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Wenfei He <hewenfei@kylinos.cn>
 *
 */

#include "open-with-properties-page.h"
#include "file-info.h"
#include "file-launch-manager.h"
#include "global-settings.h"

#include <QPushButton>
#include <QFrame>
#include <QFileInfo>
#include <QDebug>

using namespace Peony;

OpenWithPropertiesPage::OpenWithPropertiesPage(const QString &uri, QWidget *parent) : PropertiesWindowTabIface(parent)
{
    m_fileInfo = FileInfo::fromUri(uri);
    FileInfoJob *job = new FileInfoJob(m_fileInfo);
    job->setAutoDelete(true);
    connect(job, &FileInfoJob::queryAsyncFinished, this, &OpenWithPropertiesPage::init);
    job->queryAsync();
}

void OpenWithPropertiesPage::init()
{
    if (m_futureWatcher) {
        delete m_futureWatcher;
        m_futureWatcher = nullptr;
    }
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0,0,0,0);

    this->initFloorOne();
    this->addSeparator();

    this->initFloorTwo();
    this->addSeparator();

    this->initFloorThree();

    this->setLayout(m_layout);
}

OpenWithPropertiesPage::~OpenWithPropertiesPage()
{

}

void OpenWithPropertiesPage::saveAllChange()
{
    if (!m_thisPageChanged)
        return;

    if (m_newAction) {
        FileLaunchManager::setDefaultLauchAction(m_fileInfo.get()->uri(), m_newAction);
    }
}

void OpenWithPropertiesPage::initFloorOne()
{
    QFrame      *floor1  = new QFrame(this);
    QVBoxLayout *layout1 = new QVBoxLayout(this);
    layout1->setContentsMargins(22,0,22,0);

    floor1->setLayout(layout1);
    floor1->setMaximumHeight(142);

    //这几个组件的最小高度可能让人迷糊，这么设置的原因是因为设计图有一定的区域留空，而label可以自适应高度，
    //利用这个特性可以将留空的高度全部设置给label使得不用设置margin。(setContentsMargins)
    QLabel *targetTypeMsgLabel = new QLabel(floor1);
    targetTypeMsgLabel->setMinimumHeight(60);
    qDebug() << "targetTypeMsgLabel :" << targetTypeMsgLabel->height();
    targetTypeMsgLabel->setText(tr("How do you want to open \"%1%2\" files ?").arg(".").arg(m_fileInfo.get()->displayName().split(".").last()));
    layout1->addWidget(targetTypeMsgLabel);

    QLabel *defaultOpenLabel = new QLabel(floor1);
    defaultOpenLabel->setMinimumHeight(35);
    defaultOpenLabel->setText(tr("Default open with:"));
    layout1->addWidget(defaultOpenLabel);

    m_defaultOpenWithWidget = OpenWithPropertiesPage::createDefaultOpenWithWidget(m_fileInfo->uri(), floor1);
    m_defaultOpenWithWidget->setMinimumHeight(45);
    layout1->addWidget(m_defaultOpenWithWidget);

    m_layout->addWidget(floor1);
}

void OpenWithPropertiesPage::initFloorTwo()
{
    QFrame      *floor2  = new QFrame(this);
    QVBoxLayout *layout2 = new QVBoxLayout(this);
    floor2->setLayout(layout2);
    floor2->setMaximumHeight(226);
    layout2->setContentsMargins(0,0,0,0);

    QLabel *otherOpenLabel = new QLabel(floor2);
    otherOpenLabel->setText(tr("Other:"));
    otherOpenLabel->setContentsMargins(22,0,0,0);
    layout2->addWidget(otherOpenLabel);

    m_launchHashList = new LaunchHashList(m_fileInfo.get()->uri(), floor2);
    //自定义列表属性
    m_launchHashList->m_actionList->setFrameShape(QListWidget::NoFrame);
    m_launchHashList->m_actionList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    m_launchHashList->m_actionList->setContentsMargins(22,0,0,0);
    m_launchHashList->m_actionList->setFixedHeight(180);
    m_launchHashList->m_actionList->setAlternatingRowColors(true);
    m_launchHashList->m_actionList->setIconSize(QSize(44, 44));
//    m_launchHashList->m_actionList->setStyleSheet("QListWidget::Item{padding-left:22px;}");
    m_launchHashList->m_actionList->setStyleSheet("QListWidget::Item{margin-left:22px;}");
    //m_newAction
    connect(m_launchHashList->m_actionList, &QListWidget::currentItemChanged, [=](QListWidgetItem *current) {
        if (!current)
            return ;
        FileLaunchAction *action = m_launchHashList->m_actionHash->value(current);
        if (action == FileLaunchManager::getDefaultAction(m_fileInfo.get()->uri()))
            return ;

        this->m_newAction = action;
        this->thisPageChanged();
    });

    layout2->addWidget(m_launchHashList->m_actionList);
    this->m_layout->addWidget(floor2);

}

void OpenWithPropertiesPage::initFloorThree()
{
    QFrame      *floor3  = new QFrame(this);
    QVBoxLayout *layout3 = new QVBoxLayout(this);
    floor3->setLayout(layout3);
    floor3->setMaximumHeight(122);
    layout3->setContentsMargins(22,0,0,0);

    QString str1;
    str1 = "<a href=\"lingmo-software-center\" style=\"color: #3D6BE5;text-underline: none;\">"
          + tr("Choose other application")
          + "</a>";
    QLabel *allOpenLabel = new QLabel(str1, floor3);
    allOpenLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    connect(allOpenLabel, &QLabel::linkActivated, this, [=]() {
        AllFileLaunchDialog dialog(m_fileInfo.get()->uri());
        if (QDialog::Accepted == dialog.exec()) {
            m_defaultOpenWithWidget->setLaunchAction(FileLaunchManager::getDefaultAction(m_fileInfo->uri()));
        }
    });

    layout3->addWidget(allOpenLabel);

    QString str2;
    str2 = "<a href=\"lingmo-software-center\" style=\"color: #3D6BE5;text-underline: none;\">"
          + tr("Go to application center")
          + "</a>";
    QLabel *otherOpenLabel = new QLabel(str2, floor3);
    otherOpenLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    connect(otherOpenLabel, &QLabel::linkActivated, this, [=]() {
        QtConcurrent::run([=]() {
            QProcess p;
            if (COMMERCIAL_VERSION)
                p.setProgram("/usr/bin/lingmo-software-center");
            else
                p.setProgram("/usr/bin/ubuntu-lingmo-software-center");

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
            p.startDetached();
#else
            if (COMMERCIAL_VERSION)
                p.startDetached("/usr/bin/lingmo-software-center");
            else
                p.startDetached("/usr/bin/ubuntu-lingmo-software-center");

#endif
        });
    });

    bool isVisible = false;
    if(QFileInfo::exists("/usr/bin/lingmo-software-center")
            || QFileInfo::exists("/usr/bin/ubuntu-lingmo-software-center")){
        isVisible = true;
    }

    otherOpenLabel->setVisible(isVisible);
    layout3->addWidget(otherOpenLabel);
    layout3->addStretch(1);

    this->m_layout->addWidget(floor3);
}

NewFileLaunchDialog::NewFileLaunchDialog(const QString &uri, QWidget *parent) : QDialog(parent)
{
    m_launchHashList = new LaunchHashList(uri, this);
    m_layout = new QVBoxLayout(this);
    setLayout(m_layout);
    setWindowTitle(tr("Choose new application"));

    m_layout->addWidget(new QLabel(tr("Choose an Application to open this file"), this));

    m_launchHashList->m_actionList->setIconSize(QSize(48, 48));
    m_launchHashList->m_actionList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_layout->addWidget(m_launchHashList->m_actionList, 1);

    m_layout->addWidget(new QLabel(tr("apply now"), this));
    m_button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,this);

    m_layout->addWidget(m_button_box);

    //add button translate
    m_button_box->button(QDialogButtonBox::Ok)->setText(tr("OK"));
    m_button_box->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

    connect(this, &QDialog::accepted, [=]() {
        if (!m_launchHashList->m_actionList->currentItem())
            return ;
        FileLaunchAction *action = m_launchHashList->m_actionHash->value(m_launchHashList->m_actionList->currentItem());
        if (action) {
            FileLaunchManager::setDefaultLauchAction(uri, action);
        }
    });

    connect(m_button_box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

NewFileLaunchDialog::~NewFileLaunchDialog()
{
    if (m_launchHashList) {
        delete m_launchHashList;
        m_launchHashList = nullptr;
    }
}

DefaultOpenWithWidget* OpenWithPropertiesPage::createDefaultOpenWithWidget(const QString &uri, QWidget *parent)
{
    DefaultOpenWithWidget* defaultOpenWithWidget = new DefaultOpenWithWidget(parent);

    defaultOpenWithWidget->setLaunchAction(FileLaunchManager::getDefaultAction(uri));

    return defaultOpenWithWidget;
}

LaunchHashList::LaunchHashList(const QString &uri, QWidget *parent)
{
    m_actionHash = new QHash<QListWidgetItem*,FileLaunchAction*>();
    m_actionList = new QListWidget(parent);
    auto allLaunchActions = FileLaunchManager::getRecommendActions(uri);

    if (allLaunchActions.count() >= 1) {
        auto defaultLaunchAction = FileLaunchManager::getDefaultAction(uri);
        auto defaultItem = new QListWidgetItem(!defaultLaunchAction->icon().isNull()? defaultLaunchAction->icon() : QIcon::fromTheme("application-x-desktop"),
                                        defaultLaunchAction->text(),
                                        m_actionList);
        m_actionList->addItem(defaultItem);
        m_actionList->setCurrentItem(defaultItem);
        //NOTE:是否需要在列表中显示默认的打开方式 - Do you need to display the default opening method in the list
//        m_actionList->setItemHidden(defaultItem, true);
        for (auto action : allLaunchActions) {
            if (action->getAppInfoDisplayName() == defaultLaunchAction->getAppInfoDisplayName())
                continue;

            auto item = new QListWidgetItem(!action->icon().isNull()? action->icon(): QIcon::fromTheme("application-x-desktop"),
                                            action->text(),
                                            m_actionList);

            action->setParent(m_actionList);
            m_actionList->addItem(item);
            m_actionHash->insert(item, action);
        }
    }
}

LaunchHashList *LaunchHashList::getAllLaunchHashList(const QString &uri, QWidget *parent)
{
    LaunchHashList *launchHashList = new LaunchHashList();

    launchHashList->m_actionHash = new QHash<QListWidgetItem*,FileLaunchAction*>();
    launchHashList->m_actionList = new QListWidget(parent);
    auto allLaunchActions = FileLaunchManager::getAllActions(uri);

    if (allLaunchActions.count() >= 1) {
        for (auto action : allLaunchActions) {
            auto item = new QListWidgetItem(!action->icon().isNull()? action->icon(): QIcon::fromTheme("application-x-desktop"),
                                            action->text(),
                                            launchHashList->m_actionList);

            action->setParent(launchHashList->m_actionList);
            launchHashList->m_actionHash->insert(item, action);
            launchHashList->m_actionList->addItem(item);
        }
    }

    return launchHashList;
}

LaunchHashList::LaunchHashList()
{

}

LaunchHashList::~LaunchHashList()
{
    if (m_actionHash) {
        for (auto key : m_actionHash->keys()) {
            delete m_actionHash->value(key);
        }

        delete m_actionHash;
    }
    if (m_actionHash)
        m_actionList->deleteLater();
}

AllFileLaunchDialog::AllFileLaunchDialog(const QString &uri, QWidget *parent) : QDialog(parent)
{
    m_launchHashList = LaunchHashList::getAllLaunchHashList(uri, this);
    m_layout = new QVBoxLayout(this);
    setLayout(m_layout);
    setWindowTitle(tr("Choose new application"));

    m_layout->addWidget(new QLabel(tr("Choose an Application to open this file"), this));

    m_launchHashList->m_actionList->setIconSize(QSize(48, 48));
    m_launchHashList->m_actionList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_layout->addWidget(m_launchHashList->m_actionList, 1);

    m_layout->addWidget(new QLabel(tr("apply now"), this));
    m_button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,this);

    m_layout->addWidget(m_button_box);

    //add button translate
    m_button_box->button(QDialogButtonBox::Ok)->setText(tr("OK"));
    m_button_box->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

    connect(this, &QDialog::accepted, [=]() {
        if (!m_launchHashList->m_actionList->currentItem())
            return ;
        FileLaunchAction *action = m_launchHashList->m_actionHash->value(m_launchHashList->m_actionList->currentItem());
        if (action) {
            FileLaunchManager::setDefaultLauchAction(uri, action);
        }
    });

    connect(m_button_box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

AllFileLaunchDialog::~AllFileLaunchDialog()
{
    if (m_launchHashList) {
        delete m_launchHashList;
        m_launchHashList = nullptr;
    }
}

//默认打开方式组件
DefaultOpenWithWidget::DefaultOpenWithWidget(QWidget *parent) : QWidget(parent)
{
    m_appIconLabel = new QLabel(this);
    m_appNameLabel = new QLabel(this);

    //设置绝对宽度解决在一定概率下因为图标过大导致appName只剩 ‘...’ 问题
    m_appIconLabel->setFixedWidth(32);

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setAlignment(Qt::AlignVCenter);

    m_layout->addWidget(m_appIconLabel, 1);
    m_layout->addWidget(m_appNameLabel, 9);
}

void DefaultOpenWithWidget::setAppName(QString appName)
{
    if (appName.isNull()) {
        this->m_appNameLabel->setText(tr("No default app"));
        this->m_appIconLabel->setFixedWidth(0);
        this->m_layout->setSpacing(0);
    } else {
        this->m_appNameLabel->setText(appName);
        this->m_appNameLabel->setToolTip(appName);
    }
}

void DefaultOpenWithWidget::setAppIcon(QIcon appIcon)
{
    if (appIcon.isNull()) {
        this->m_appIconLabel->setPixmap(QIcon::fromTheme("application-x-desktop").pixmap(24, 24));
    } else {
        this->m_appIconLabel->setPixmap(appIcon.pixmap(24, 24));
    }
}

void DefaultOpenWithWidget::resizeEvent(QResizeEvent *event)
{
    //m_appIconLabel->maximumWidth() = 32px;
    int width = this->width() - m_appIconLabel->maximumWidth();

    if (m_appNameLabel->fontMetrics().width(m_appNameLabel->text()) > width) {
        m_appNameLabel->setText(m_appNameLabel->fontMetrics().elidedText(m_appNameLabel->text(), Qt::ElideRight, width));
    }

    QWidget::resizeEvent(event);
}

FileLaunchAction* DefaultOpenWithWidget::getLaunchAction()
{
    return m_launchAction;
}

void DefaultOpenWithWidget::setLaunchAction(FileLaunchAction* launchAction)
{
    if (m_launchAction) {
        delete m_launchAction;
        m_launchAction = nullptr;
    }

    if (launchAction) {
        this->m_launchAction = launchAction;

        this->setAppIcon(m_launchAction->icon());
        this->setAppName(m_launchAction->text());
    } else {
        this->setAppIcon(QIcon());
        this->setAppName(QString());
    }
}

DefaultOpenWithWidget::~DefaultOpenWithWidget()
{
    if (m_launchAction) {
        delete m_launchAction;
        m_launchAction = nullptr;
    }
}
