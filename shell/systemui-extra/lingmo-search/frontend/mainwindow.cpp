/*
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: zhangjiaping <zhangjiaping@kylinos.cn>
 * Modified by: zhangpengfei <zhangpengfei@kylinos.cn>
 *
 */

#include "mainwindow.h"
#include <QHBoxLayout>
#include <QDebug>
#include <QDesktopWidget>
#include <syslog.h>
#include <QPalette>
#include <QScreen>
#include <QStyleOption>
#include <QPixmap>
#include <KWindowEffects>
#include <KWindowSystem>
#include <QtX11Extras/QX11Info>
#include <gperftools/malloc_extension.h>
#include <lingmostylehelper/lingmostylehelper.h>
#include <windowmanager/windowmanager.h>
#include <window-helper.h>
#include "global-settings.h"
#include "action-transmiter.h"
#include "icon-loader.h"

#define MAIN_MARGINS 0, 0, 0, 0
#define TITLE_MARGINS 0,0,0,0
#define WINDOW_WIDTH 700
#define WINDOW_HEIGHT 610
#define TITLE_HEIGHT 40
#define WINDOW_ICON_SIZE 24
#define SETTING_BTN_SIZE 30
#define SEARCH_BAR_SIZE 48
#define ASK_INDEX_TIME 5*1000
#define RESEARCH_TIME 10*1000

#define MAIN_SETTINGS QDir::homePath() + "/.config/org.lingmo/lingmo-search/lingmo-search.conf"
#define ENABLE_CREATE_INDEX_ASK_DIALOG "enable_create_index_ask_dialog"
const static QString FILE_INDEX_ENABLE_KEY = "fileIndexEnable";
static const QByteArray LINGMO_SEARCH_SCHEMAS = QByteArrayLiteral("org.lingmo.search.settings");

using namespace LingmoUISearch;
extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);
/**
 * @brief MainWindow 主界面
 * @param parent
 *
 * 慎用KWindowSystem::setShowingDesktop(!KWindowSystem::showingDesktop());
 * 可能造成窗口属性的混乱
 */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent) {
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->setWindowFlag(Qt::FramelessWindowHint);
    this->setAutoFillBackground(false);
    this->setFocusPolicy(Qt::StrongFocus);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->setWindowTitle(tr("lingmo-search"));
    KWindowSystem::setState(this->winId(),NET::SkipTaskbar | NET::SkipPager | NET::SkipSwitcher );
    initSettings();
    initUi();
    initTimer();
    installEventFilter(this);
    initConnections();

//    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, this,[&](WId activeWindowId){
//        qDebug() << "activeWindowChanged!!!" << activeWindowId;
//            if (activeWindowId != this->winId()) {
//                tryHideMainwindow();
//            }
//        });

    m_appWidgetPlugin = new AppWidgetPlugin;

//    connect(m_appWidgetPlugin, &AppWidgetPlugin::startSearch, this, [ & ] (QString keyword){
//        this->bootOptionsFilter("-s");
//        this->setText(keyword);
//    });
    connect(m_appWidgetPlugin, &AppWidgetPlugin::start, this, [&] {
        this->bootOptionsFilter("-s");
    });
    connect(ActionTransmiter::getInstance(), &ActionTransmiter::hideUIAction, this, &MainWindow::tryHideMainwindow);
}

MainWindow::~MainWindow() {
    if(m_askDialog) {
        delete m_askDialog;
        m_askDialog = NULL;
    }
//    if(m_askTimer) {
//        delete m_askTimer;
//        m_askTimer = NULL;
//    }
    if(m_searchGsettings) {
        delete m_searchGsettings;
        m_searchGsettings = NULL;
    }
}

/**
 * @brief initUi 初始化主界面主要ui控件
 */
void MainWindow::initUi()
{
    m_radius = GlobalSettings::getInstance().getValue(WINDOW_RADIUS_KEY).toInt();
    connect(&GlobalSettings::getInstance(), &GlobalSettings::valueChanged, this, [&](const QString& key, const QVariant& value){
        if(key == WINDOW_RADIUS_KEY) {
            m_radius = value.toInt();
        }
    });

    this->setFixedSize(WINDOW_WIDTH, 68);
    m_searchBarWidget = new SearchBarWidget(this);
    m_searchBarWidget->move(this->rect().topLeft());
    m_searchBarWidget->show();
    m_searchResultPage = new SearchResultPage(this);
    m_searchResultPage->hide();
    m_searchResultPage->move(0, 58);
    this->setFocusProxy(m_searchBarWidget);

    //创建索引询问弹窗
    m_askDialog = new CreateIndexAskDialog(this);
}

void MainWindow::initConnections()
{
    connect(QApplication::primaryScreen(), &QScreen::geometryChanged, this, &MainWindow::ScreenGeometryChanged);
    connect(m_askDialog, &CreateIndexAskDialog::closed, this, [ = ]() {
        m_isAskDialogVisible = false;
    });
    connect(m_askDialog, &CreateIndexAskDialog::btnClicked, this, [ = ](const bool &isCreateIndex, const bool &isAskAgain) {
        setSearchMethodConfig(isCreateIndex, isAskAgain);
    });
    connect(m_askDialog, &CreateIndexAskDialog::focusChanged, this, [ & ] {
            if (kdk::WindowManager::getPid(kdk::WindowManager::currentActiveWindow()) != QApplication::applicationPid()) {
            this->tryHideMainwindow();
        }
    });
//    connect(m_settingsBtn, &QPushButton::clicked, this, &MainWindow::settingsBtnClickedSlot);
    //主题改变时，更新自定义标题栏的图标
//    connect(qApp, &QApplication::paletteChanged, this, [ = ]() {
//        m_iconLabel->setPixmap(QIcon::fromTheme("lingmo-search").pixmap(QSize(WINDOW_ICON_SIZE, WINDOW_ICON_SIZE)));
//    });
    connect(m_searchBarWidget, &SearchBarWidget::requestSearchKeyword, this, &MainWindow::searchKeywordSlot);
//    connect(m_stackedWidget, &StackedWidget::effectiveSearch, m_searchLayout, &SearchBarHLayout::effectiveSearchRecord);
    //connect(m_searchResultPage, &SearchResultPage::resizeHeight, this, &MainWindow::resizeHeight);
    connect(this,&MainWindow::setText,m_searchBarWidget,&SearchBarWidget::setText);
}

/**
 * @brief bootOptionsFilter 过滤终端命令
 * @param opt
 */
void MainWindow::bootOptionsFilter(QString opt) {
    if(opt == "-s" || opt == "--show") {
        if (this->isHidden()) {
            clearSearchResult();
            centerToScreen(this);
            this->m_searchBarWidget->setFocus();
            QTimer::singleShot(0, [&](){
                QGuiApplication::inputMethod()->show();
            });
        }
        this->activateWindow();
        this->raise();
    }
}

/**
 * @brief clearSearchResult 清空搜索结果
 */
void MainWindow::clearSearchResult() {
    m_searchBarWidget->clear();
//    m_searchLineEdit->clearFocus();
}

/**
 * @brief setSearchMethodConfig 在询问弹窗点击按钮后执行
 * @param create_index 是否同意创建索引
 * @param no_longer_ask 是否勾选了不再提示
 */
void MainWindow::setSearchMethodConfig(const bool& createIndex, const bool& noLongerAsk)
{
    if(noLongerAsk) {
        m_settings->setValue(ENABLE_CREATE_INDEX_ASK_DIALOG, false);
    } else {
        m_settings->setValue(ENABLE_CREATE_INDEX_ASK_DIALOG, true);
    }
    if(createIndex) {
        if(m_searchGsettings && m_searchGsettings->keys().contains(FILE_INDEX_ENABLE_KEY)) {
            m_searchGsettings->set(FILE_INDEX_ENABLE_KEY, true);
        }
        //创建索引十秒后重新搜索一次(如果用户十秒内没有退出搜索界面且没有重新搜索)
        m_researchTimer->start();
    }
}
/**
 * @brief MainWindow::searchKeywordSlot 执行搜索的槽函数
 * @param keyword 关键词
 */
void MainWindow::searchKeywordSlot(const QString &keyword)
{
    if(keyword == "" || keyword.isEmpty()) {
//        m_stackedWidget->setPage(int(StackedPage::HomePage));
        QTimer::singleShot(10, this, [ = ]() {
            m_askTimer->stop();
            Q_EMIT m_searchResultPage->stopSearch();
            m_searchResultPage->hide();
            this->resizeHeight(68);
        });

    } else {
        QTimer::singleShot(10, this, [ = ]() {
            //允许弹窗且当前次搜索（为关闭主界面，算一次搜索过程）未询问且当前为暴力搜索
            if(m_settings->value(ENABLE_CREATE_INDEX_ASK_DIALOG).toBool()
                    && !m_currentSearchAsked
                    && !m_isIndexSearch) {
                m_askTimer->start();
            }
            Q_EMIT m_searchResultPage->startSearch(keyword);
            this->resizeHeight(WINDOW_HEIGHT);

            m_searchResultPage->move(0, 58);
            m_searchResultPage->show();
        });
    }
    m_researchTimer->stop(); //如果搜索内容发生改变，则停止建索引后重新搜索的倒计时
}

void MainWindow::resizeHeight(int height)
{
    this->setFixedHeight(height);
}

void MainWindow::tryHide()
{
    this->tryHideMainwindow();
}

void MainWindow::ScreenGeometryChanged(QRect rect) {
    Q_UNUSED(rect);
    if(this->isVisible()) {
        centerToScreen(this);
    }
}

/**
 * @brief MainWindow::centerToScreen 使窗口显示在屏幕中间
 * @param widget
 */
void MainWindow::centerToScreen(QWidget* widget) {
    if(!widget)
        return;
    KWindowSystem::setState(this->winId(),NET::SkipTaskbar | NET::SkipPager | NET::SkipSwitcher);
    QRect desk_rect = LingmoUIQuick::WindowProxy::currentScreen()->geometry();
    int desk_x = desk_rect.width();
    int desk_y = desk_rect.height();
    int x = widget->width();
    widget->show();
    kdk::WindowManager::setGeometry(this->windowHandle(),QRect(desk_x / 2 - x / 2 + desk_rect.left(),
                                               desk_y / 6 + desk_rect.top(),
                                               this->width(),
                                               this->height()));
    //设置跳过多任务视图
    kdk::WindowManager::setSkipSwitcher(this->windowHandle(),true);
    //设置跳过任务栏
    kdk::WindowManager::setSkipTaskBar(this->windowHandle(),true);
//    widget->move(desk_x / 2 - x / 2 + desk_rect.left(), desk_y / 3 + desk_rect.top());
}

void MainWindow::initSettings() {
    const QByteArray id(LINGMO_SEARCH_SCHEMAS);
    if(QGSettings::isSchemaInstalled(id)) {
        m_searchGsettings = new QGSettings(id);
        if (m_searchGsettings->keys().contains(FILE_INDEX_ENABLE_KEY)) {
            m_isIndexSearch = m_searchGsettings->get(FILE_INDEX_ENABLE_KEY).toBool();
        }
        connect(m_searchGsettings, &QGSettings::changed, this, [ = ](const QString & key) {
            if(key == FILE_INDEX_ENABLE_KEY) {
                m_isIndexSearch = m_searchGsettings->get(FILE_INDEX_ENABLE_KEY).toBool();
                if(m_researchTimer->isActive() && !m_isIndexSearch) {
                    m_researchTimer->stop();
                }
            }
        });
    }
    m_settings = new QSettings(MAIN_SETTINGS, QSettings::IniFormat, this);
}

//使用GSetting获取当前窗口应该使用的透明度
double MainWindow::getTransparentData() {
    return GlobalSettings::getInstance().getValue(TRANSPARENCY_KEY).toDouble();
}

void MainWindow::initTimer() {
    m_askTimer = new QTimer(this);
    m_askTimer->setInterval(ASK_INDEX_TIME);
    connect(m_askTimer, &QTimer::timeout, this, [ = ]() {
        QWindow *modal = QGuiApplication::modalWindow();
        if(modal) {
            m_askTimer->stop();
            connect(modal, &QWindow::visibleChanged, this, [ & ](bool visible){
                if(!visible) {
                  m_askTimer->start();
                }
            }, Qt::UniqueConnection);
            return;
        }
        if(this->isVisible() && !m_isIndexSearch) {
            m_isAskDialogVisible = true;
            kdk::LingmoUIStyleHelper::self()->removeHeader(m_askDialog);
            m_askDialog->show();
            //设置跳过多任务视图
            kdk::WindowManager::setSkipSwitcher(m_askDialog->windowHandle(),true);
            //设置跳过任务栏
            kdk::WindowManager::setSkipTaskBar(m_askDialog->windowHandle(),true);
            m_currentSearchAsked = true;
        }
        m_askTimer->stop();
    });
    m_researchTimer = new QTimer;
    m_researchTimer->setInterval(RESEARCH_TIME);
    connect(m_researchTimer, &QTimer::timeout, this, [ = ]() {
        if(this->isVisible()) {
            m_searchBarWidget->reSearch();
        }
        m_researchTimer->stop();
    });
    connect(m_searchBarWidget, &SearchBarWidget::requestSearchKeyword, this, [ = ](QString text) {
        if(text == "" || text.isEmpty()) {
            m_askTimer->stop();
        } else {
            //允许弹窗且当前次搜索（为关闭主界面，算一次搜索过程）未询问且当前为暴力搜索
            if(m_settings->value(ENABLE_CREATE_INDEX_ASK_DIALOG, true).toBool() && !m_currentSearchAsked && !m_isIndexSearch) {
                m_askTimer->start();
            }
        }
    });
}

/**
 * @brief MainWindow::tryHideMainwindow 尝试隐藏主界面并停止部分未完成的动作，重置部分状态值
 */
bool MainWindow::tryHideMainwindow()
{
    if (QApplication::activeModalWidget() == nullptr) {
        qDebug()<<"Mainwindow will be hidden";
        m_currentSearchAsked = false;
        m_askTimer->stop();
        m_researchTimer->stop();
        Q_EMIT m_searchResultPage->stopSearch();
        this->clearSearchResult();
        this->hide();
        if(!m_releaseFreeMemoryTimerWorking) {
            m_releaseFreeMemoryTimerWorking = true;
            QTimer::singleShot(2000, this, [&](){
                if(!this->isVisible()) {
                    MallocExtension::instance()->ReleaseFreeMemory();
                }
                m_releaseFreeMemoryTimerWorking = false;
            });
        }
        return true;
    } else {
        //有上层弹窗未关闭，不允许隐藏主界面
        qWarning()<<"There is a dialog onside, so that mainwindow can not be hidden.";
        return false;
    }
}

/**
 * @brief MainWindow::setSearchMethod 设置搜索模式
 * @param is_index_search true为索引搜索，false为暴力搜索
 */
void MainWindow::setSearchMethod(const bool isIndexSearch) {
    if(!isIndexSearch) {
        m_researchTimer->stop();
    }
}

/**
 * @brief MainWindow::keyPressEvent 按esc键关闭主界面
 * @param event
 */
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        tryHideMainwindow();
    } else if (event->key() == Qt::Key_Return or event->key() == Qt::Key_Enter) {
        if (!m_searchResultPage->isHidden()) {
            //显示最佳匹配中第一项的详情页，无搜索结果则调取网页搜索
            qDebug() << "Press Enter";
            m_searchResultPage->pressEnter();
        }
    } else if (event->key() == Qt::Key_Up) {
        qDebug() << "Press ↑";
        m_searchResultPage->pressUp();
    } else if (event->key() == Qt::Key_Down) {
        qDebug() << "Press ↓";
        if (!m_searchResultPage->getSelectedState()) {
            m_searchResultPage->pressEnter();
        } else {
            m_searchResultPage->pressDown();
        }
    }
    return QWidget::keyPressEvent(event);
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainterPath path;
    path.addRoundedRect(m_searchBarWidget->x()+10, m_searchBarWidget->y()+10, m_searchBarWidget->width()-20, m_searchBarWidget->height()-20, m_radius, m_radius);
    path.addRoundedRect(m_searchResultPage->x()+10, m_searchResultPage->y()+10, m_searchResultPage->width()-20, m_searchResultPage->height()-20, m_radius, m_radius);
    KWindowEffects::enableBlurBehind(this->windowHandle(), true, QRegion(path.toFillPolygon().toPolygon()));
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this) {
        //失焦退出
        if (event->type() == QEvent::ActivationChange) {
            if (QApplication::activeWindow() != this && QApplication::activeWindow() != m_askDialog) {
                tryHideMainwindow();
                return true;
            }
        }
        //kwin alt+f4发出close事件, 需要在存在子窗口时屏蔽该事件
        if (event->type() == QEvent::Close) {
            event->ignore();
            tryHideMainwindow();
//            return true;
        }
    }

    return QObject::eventFilter(watched, event);
}
