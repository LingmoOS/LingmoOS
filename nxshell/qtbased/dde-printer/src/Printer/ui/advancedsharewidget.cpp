// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "advancedsharewidget.h"
#include "common.h"

#include <DGuiApplicationHelper>
#include <DFontSizeManager>
#include <DApplicationHelper>
#include <DPalette>
#include <DTitlebar>

#include <QHBoxLayout>
#include <QPainter>
#include <QFile>
#include <QProcess>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

static QString getInfoFromDesktopFile(const QString &name, const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }

    QTextStream in(&file);
    QString execPath;

    QString mateName = name + '=';

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.startsWith(mateName)) {
            execPath = line.mid(mateName.length());
            break;
        }
    }

    file.close();
    return execPath;
}

static QStringList getDesktopFilePath(const QString &packageName)
{
    QStringList ret;
    QString cmd = QString("dpkg -L %1 | grep -E 'applications.*desktop'").arg(packageName);

    QProcess p;
    p.start("/bin/bash", QStringList() << "-c" << cmd);
    p.waitForFinished();

    QString output = p.readAllStandardOutput();
    ret = output.split("\n", QString::SkipEmptyParts);

    return ret;
}

AdvanceShareWidget::AdvanceShareWidget(QWidget *parent)
    : QWidget(parent)
    , m_textLabel(new QLabel)
    , m_enterIcon(new QLabel)
{
    setMinimumHeight(36);
    updateIcon();
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &AdvanceShareWidget::updateIcon);
    m_enterIcon->setFixedSize(16, 16);
    m_textLabel->setContentsMargins(0, 0, 0, 0);
    DFontSizeManager::instance()->bind(m_textLabel, DFontSizeManager::T6, QFont::Medium);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_textLabel);
    mainLayout->addWidget(m_enterIcon);
    mainLayout->setContentsMargins(10, 0, 10, 0);
    
    setLayout(mainLayout);
}

void AdvanceShareWidget::setLabelText(const QString &text)
{
    m_textLabel->setText(text);
}

void AdvanceShareWidget::click()
{
    emit clicked();
}

void AdvanceShareWidget::enterEvent(QEvent *event)
{
    QWidget::enterEvent(event);
    m_hover = true;
    update();
}

void AdvanceShareWidget::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    m_hover = false;
    update();
}

void AdvanceShareWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
    emit clicked();
}

void AdvanceShareWidget::updateIcon()
{
    if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
        m_enterIcon->setPixmap(QPixmap(":/images/enter_details_normal.svg"));
    } else {
        m_enterIcon->setPixmap(QPixmap(":/images/enter_details_normal-dark.svg"));
    }

    update();
}

void AdvanceShareWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    const DPalette &dp = DApplicationHelper::instance()->palette(this);
    QPainter p(this);
    p.setPen(Qt::NoPen);
    p.setBrush(dp.brush(DPalette::ItemBackground));
    p.drawRoundedRect(rect(), 8, 8);
    QWidget::paintEvent(event);
}

ManagementToolWidget::ManagementToolWidget(QWidget *parent) : DMainWindow(parent)
{
    initUI();
    initConnections();
}

ManagementToolWidget::~ManagementToolWidget()
{
}

void ManagementToolWidget::initUI()
{
    titlebar()->setMenuVisible(false);
    titlebar()->setTitle("");
    titlebar()->setIcon(QIcon(":/images/dde-printer.svg"));

    setWindowFlags(Qt::Dialog);
    setWindowModality(Qt::ApplicationModal);
    setFixedSize(520, 317);

    QLabel *titleLabel = new QLabel(tr("Management Tools"));
    titleLabel->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout *pHLayout = new QHBoxLayout();
    pHLayout->addStretch();
    pHLayout->addWidget(titleLabel);
    pHLayout->addStretch();

    m_pDeviceList = new DListView(this);
    m_pDeviceList->setEditTriggers(DListView::NoEditTriggers);
    m_pDeviceList->setTextElideMode(Qt::ElideRight);
    m_pDeviceList->setSelectionMode(QAbstractItemView::NoSelection);
    m_pDeviceList->setFocusPolicy(Qt::NoFocus);
    m_pDeviceList->setItemSpacing(10);
    m_pDeviceList->setIconSize(QSize(24, 24));

    QStandardItemModel *adminToolModel = new QStandardItemModel(m_pDeviceList);
    m_packageInfo.clear();
    getPackageInfo("com.hp.hplip");

    for (int i = 0; i < m_packageInfo.size(); ++i) {
        DStandardItem *item = new DStandardItem();
        item->setIcon(QIcon::fromTheme(m_packageInfo.at(i).strIcon));
        if (QLocale::system().language() == QLocale::Chinese) {
            item->setText(m_packageInfo.at(i).strNameCn);
        } else {
            item->setText(m_packageInfo.at(i).strName);
        }

        item->setSizeHint(QSize(200, 54));
        adminToolModel->appendRow(item);
    }

    m_pDeviceList->setModel(adminToolModel);

    QVBoxLayout *pvLayout = new QVBoxLayout();
    pvLayout->addLayout(pHLayout);
    pvLayout->addSpacing(17);
    pvLayout->addWidget(m_pDeviceList);

    QWidget *pWidget = new QWidget;
    pWidget->setLayout(pvLayout);
    pWidget->setContentsMargins(12, 0, 12, 0);

    setCentralWidget(pWidget);
}

void ManagementToolWidget::initConnections()
{
    connect(m_pDeviceList, &DListView::clicked, this, &ManagementToolWidget::listClickedSlot);
}

void ManagementToolWidget::listClickedSlot(const QModelIndex &index)
{
    if (index.row() >= m_packageInfo.size()) {
        return;
    }

    QProcess::startDetached(m_packageInfo.at(index.row()).strExec);
}

void ManagementToolWidget::getPackageInfo(const QString &packageName)
{
    if (isPackageInstalled(packageName)) {
        QStringList ret = getDesktopFilePath(packageName);

        QStringList::iterator it;
        for (it = ret.begin(); it != ret.end(); ++it) {
            PACKAGEINFO_S info;
            QString exec = "Exec", icon = "Icon", name = "Name" , namzh = "Name[zh_CN]";
            if ((*it).contains("scan")) continue;

            info.strExec = getInfoFromDesktopFile(exec, *it);
            info.strIcon = getInfoFromDesktopFile(icon, *it);
            info.strName = getInfoFromDesktopFile(name, *it);
            info.strNameCn = getInfoFromDesktopFile(namzh, *it);

            m_packageInfo.append(info);
        }
    }
}

void ManagementToolWidget::closeEvent(QCloseEvent *event)
{
    emit finished();
    event->accept();
}

int ManagementToolWidget::exec()
{
    show();
    QEventLoop loop;
    connect(this, &ManagementToolWidget::finished, &loop, &QEventLoop::quit);
    loop.exec();
    return 1;
}
