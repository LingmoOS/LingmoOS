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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "properties-window.h"

#include "properties-window-tab-page-plugin-iface.h"

#include "basic-properties-page-factory.h"
#include "permissions-properties-page-factory.h"
#include "computer-properties-page-factory.h"
#include "recent-and-trash-properties-page-factory.h"
#include "mark-properties-page-factory.h"
#include "open-with-properties-page-factory.h"
#include "details-properties-page-factory.h"
#include "thumbnail-manager.h"
#include "file-utils.h"
#include "vfs-plugin-manager.h"
#include "xatom-helper.h"
//#include "properties-window-factory.h"

#include <QToolBar>
#include <QPushButton>
#include <QProcess>
#include <QDebug>
#include <QStatusBar>

#include <QList>
#include <QStringList>
#include <QString>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QTimer>
#include <QPainter>
#include <QStyleOptionTab>
#include <QApplication>
#include <KWindowSystem>
#include <QGSettings>

#include <QPainterPath>

#include <glib/gstdio.h>
#include <gio/gdesktopappinfo.h>
#include <pwd.h>

#include <QApplication>
#include <QGSettings>
#include <QtConcurrent>

#include "file-info-job.h"

#include <lingmosdk/applications/lingmostylehelper/lingmostylehelper.h>

#include <lingmosdk/applications/ktabbar.h>

using namespace Peony;

#define WINDOW_NOT_OPEN -1
//single properties-window
//static QHash<QString,PropertiesWindow> openPropertiesWindow = nullptr;
static QList<PropertiesWindow *> *openedPropertiesWindows = nullptr;
//plugin manager

static PropertiesWindowPluginManager *global_instance = nullptr;

PropertiesWindowPluginManager *PropertiesWindowPluginManager::getInstance()
{
    if (!global_instance)
        global_instance = new PropertiesWindowPluginManager;
    return global_instance;
}

void PropertiesWindowPluginManager::setOpenFromDesktop()
{
    unregisterFactory(MarkPropertiesPageFactory::getInstance());
}

void PropertiesWindowPluginManager::setOpenFromPeony()
{
    registerFactory(MarkPropertiesPageFactory::getInstance());
}

void PropertiesWindowPluginManager::release()
{
    deleteLater();
}

PropertiesWindowPluginManager::PropertiesWindowPluginManager(QObject *parent) : QObject(parent)
{
    //register internal factories.
    registerFactory(BasicPropertiesPageFactory::getInstance());
    registerFactory(PermissionsPropertiesPageFactory::getInstance());
    registerFactory(ComputerPropertiesPageFactory::getInstance());
    registerFactory(RecentAndTrashPropertiesPageFactory::getInstance());
    registerFactory(MarkPropertiesPageFactory::getInstance());
    registerFactory(OpenWithPropertiesPageFactory::getInstance());
    registerFactory(DetailsPropertiesPageFactory::getInstance());
}

PropertiesWindowPluginManager::~PropertiesWindowPluginManager()
{
    for (auto factory : m_factory_hash) {
        factory->closeFactory();
    }
    m_factory_hash.clear();
}


bool PropertiesWindowPluginManager::registerFactory(PropertiesWindowTabPagePluginIface *factory)
{
    m_mutex.lock();
    auto id = factory->name();
    if (m_factory_hash.value(id)) {
        m_mutex.unlock();
        return false;
    }

    m_factory_hash.insert(id, factory);
    m_sorted_factory_map.insert(-factory->tabOrder(), id);
    m_mutex.unlock();
    return true;
}

bool PropertiesWindowPluginManager::unregisterFactory(PropertiesWindowTabPagePluginIface *factory)
{
    m_mutex.lock();
    auto id = factory->name();
    if (m_factory_hash.value(id)) {
        m_factory_hash.remove(id);

        int current = 0;
        for (auto i = m_sorted_factory_map.begin(); i != m_sorted_factory_map.end(); ++i) {
            if (i.value() == id) {
                current = i.key();
                break;
            }
        }
        m_sorted_factory_map.remove(current);
        m_mutex.unlock();
        return true;
    }

    m_mutex.unlock();
    return false;
}

const QStringList PropertiesWindowPluginManager::getFactoryNames()
{
    QStringList list;
    for (auto factoryId : m_sorted_factory_map) {
        list << factoryId;
    }
    return list;
}

PropertiesWindowTabPagePluginIface *PropertiesWindowPluginManager::getFactory(const QString &id)
{
    return m_factory_hash.value(id);
}

/*!
 * init PropertiesWindows`s static member
 * \brief PropertiesWindow::s_windowWidth
 */
const qint32 PropertiesWindow::s_windowWidth        = 440;
const qint32 PropertiesWindow::s_windowHeightFolder = 587;
const qint32 PropertiesWindow::s_windowHeightOther  = 651;
const QSize  PropertiesWindow::s_bottomButtonSize   = QSize(96, 36);

PropertiesWindow::PropertiesWindow(const QStringList &uris, QWidget *parent) : QMainWindow(parent)
{
    if (uris.count() == 0) {
        m_destroyThis = true;
        return;
    }

    //将uri编码统一解码,解决uri的不一致问题。from bug:53504
    for (QString uri : uris) {
        uri = FileUtils::urlDecode(uri);

        if (uri.startsWith("favorite://")) {
            rebuildUriBySchema(uri);

        } else if (uri.startsWith("kmre://")) {
            if (!handleKMREUri(uri)) {
                m_destroyThis = true;
                return;
            }

        } else if (uri.startsWith("network://")) {
            m_destroyThis = true;
            return;
        }else if(uri.startsWith("label://")){
            uri = FileUtils::getTargetUri(uri);/* 转化为真实的路径 */
        }
        //fix bug:70565,将已被编码的字符串解码后从新编码，保证在属性窗口中的编码中特殊字符为%xx形式。
        //编码时排除'()',防止 FileUtils::handleDesktopFileName 方法匹配不到(),避免出现bug:53504.
        m_uris.append(uri.toUtf8().toPercentEncoding(":/()"));
    }
//    m_uris = uris;
    m_uris.removeDuplicates();
    qDebug() << __FUNCTION__ << m_uris.count() << m_uris;
    setWindowOpacity(0);

    if (qApp->property("showProperties").isValid() && qApp->property("showProperties").toBool()) {
        PropertiesWindowPluginManager::getInstance()->setOpenFromDesktop();
        qApp->setProperty("showProperties", false);
    } else {
        PropertiesWindowPluginManager::getInstance()->setOpenFromPeony();
    }

    //FIX:BUG #31635
    if (m_uris.contains("computer:///")) {
        QtConcurrent::run([=]() {
            gotoAboutComputer();
        });
        m_uris.removeAt(m_uris.indexOf("computer:///"));
    }

    this->notDir();

    if (!m_uris.isEmpty() && !PropertiesWindow::checkUriIsOpen(m_uris, this)) {
        this->init();
    } else {
        m_destroyThis = true;
    }

    connect(qApp, &QApplication::fontChanged, [=](){
        QFont font = qApp->font();
        for (auto widget : qApp->allWidgets()) {
            widget->setFont(font);
        }
    });

    if (QGSettings::isSchemaInstalled("org.lingmo.style")) {
        QGSettings *settings = new QGSettings("org.lingmo.style", QByteArray(), this);
        connect(settings, &QGSettings::changed, this, [=](const QString &key) {
            if("iconThemeName" == key)
            {
                if (!m_uris.isEmpty()) {
                    this->setWindowTitleTextAndIcon();
                }
            }
        });
    }
}

void PropertiesWindow::init()
{
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setContentsMargins(0, 0, 0, 0);
    this->setAttribute(Qt::WA_TranslucentBackground);
#ifdef LINGMO_SDK_WAYLANDHELPER
    kdk::LingmoUIStyleHelper::self()->removeHeader(this);
#else
    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(window()->winId(), hints);
#endif
    //only show close button
    //this->setWindowFlags(this->windowFlags() & ~Qt::WindowMinMaxButtonsHint & ~Qt::WindowSystemMenuHint);

    this->setWindowTitleTextAndIcon();
    KWindowSystem::setState(this->winId(), NET::SkipTaskbar|NET::SkipPager);

    if (m_notDir) {
        //如果含有文件夹，那么高度是600，如果是其他文件，那么高度是652
        //If there are folders, the height is 600, if it is other files, the height is 652
        this->setFixedSize(PropertiesWindow::s_windowWidth, PropertiesWindow::s_windowHeightOther);
    } else {
        this->setFixedSize(PropertiesWindow::s_windowWidth, PropertiesWindow::s_windowHeightFolder);
    }
    this->initStatusBar();

    this->initTabPage(m_uris);

    QTimer::singleShot(400, Qt::PreciseTimer, this, [=]{
        //fix bug:58167
        setFocus(Qt::OtherFocusReason);
        setWindowOpacity(1);
    });
}

/*!
 * recent:///  : 最近
 * trash:///   : 回收站
 *
 * \brief PropertiesWindow::setWindowTitleText
 */
void PropertiesWindow::setWindowTitleTextAndIcon()
{
    HeaderBar *headerBar = new HeaderBar(this);
    this->addToolBar(Qt::ToolBarArea::TopToolBarArea, headerBar);

    connect(headerBar, &HeaderBar::buttonClicked, this, [=](const HeaderBar::ButtonType buttonType) {
        switch (buttonType) {
            case HeaderBar::ButtonType::Spread_Button: {
                if (this->width() < 600) {
                    this->setFixedWidth(600);
                } else {
                    this->setFixedWidth(PropertiesWindow::s_windowWidth);
                }
                break;
            }
            case HeaderBar::ButtonType::Minimize_Button: {
                KWindowSystem::minimizeWindow(this->winId());
                this->showMinimized();
                break;
            }
            case HeaderBar::ButtonType::Close_Button: {
                this->close();
                break;
            }
        }
    });

    QString windowTitle = "";
    QString iconName = "system-file-manager";

    /**
     * \brief Trash 和 Recent 情况下，uris中只有一个uri - In the case of Trash and Recent, there is only one uri in the uris
     */
    if (m_uris.contains("trash:///")) {
        windowTitle = tr("Trash");
        iconName = m_fileInfo.get()->iconName();

    } else if (m_uris.contains("recent:///")) {
        windowTitle = tr("Recent");
        iconName = m_fileInfo.get()->iconName();

    } else {
        qint32 fileNum = m_uris.count();

        if (fileNum > 1) {
            //use default icon
            windowTitle = tr("Selected") + QString(tr(" %1 Files")).arg(fileNum);
        } else {
            qDebug() << __FILE__ << __FUNCTION__ << "fileInfo is null :" << (m_fileInfo.get() == nullptr);
            if (m_fileInfo) {
                //fix bug:#82320
                if (QRegExp("^file:///data/usershare(/{,1})$").exactMatch(m_fileInfo->uri())) {
                    windowTitle = tr("usershare");
                } else {
                    windowTitle = m_fileInfo.get()->displayName();
                }  
                //fix bug#182415, fix show Unknown-icon issue, but basic info icon is correct
                iconName = FileUtils::getFileIconName(m_fileInfo.get()->uri(), true);
                if (iconName.isEmpty()) {
                    iconName = FileUtils::getFileIconName(m_fileInfo.get()->uri(), false);
                }

                if("computer:///lingmo-data-volume" == m_fileInfo->uri()){
                    windowTitle = tr("Data");
                    iconName = "drive-harddisk";
                }
            }
        }
    }

    windowTitle += " " + tr("Properties");

    if (iconName == "application-x-desktop") {
        iconName = getIconName();
    }

    const QByteArray id("org.lingmo.style");
    if (QGSettings::isSchemaInstalled(id)) {
        QGSettings *styleSettings = new QGSettings(id, QByteArray(), this);
        connect(styleSettings, &QGSettings::changed, this, [=](const QString &key){
            if (key == "iconThemeName") {
                setWindowIcon(QIcon::fromTheme(iconName, QIcon::fromTheme("unknown")));
            }
        });
    }

    this->setWindowIcon(QIcon::fromTheme(iconName, QIcon::fromTheme("unknown")));
    this->setWindowTitle(windowTitle);
    headerBar->setIcon(iconName);
    headerBar->setTitle(windowTitle);
}

void PropertiesWindow::notDir()
{
    //FIXME:请尝试使用非阻塞方式获取 FIleInfo - Please try to obtain FIleInfo in a non-blocking way
    bool first = true;
    QStringList targetUris;
    for (QString uri : m_uris) {
        auto fileInfo = FileInfo::fromUri(uri);
        FileInfoJob *fileInfoJob = new FileInfoJob(fileInfo);
        fileInfoJob->setAutoDelete();
        fileInfoJob->querySync();

        if (first) {
            //使用第一个文件信息确认所在目录等基本信息。
            //在最近文件夹中多选状态下，文件所在位置将会不准确，因为最近文件夹中的文件来自于不同的位置。
            m_fileInfo = fileInfo;
            first = false;
        }

        if (fileInfo.get()->isDir() && m_notDir) {
            m_notDir = false;
        }

        if (uri.startsWith("recent://")) {
            if (fileInfo->targetUri() != "") {
                targetUris.append(fileInfo->targetUri());
            }
        }
    }

    if (targetUris.count() > 0)
        m_uris = targetUris;
}

void PropertiesWindow::show()
{
    if (m_destroyThis) {
        this->close();
        return;
    }

    return QWidget::show();
}

void PropertiesWindow::gotoAboutComputer()
{
    QProcess p;
    p.setProgram("/usr/bin/lingmo-control-center");
    //-m About para to show about computer infos, related to bug#88258
    p.setArguments(QStringList()<<"-m" << "About");
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    p.startDetached();
#else
    p.startDetached("/usr/bin/lingmo-control-center", QStringList()<<"-m" << "About");
#endif
    p.waitForFinished(-1);
}

/*!
 *
 * \brief PropertiesWindow::initStatusBar
 */
void PropertiesWindow::initStatusBar()
{
    QToolBar *bottomToolBar = new QToolBar(this);
    bottomToolBar->setMovable(false);
    bottomToolBar->setMinimumHeight(64);
    bottomToolBar->setContentsMargins(0, 0, 0, 12);
    bottomToolBar->setStyleSheet("QToolBar{border-color: transparent;border: none;}");

    QWidget *container = new QWidget(bottomToolBar);
    container->setContentsMargins(0, 0, 0, 0);
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    //use HBox-layout
    QHBoxLayout *bottomToolLayout = new QHBoxLayout(container);
    bottomToolLayout->setSpacing(16);
    bottomToolLayout->setContentsMargins(24, 0, 24, 12);

    QPushButton *okButton = new QPushButton(tr("Ok"), container);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"), container);

    okButton->setMinimumSize(PropertiesWindow::s_bottomButtonSize);
    cancelButton->setMinimumSize(PropertiesWindow::s_bottomButtonSize);

    //task#100231 trash page OK button set as restore
    //fix bug#143817, trash properties issue
    if (m_uris.first().startsWith("trash:///") && m_uris.first() != "trash:///")
        okButton->setText(tr("Restore"));

    bottomToolLayout->addStretch(1);
    bottomToolLayout->addWidget(cancelButton);
    bottomToolLayout->addWidget(okButton);

    container->setLayout(bottomToolLayout);

    bottomToolBar->addWidget(container);
    this->addToolBar(Qt::ToolBarArea::BottomToolBarArea, bottomToolBar);

    //set cancelButton event process
    connect(cancelButton, &QPushButton::clicked, this, &QMainWindow::close);
    connect(okButton, &QPushButton::clicked, this, &PropertiesWindow::saveAllChanged);
}

void PropertiesWindow::initTabPage(const QStringList &uris)
{
    if(uris.isEmpty())
        return;

    auto window = new PropertiesWindowPrivate(uris, this);
    window->tabBar()->setStyle(new tabStyle);
    //Warning: 不要设置tab高度，否则会导致tab页切换上下跳动
    //Do not set the tab height, otherwise it will cause the tab page to switch up and down
    //window->tabBar()->setMinimumHeight(72);
    this->setCentralWidget(window);
}

bool PropertiesWindow::checkUriIsOpen(QStringList &uris, PropertiesWindow *newWindow)
{
    if (!openedPropertiesWindows) {
        openedPropertiesWindows = new QList<PropertiesWindow *>();
        qDebug() << __FILE__ << __FUNCTION__ << "new->openedPropertiesWindows";
    }
    //1.对uris进行排序 - Sort uris
    std::sort(uris.begin(), uris.end(), [](QString a, QString b) {
        return a < b;
    });

    //2.检查是否已经打开 - Check if it is open
    qint64 index = PropertiesWindow::getOpenUriIndex(uris);
    if (index != WINDOW_NOT_OPEN) {
        openedPropertiesWindows->at(index)->raise();
        return true;
    }

    openedPropertiesWindows->append(newWindow);

    return false;
}

qint64 PropertiesWindow::getOpenUriIndex(QStringList &uris)
{
    //strong !
    if (!openedPropertiesWindows)
        return WINDOW_NOT_OPEN;

    quint64 index = 0;
    for (PropertiesWindow *window : *openedPropertiesWindows) {
        if (window->getUris() == uris) {
            //当前的uris已经存在打开的窗口 - The current uris already exists in the open window
            return index;
        }
        index++;
    }

    return WINDOW_NOT_OPEN;
}

void PropertiesWindow::removeThisWindow(qint64 index)
{
    if (index == WINDOW_NOT_OPEN)
        return;

    if (!openedPropertiesWindows)
        return;

    openedPropertiesWindows->removeAt(index);

    if (openedPropertiesWindows->count() == 0) {
        delete openedPropertiesWindows;
        openedPropertiesWindows = nullptr;
        qDebug() << __FILE__ << __FUNCTION__ << "delete->openedPropertiesWindows";
    }

}

QString PropertiesWindow::getIconName()
{
    if (m_fileInfo == nullptr)
        return "application-x-desktop";

    QString realPath;
    bool startWithTrash = m_fileInfo->uri().startsWith("trash:///");
    bool startWithRecent = m_fileInfo->uri().startsWith("recent:///");

    if (startWithTrash) {
        realPath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first()
                   + "/.local/share/Trash/files/"
                   + m_fileInfo->displayName();
    } else if (startWithRecent) {
        realPath = m_fileInfo->targetUri();
    } else {
        realPath = m_fileInfo->uri();
    }

    auto _desktop_file = g_desktop_app_info_new_from_filename(QUrl(realPath).path().toUtf8().constData());
    if (_desktop_file) {
        return QString(g_desktop_app_info_get_string(_desktop_file, "Icon"));
    }
    //在找不到图标时，返回默认图标 - When the icon is not found, return to the default icon
    return "application-x-desktop";
}

void PropertiesWindow::closeEvent(QCloseEvent *event)
{
    /**
     * \brief 如果当前uris窗口已经打开，那么不能移除全局的记录，只需要关闭当前窗口即可
     * If the current uris window is already open, then the global record cannot be removed, just close the current window
     */
    if (m_destroyThis)
        return;

    PropertiesWindow::removeThisWindow(PropertiesWindow::getOpenUriIndex(this->getUris()));
}

/*!
 * save all changed settings when 'ok' is clicked
 * \brief PropertiesWindow::saveAllChanged
 */
void PropertiesWindow::saveAllChanged()
{
    qDebug() << "PropertiesWindow::saveAllChanged()" << "count" << m_openTabPage.count();
    if (!m_openTabPage.count() == 0) {
        for (auto tabPage : m_openTabPage) {
            tabPage->saveAllChange();
        }
    }

    this->close();
}

QString PropertiesWindow::rebuildUriBySchema(QString &uri)
{
    QUrl url(uri);

    if (!url.isValid()) {
        return uri;
    }
    QMap<QString, QString> queryMap;
    QStringList queryList = url.query().split("&");

    for (QString str : queryList) {
        QStringList query = str.split("=");
        queryMap.insert(query.first(), query.last());
    }

    QString schema = queryMap.value("schema");

    if (schema.isEmpty()) {
        return uri;
    }

    uri.replace(0, QString("favorite").length(), schema);
    //删除uri '?'及之后的信息
    uri.remove(uri.lastIndexOf("?"), (url.query().length() + 1));

    return uri;
}

bool PropertiesWindow::handleKMREUri(QString &uri)
{
//    bool kmreIsInstalled = false;
//    for (VFSPluginIface *vfs : VFSPluginManager::getInstance()->registeredPlugins()) {
//        if (vfs->uriScheme() == "kmre://") {
//            kmreIsInstalled = true;
//            break;
//        }
//    }
//
//    if (!kmreIsInstalled) {
//        return false;
//    }

    if (uri == "kmre:///") {
        uid_t uid = geteuid();
        struct passwd *pw = getpwuid(uid);
        if (!pw) {
            return false;
        }
        uri = QString("file:///var/lib/kmre/data/kmre-%1-%2").arg(QString::number(uid)).arg(QString(pw->pw_name));

    } else if (uri.contains("&real-path:")) {
        //kmre:///picture&real-path:/var/lib
        uri = "file://" + uri.split("&real-path:").last();

    } else {

    }

    return true;
}

void PropertiesWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(this->rect(), this->palette().base());

    QWidget::paintEvent(event);
}


#ifdef LINGMO_SDK_QT_WIDGETS
class TabBar : public kdk::KTabBar
{
public:
    explicit TabBar(QWidget *parent) : kdk::KTabBar(kdk::KTabBarStyle::SegmentDark, parent)
    {

    }

protected:
    QSize minimumTabSizeHint(int index) const
    {
        return QTabBar::minimumTabSizeHint(index);
    }
    QSize tabSizeHint(int index) const
    {
        return QTabBar::tabSizeHint(index);
    }
};
#endif

//properties window
PropertiesWindowPrivate::PropertiesWindowPrivate(const QStringList &uris, QWidget *parent) : QTabWidget(parent)
{
#ifdef LINGMO_SDK_QT_WIDGETS
    auto tabbar = new TabBar(nullptr);
    setTabBar(tabbar);
#endif
    setTabsClosable(false);
    setMovable(false);
    setContentsMargins(0, 0, 0, 0);
    //重写subElementRect设置居中
    setStyle(new tabWidgetStyle);

    //监听悬浮事件
    this->tabBar()->setAttribute(Qt::WA_Hover, true);
    auto manager = PropertiesWindowPluginManager::getInstance();
    auto names = manager->getFactoryNames();
    int index = 0;
    for (auto name : names) {
        auto factory = manager->getFactory(name);
        if (factory->supportUris(uris)) {
            auto tabPage = factory->createTabPage(uris);
            tabPage->setParent(this);
            addTab(tabPage, factory->name());
            setTabToolTip(index, factory->name());
            ++index;

            (qobject_cast<PropertiesWindow *>(parent))->addTabPage(tabPage);
        }
    }
}

void tabStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter,
                           const QWidget *widget) const
{
#ifdef LINGMO_SDK_QT_WIDGETS
    return qApp->style()->drawControl(element, option, painter, widget);
#endif
    /**
     * FIX:需要修复颜色不能跟随主题的问题
     * \brief
     */
    if (element == CE_TabBarTab) {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            //设置按钮的左右上下偏移
            QRect rect = tab->rect;
            //左侧移动1px
            rect.setLeft(rect.x() + 1);
            //右侧移动1px
            rect.setRight((rect.x() + rect.width())-2);
            QPainterPath path;
            const qreal radius = 6;
            if(tab->position == QStyleOptionTab::Beginning)
            {
                path.moveTo(rect.topRight());
                path.lineTo(rect.topLeft() + QPointF(radius, 0));
                path.quadTo(rect.topLeft(), rect.topLeft() + QPointF(0, radius));
                path.lineTo(rect.bottomLeft() - QPointF(0, radius));
                path.quadTo(rect.bottomLeft(), rect.bottomLeft() + QPointF(radius, 0));
                path.lineTo(rect.bottomRight());
                path.lineTo(rect.topRight());

            }
            else if(tab->position == QStyleOptionTab::End)
            {
                path.moveTo(rect.topRight() - QPointF(radius, 0));
                path.lineTo(rect.topLeft());
                path.lineTo(rect.bottomLeft() );
                path.lineTo(rect.bottomRight() - QPointF(radius, 0));
                path.quadTo(rect.bottomRight(), rect.bottomRight() + QPointF(0, -radius));
                path.lineTo(rect.topRight() + QPointF(0, radius));
                path.quadTo(rect.topRight(), rect.topRight() + QPointF(-radius, -0));

            }
            else if(tab->position == QStyleOptionTab::OnlyOneTab)
            {
                path.addRoundedRect(rect, radius, radius);
            }
            else
            {
                path.addRect(rect);
            }
            const QPalette &palette = widget->palette();

            //未选中时文字颜色 - Text color when not selected
            painter->setPen(palette.color(QPalette::ButtonText));

            if (tab->state & QStyle::State_Sunken) {
                painter->save();
                QColor color = palette.color(QPalette::Button).lighter(70);
                painter->setPen(Qt::NoPen);
                painter->setBrush(color);

                painter->setRenderHint(QPainter::Antialiasing);  // 反锯齿;
                painter->drawPath(path);
                painter->restore();
            } else if (tab->state & QStyle::State_Selected) {
                painter->save();
                painter->setPen(Qt::NoPen);
                painter->setBrush(palette.brush(QPalette::Highlight));

                painter->setRenderHint(QPainter::Antialiasing);  // 反锯齿;
                painter->drawPath(path);
                painter->restore();

                //选中时文字颜色 - Text color when selected
                painter->setPen(palette.color(QPalette::BrightText));
            } else if (tab->state & QStyle::State_MouseOver) {
                painter->save();
                //QColor color = palette.color(QPalette::Highlight).lighter(140);
                QColor color = palette.color(QPalette::Button).lighter(90);
                painter->setPen(Qt::NoPen);
                painter->setBrush(color);

                painter->setRenderHint(QPainter::Antialiasing);  // 反锯齿;
                painter->drawPath(path);
                painter->restore();
                //选中时文字颜色 - Text color when selected
                painter->setPen(palette.color(QPalette::BrightText));
            } else {
                painter->save();
                painter->setPen(Qt::NoPen);
                painter->setBrush(palette.color(QPalette::Button));

                painter->setRenderHint(QPainter::Antialiasing);  // 反锯齿;
                painter->drawPath(path);
                painter->restore();
            }

            painter->drawText(rect, tab->text, QTextOption(Qt::AlignCenter));

            return;
        }
    }
    if (element == CE_TabBarTabLabel) {
        qApp->style()->drawControl(element, option, painter, widget);
    }
}

QSize tabStyle::sizeFromContents(QStyle::ContentsType ct, const QStyleOption *opt, const QSize &contentsSize,
                                 const QWidget *widget) const
{
    QSize barSize = QProxyStyle::sizeFromContents(ct, opt, contentsSize, widget);

    if (ct == QStyle::CT_TabBarTab) {
        barSize.transpose();
        const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(opt);
        //解决按钮不能自适应的问题
        int fontWidth = tab->fontMetrics.width(tab->text);
        //宽度统一加上30px
        barSize.setWidth(fontWidth + 30);

        //46 - 8 - 8 = 30;
        barSize.setHeight(36);
    }

    return barSize;
}

QRect tabWidgetStyle::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    switch(element)
    {
        case SE_TabWidgetTabBar:
        {
            //解决QTabBar设置为居中
            if (const QStyleOptionTabWidgetFrame *twf = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option))
            {
                QRect rect = QRect(QPoint(0, 0), twf->tabBarSize);
                if (twf->rect.size().width() > twf->tabBarSize.width()) {
                    rect.moveLeft((twf->rect.size() - twf->leftCornerWidgetSize - twf->rightCornerWidgetSize - twf->tabBarSize).width() / 2);
                } else {
                    rect.setWidth(twf->rect.size().width());
                    rect.moveLeft(0);
                }
                return rect;
            }
            break;

        }
    default:
        break;

    }
    return QProxyStyle::subElementRect(element, option, widget);
}

HeaderBar::HeaderBar(QWidget *parent) : QToolBar(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);

    setStyleSheet("QToolBar{border-color: transparent;border:none;}");

    setFixedHeight(40);
    setContentsMargins(0, 0, 0, 0);
    setMovable(false);

    initUI();
}

void HeaderBar::initUI()
{
    QWidget *mainWidget = new QWidget(this);
    mainWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setAlignment(Qt::AlignVCenter);
    layout->setSpacing(4);
    layout->setContentsMargins(8, 2, 8, 2);

    mainWidget->setLayout(layout);

    m_iconLabel = new QLabel(this);
    m_titleLabel = new QLabel(this);

    m_iconLabel->setFixedSize(24, 24);
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QToolButton *spreadButton = new QToolButton(this);
    QToolButton *minimizeButton = new QToolButton(this);
    QToolButton *closeButton = new QToolButton(this);

    spreadButton->setIconSize(QSize(16, 16));
    minimizeButton->setIconSize(QSize(16, 16));
    closeButton->setIconSize(QSize(16, 16));

    spreadButton->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    minimizeButton->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    closeButton->setIcon(QIcon::fromTheme("window-close-symbolic"));

    spreadButton->setToolTip(tr("Spread"));
    minimizeButton->setToolTip(tr("Minimize"));
    closeButton->setToolTip(tr("Close"));

    auto palette = qApp->palette();
    palette.setColor(QPalette::Highlight, QColor("#E54A50"));
    closeButton->setPalette(palette);

    layout->addWidget(m_iconLabel, 1, Qt::AlignLeft);
    layout->addWidget(m_titleLabel, 9, Qt::AlignLeft);
    layout->addWidget(spreadButton, 1, Qt::AlignRight);
    layout->addWidget(minimizeButton, 1, Qt::AlignRight);
    layout->addWidget(closeButton, 1, Qt::AlignRight);

    m_buttonList.append(spreadButton);
    m_buttonList.append(minimizeButton);
    m_buttonList.append(closeButton);

    for (QToolButton *button : m_buttonList) {
        button->setFixedSize(36, 36);
        button->setAutoRaise(true);

        button->setProperty("isWindowButton", 1);
        button->setProperty("useIconHighlightEffect", 0x2);
    }
    closeButton->setProperty("isWindowButton", 2);
    closeButton->setProperty("useIconHighlightEffect", 0x8);

    this->addWidget(mainWidget);

    spreadButton->hide();

    connect(spreadButton, &QToolButton::clicked, this, [=] {
        Q_EMIT buttonClicked(ButtonType::Spread_Button);
        m_titleLabel->setText(m_title);
        resetTitle();
    });

    connect(minimizeButton, &QToolButton::clicked, this, [=] {
        Q_EMIT buttonClicked(ButtonType::Minimize_Button);
    });

    connect(closeButton, &QToolButton::clicked, this, [=] {
        Q_EMIT buttonClicked(ButtonType::Close_Button);
    });
}

void HeaderBar::setTitle(const QString &title)
{
    m_title = title;
    m_titleLabel->setText(m_title);
    m_titleLabel->setToolTip(m_title);
}

void HeaderBar::setIcon(const QString &iconName)
{
    QIcon icon = QIcon::fromTheme(iconName, QIcon::fromTheme("text-x-generic"));
    m_iconLabel->setPixmap(icon.pixmap(24, 24));
}

void HeaderBar::paintEvent(QPaintEvent *event)
{
    resetTitle();
    QToolBar::paintEvent(event);
}

void HeaderBar::resetTitle()
{
    quint32 titleLength = m_titleLabel->fontMetrics().width(m_title);
    //左右边距8，组件间距4，按钮组件36px宽 (8*2 + 4*4 + 36*4) = 176
    quint32 labelLength = this->width() - 176;

    if (titleLength > labelLength) {
        QString title = m_titleLabel->fontMetrics().elidedText(m_title ,Qt::ElideMiddle ,labelLength);
        m_titleLabel->setText(title);
    }
}
