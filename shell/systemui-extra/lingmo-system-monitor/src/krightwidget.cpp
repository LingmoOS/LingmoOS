/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "krightwidget.h"
#include "macro.h"

#include <QProcess>
#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>
#include <QKeyEvent>
#include <QDebug>
#include <QDBusInterface>
#include <QDBusMessage>
#include "util.h"

QString KRightWidget::searchText = "";

KRightWidget::KRightWidget(QWidget *parent)
    : QWidget(parent)
{
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->installEventFilter(this);
    initUI();
    initConnections();
}

KRightWidget::~KRightWidget()
{
    if (m_searchTimer) {
        disconnect(m_searchTimer, SIGNAL(timeout()), this, SLOT(onRefreshSearchResult()));
        if(m_searchTimer->isActive()) {
            m_searchTimer->stop();
        }
    }
    if (m_styleSettings) {
        delete m_styleSettings;
        m_styleSettings = nullptr;
    }
}

void KRightWidget::initUI()
{
    // 初始化布局
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setContentsMargins(0,0,0,0);
    m_mainLayout->setSpacing(16);
    m_titleLayout = new QHBoxLayout();
    m_titleLayout->setContentsMargins(0,0,0,0);
    m_titleLayout->setSpacing(0);
    m_searchLayout = new QHBoxLayout();
    m_searchLayout->setContentsMargins(0,0,0,0);
    m_searchLayout->setSpacing(0);

    // 初始化控件
    m_btnOption  = new QToolButton(this);
    m_btnOption->setToolTip(tr("Option"));
    m_btnOption->setProperty("isWindowButton", 0x01);
    m_btnOption->setProperty("useIconHighlightEffect", 0x2);
    m_btnOption->setIcon(QIcon::fromTheme("open-menu-symbolic"));
    m_btnOption->setAutoRaise(true);
    m_btnMin      = new QToolButton(this);
    m_btnMin->setToolTip(tr("Minimize"));
    m_btnMin->setProperty("isWindowButton", 0x01);
    m_btnMin->setProperty("useIconHighlightEffect", 0x2);
    m_btnMin->setAutoRaise(true);
    m_btnMin->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    m_btnMax      = new QToolButton(this);
    m_btnMax->setToolTip(tr("Maximize"));
    m_btnMax->setProperty("isWindowButton", 0x01);
    m_btnMax->setProperty("useIconHighlightEffect", 0x2);
    m_btnMax->setAutoRaise(true);
    m_btnMax->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
    m_btnClose    = new QToolButton(this);
    m_btnClose->setToolTip(tr("Close"));
    m_btnClose->setProperty("isWindowButton", 0x02);
    m_btnClose->setProperty("useIconHighlightEffect", 0x08);
    m_btnClose->setAutoRaise(true);
    m_btnClose->setIcon(QIcon::fromTheme("window-close-symbolic"));

    m_mainMenu = new QMenu(this);
    m_mainMenu->setMinimumWidth(160);
    m_mainMenu->addAction(tr("Help"));
    m_mainMenu->addAction(tr("About"));
    m_mainMenu->addAction(tr("Exit"));
    m_btnOption->setMenu(m_mainMenu);
    m_btnOption->setPopupMode(QToolButton::InstantPopup);

    connect(m_mainMenu,&QMenu::triggered,this,[=](QAction *action){
        qDebug() << Q_FUNC_INFO << action->text();
        if(action->text() == tr("About")){
            KAboutDialog *k_aboutDialog = new KAboutDialog(this);
            k_aboutDialog->setAppIcon(QIcon::fromTheme("lingmo-system-monitor"));
            k_aboutDialog->setWindowTitle(tr("Lingmo System Monitor"));
            k_aboutDialog->setAppName(tr("Lingmo System Monitor"));
            k_aboutDialog->setAppVersion(getUsmVersion());
            k_aboutDialog->setAppSupport(k_aboutDialog->appSupport());
            k_aboutDialog->exec();
        }else if(action->text() == tr("Help")){
            QString service_name = "com.lingmoUserGuide.hotel_" + QString::number(getuid());
            QDBusInterface *interface = new QDBusInterface(service_name,
                                                           LINGMO_USER_GUIDE_PATH,
                                                           LINGMO_USER_GUIDE_INTERFACE,
                                                           QDBusConnection::sessionBus(),
                                                           this);
            QDBusMessage msg = interface->call("showGuide", "lingmo-system-monitor");
        }else if(action->text() == tr("Exit")){
            qApp->exit();
        }
    });

    // 搜索框
    m_searchEditNew = new KSearchLineEdit();
    m_searchEditNew->setClearButtonEnabled(true);
    m_searchEditNew->setFixedSize(SPECIALWIDTH, NORMALHEIGHT);
    m_searchEditNew->installEventFilter(this);

    m_searchTimer = new QTimer(this);
    m_searchTimer->setSingleShot(true);
    m_searchLayout->addWidget(m_searchEditNew, 0, Qt::AlignHCenter);

    QString language = QLocale::system().name();
    // 维吾尔 哈萨克斯 柯尔克孜 语言控件翻转
    if (language == "ug_CN" || language == "kk_KZ" || language == "ky_KG") {
        m_titleLayout->setContentsMargins(0,4,4,140);
    } else {
        m_titleLayout->setContentsMargins(140,4,4,0);
    }

    m_titleLayout->setSpacing(4);
    m_titleLayout->addLayout(m_searchLayout);
    m_titleLayout->addWidget(m_btnOption);
    m_titleLayout->addWidget(m_btnMin);
    m_titleLayout->addWidget(m_btnMax);
    m_titleLayout->addWidget(m_btnClose);

    m_titleWidget = new QWidget();
    QVBoxLayout *titleVLayout = new QVBoxLayout();
    titleVLayout->setContentsMargins(0,0,0,0);
    titleVLayout->setSpacing(0);
    titleVLayout->addLayout(m_titleLayout);
    titleVLayout->addSpacing(6);
    m_titleWidget->setLayout(titleVLayout);

    m_mainLayout->addWidget(m_titleWidget);
    m_stackedWidget = new QStackedWidget();
    m_mainLayout->addWidget(m_stackedWidget);

    this->setLayout(m_mainLayout);
}

void KRightWidget::addPanel(QWidget* pWidget, QString strName, QString strIcon, int nPanelIdx)
{
    Q_UNUSED(strName);
    Q_UNUSED(strIcon);
    if (!pWidget)
        return;
    int nIndex = m_stackedWidget->indexOf(pWidget);
    if (nIndex == -1) { // statckedwidget not contains pWidget
        if (nPanelIdx >= 0) {
            m_stackedWidget->insertWidget(nPanelIdx, pWidget);
        } else {
            m_stackedWidget->addWidget(pWidget);
        }
    }
}

void KRightWidget::initConnections()
{
    connect(m_btnClose, SIGNAL(clicked()), this, SLOT(onCloseBtnClicked()));
    connect(m_btnMin, SIGNAL(clicked()), this, SLOT(onMinBtnClicked()));
    connect(m_btnMax, SIGNAL(clicked()), this, SLOT(onMaxBtnClicked()));
    connect(m_searchTimer, SIGNAL(timeout()), this, SLOT(onRefreshSearchResult()));
    connect(m_searchEditNew,&QLineEdit::textChanged,this,&KRightWidget::handleSearchTextChanged,Qt::QueuedConnection);
}

void KRightWidget::createAboutDialog()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_aboutDlg = new USMAboutDialog(this);

    QApplication::restoreOverrideCursor();
}

void KRightWidget::onMinBtnClicked()
{
    emit minimizeWindow();
}

void KRightWidget::onMaxBtnClicked()
{
    emit maximizeWindow();
}

void KRightWidget::onCloseBtnClicked()
{
    window()->close();
}

void KRightWidget::onUpdateMaxBtnState()
{
    if (m_btnMax) {
        if(window()->isMaximized()) {
            m_btnMax->setIcon(QIcon::fromTheme("window-restore-symbolic"));
             m_btnMax->setToolTip(tr("Restore"));
        } else {
            m_btnMax->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
             m_btnMax->setToolTip(tr("Maximize"));
        }
    }
}

void KRightWidget::updateMaxBtn(bool max)
{
    if (max) {
        m_btnMax->setIcon(QIcon::fromTheme("window-restore-symbolic"));
        m_btnMax->setToolTip(tr("Restore"));
    } else {
        m_btnMax->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
        m_btnMax->setToolTip(tr("Maximize"));
    }
}

void KRightWidget::paintEvent(QPaintEvent *)
{
    QPainterPath path;

    QPainter painter(this);
    painter.setOpacity(1);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setClipping(true);
    painter.setPen(Qt::transparent);

    path.addRect(this->rect());
    path.setFillRule(Qt::WindingFill);
    painter.setBrush(this->palette().base());
    painter.setPen(Qt::transparent);

    painter.drawPath(path);
    QStyleOption opt;
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}

void KRightWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
}

bool KRightWidget::eventFilter(QObject *obj, QEvent *event)    //set the esc and tab pressbutton effect
{

    return QWidget::eventFilter(obj, event);
}

void KRightWidget::onSwichPanel(int nIndex)
{
    if (nIndex >= m_stackedWidget->count())
        return;
    m_searchEditNew->clear();
    m_searchEditNew->clearFocus();
    emit this->searchSignal("");
    m_stackedWidget->setCurrentIndex(nIndex);
}

int KRightWidget::currentIndex()
{
    return m_stackedWidget->currentIndex();
}

void KRightWidget::animationFinishedSlot()
{

}

void KRightWidget::onRefreshSearchResult()
{
    if (m_searchEditNew->text() == m_searchTextCache) {
        //qInfo()<<"Search Keyword:"<<m_searchTextCache;
        emit this->searchSignal(m_searchTextCache);
    }
}

void KRightWidget::handleSearchTextChanged()
{
    m_searchTextCache = m_searchEditNew->text();
    KRightWidget::searchText = m_searchEditNew->text();
    if (m_searchTextCache.isEmpty()) {
        m_isSearching = false;
    } else {
        m_isSearching = true;
    }
    if (m_searchTimer->isActive()) {
        m_searchTimer->stop();
    }
    m_searchTimer->start(300);
}

void KRightWidget::changeTitleWidget(bool isTablet)
{
    if (isTablet) {
        m_titleWidget->setFixedHeight(64);
        m_btnOption->setFixedSize(48, 48);
        m_btnMin->setFixedSize(48, 48);
        m_btnClose->setFixedSize(48, 48);
        m_btnMax->setVisible(false);
        m_searchEditNew->setFixedSize(280,40);
    } else {
        m_titleWidget->setFixedHeight(40);
        m_btnOption->setFixedSize(30, 30);
        m_btnMin->setFixedSize(30, 30);
        m_btnMax->setFixedSize(30, 30);
        m_btnClose->setFixedSize(30, 30);
        m_btnMax->setVisible(true);
        m_searchEditNew->setFixedSize(SPECIALWIDTH, NORMALHEIGHT);
    }
}
