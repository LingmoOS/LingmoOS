// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "oceanuimanager.h"

#include "oceanuiapp.h"
#include "oceanuiobject_p.h"
#include "navigationmodel.h"
#include "pluginmanager.h"
#include "searchmodel.h"

#include <DGuiApplicationHelper>

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QElapsedTimer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>
#include <QTranslator>
#include <QWindow>

DCORE_USE_NAMESPACE

namespace oceanuiV25 {
static Q_LOGGING_CATEGORY(oceanuiLog, "ocean.oceanui.manager");

const QString WidthConfig = QStringLiteral("width");
const QString HeightConfig = QStringLiteral("height");
const QString HideConfig = QStringLiteral("hideModule");
const QString DisableConfig = QStringLiteral("disableModule");
const QString ControlCenterIcon = QStringLiteral("preferences-system");
const QString ControlCenterGroupName = "com.lingmo.ocean-grand-search.group.ocean-control-center-setting";

OceanUIManager::OceanUIManager(QObject *parent)
    : OceanUIApp(parent)
    , m_root(new OceanUIObject(this))
    , m_activeObject(m_root)
    , m_hideObjects(new OceanUIObject(this))
    , m_noAddObjects(new OceanUIObject(this))
    , m_noParentObjects(new OceanUIObject(this))
    , m_plugins(new PluginManager(this))
    , m_window(nullptr)
    , m_dconfig(DConfig::create("org.lingmo.ocean.control-center", "org.lingmo.ocean.control-center", QString(), this))
    , m_engine(nullptr)
    , m_navModel(new NavigationModel(this))
    , m_searchModel(new SearchModel(this))
{
    m_hideObjects->setName("_hide");
    m_noAddObjects->setName("_noAdd");
    m_noParentObjects->setName("_noParent");

    m_root->setName("root");
    m_root->setCanSearch(false);
    m_currentObjects.append(m_root);
    onObjectAoceand(m_root);

    initConfig();
    connect(m_plugins, &PluginManager::addObject, this, &OceanUIManager::addObject, Qt::QueuedConnection);
    connect(qApp, &QCoreApplication::aboutToQuit, this, &OceanUIManager::onQuit);
    showPage("system");
}

OceanUIManager::~OceanUIManager()
{
    qCDebug(oceanuiLog()) << "delete oceanuiManger";
    onQuit();
    delete m_plugins;
    qCDebug(oceanuiLog()) << "delete oceanuiManger end";
}

bool OceanUIManager::installTranslator(const QString &name)
{
    QTranslator *translator = new QTranslator();
    if (translator->load(QLocale(), name, "_", TRANSLATE_READ_DIR)) {
        qApp->installTranslator(translator);
#if 1 // 兼容旧版位置
    } else if (translator->load(QLocale(), name, "_", TRANSLATE_READ_DIR "/..")) {
        qApp->installTranslator(translator);
#endif
    } else {
        delete translator;
        qCWarning(oceanuiLog()) << "install translator fail:" << name << ", dir:" << TRANSLATE_READ_DIR;
        return false;
    }
    return true;
}

void OceanUIManager::init()
{
    if (m_engine)
        return;

    m_engine = new QQmlApplicationEngine(this);
    auto paths = m_engine->importPathList();
    paths.prepend(DefaultModuleDirectory);
    m_engine->setImportPathList(paths);
}

QQmlApplicationEngine *OceanUIManager::engine()
{
    return m_engine;
}

void OceanUIManager::setMainWindow(QWindow *window)
{
    m_window = window;
}

void OceanUIManager::loadModules(bool async, const QStringList &dirs)
{
    // onAddModule(m_rootModule);
    m_plugins->loadModules(m_root, async, dirs);
    // showModule(m_rootModule);
}

int OceanUIManager::width() const
{
    auto w = m_dconfig->value(WidthConfig).toInt();
    return w > 780 ? w : 780;
}

int OceanUIManager::height() const
{
    auto h = m_dconfig->value(HeightConfig).toInt();
    return h > 530 ? h : 530;
}

OceanUIApp::UosEdition OceanUIManager::uosEdition() const
{
    LSysInfo::UosEdition edition = LSysInfo::uosEditionType();
    return OceanUIApp::UosEdition(edition);
}

bool OceanUIManager::isTreeland() const
{
    return Dtk::Gui::DGuiApplicationHelper::testAttribute(Dtk::Gui::DGuiApplicationHelper::IsWaylandPlatform);
}

OceanUIObject *OceanUIManager::object(const QString &name)
{
    return findObject(name);
}

inline void noRepeatAdd(QVector<OceanUIObject *> &list, OceanUIObject *obj)
{
    if (!list.contains(obj)) {
        list.append(obj);
    }
}

void OceanUIManager::addObject(OceanUIObject *obj)
{
    if (!obj)
        return;
    QVector<OceanUIObject *> objs;
    objs.append(obj);
    while (!objs.isEmpty()) {
        OceanUIObject *o = objs.takeFirst();
        if (o->parentName().isEmpty()) {
            OceanUIObject::Private::FromObject(m_noParentObjects)->addChild(o, false);
        } else {
            if (contains(m_hideModule, o)) {
                OceanUIObject::Private::FromObject(o)->setFlagState(DCC_CONFIG_HIDDEN, true);
            }
            if (contains(m_disableModule, o)) {
                OceanUIObject::Private::FromObject(o)->setFlagState(DCC_CONFIG_DISABLED, true);
            }
            if (!o->isVisibleToApp()) {
                connect(o, &OceanUIObject::visibleToAppChanged, this, &OceanUIManager::onVisible, Qt::ConnectionType(Qt::QueuedConnection | Qt::UniqueConnection));
                OceanUIObject::Private::FromObject(m_hideObjects)->addChild(o, false);
            } else if (!addObjectToParent(o)) {
                OceanUIObject::Private::FromObject(m_noAddObjects)->addChild(o, false);
            }
        }

        objs.append(OceanUIObject::Private::FromObject(o)->getObjects());
    }
    // 处理m_noAddObject
    objs.append(m_noAddObjects->getChildren());
    while (!objs.isEmpty()) {
        OceanUIObject *o = objs.takeFirst();
        if (OceanUIObject *parentObj = findParent(o)) {
            OceanUIObject::Private::FromObject(m_noAddObjects)->removeChild(o);
            OceanUIObject::Private::FromObject(parentObj)->addChild(o);
            objs = m_noAddObjects->getChildren();
        }
    }
}

void OceanUIManager::removeObject(OceanUIObject *obj)
{
    if (!obj)
        return;
    removeObjectFromParent(obj);
}

void OceanUIManager::removeObject(const QString &name)
{
    removeObject(findObject(name));
}

void OceanUIManager::showPage(const QString &url)
{
    if (this->calledFromDBus()) {
        auto message = this->message();
        setDelayedReply(true);
        QMetaObject::invokeMethod(this, &OceanUIManager::waitShowPage, Qt::QueuedConnection, url, message);
        // show(); // 先查找再显示则注释掉此处
    } else {
        QMetaObject::invokeMethod(this, &OceanUIManager::waitShowPage, Qt::QueuedConnection, url, QDBusMessage());
    }
}

void OceanUIManager::showPage(OceanUIObject *obj)
{
    QMetaObject::invokeMethod(this, "doShowPage", Qt::QueuedConnection, obj, QString());
}

void OceanUIManager::showPage(OceanUIObject *obj, const QString &cmd)
{
    QMetaObject::invokeMethod(this, "doShowPage", Qt::QueuedConnection, obj, cmd);
}

void OceanUIManager::toBack()
{
    int row = m_navModel->rowCount();
    if (row < 3) {
        showPage(m_root);
    } else {
        row = row - 3;
        QString url = m_navModel->data(m_navModel->index(row, 0), NavigationModel::NavUrlRole).toString();
        if (!url.isEmpty()) {
            showPage(url);
        }
    }
}

QWindow *OceanUIManager::mainWindow() const
{
    return m_window;
}

void OceanUIManager::showHelp()
{
    QString helpTitle;
    if (1 < m_currentObjects.count())
        helpTitle = m_currentObjects.last()->name();
    if (helpTitle.isEmpty())
        helpTitle = "controlcenter";

    const QString &dmanInterface = "com.lingmo.Manual.Open";
    QDBusMessage message = QDBusMessage::createMethodCall(dmanInterface, "/com/lingmo/Manual/Open", dmanInterface, "OpenTitle");
    message << "ocean" << helpTitle;
    QDBusConnection::sessionBus().asyncCall(message);
}

QString OceanUIManager::search(const QString &json)
{
    QJsonDocument jsonDocument = QJsonDocument::fromJson(json.toLocal8Bit().data());
    if (!jsonDocument.isNull()) {
        QJsonObject jsonObject = jsonDocument.object();

        // 处理搜索任务, 返回搜索结果
        QJsonArray items;
        m_searchModel->setFilterRegularExpression(jsonObject.value("cont").toString());
        qCDebug(oceanuiLog()) << "search key:" << jsonObject.value("cont").toString();
        for (int i = 0; i < m_searchModel->rowCount(); ++i) {
            QJsonObject jsonObj;
            jsonObj.insert("item", m_searchModel->data(m_searchModel->index(i, 0), SearchModel::SearchUrlRole).toString());
            jsonObj.insert("name", m_searchModel->data(m_searchModel->index(i, 0), SearchModel::SearchPlainTextRole).toString());
            jsonObj.insert("icon", ControlCenterIcon);
            jsonObj.insert("type", "application/x-ocean-control-center-xx");
            qCDebug(oceanuiLog()) << "search results:" << jsonObj["name"].toString();
            items.insert(i, jsonObj);
        }

        QJsonObject objCont;
        objCont.insert("group", ControlCenterGroupName);
        objCont.insert("items", items);

        QJsonArray arrConts;
        arrConts.insert(0, objCont);

        QJsonObject jsonResults;
        jsonResults.insert("ver", jsonObject.value("ver"));
        jsonResults.insert("mID", jsonObject.value("mID"));
        jsonResults.insert("cont", arrConts);

        QJsonDocument document;
        document.setObject(jsonResults);

        return document.toJson(QJsonDocument::Compact);
    }

    return QString();
}

bool OceanUIManager::stop(const QString &)
{
    return true;
}

bool OceanUIManager::action(const QString &json)
{
    QString searchName;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(json.toLocal8Bit().data());
    if (!jsonDocument.isNull()) {
        QJsonObject jsonObject = jsonDocument.object();
        if (jsonObject.value("action") == "openitem") {
            // 打开item的操作
            searchName = jsonObject.value("item").toString();
        }
    }

    mainWindow()->show();
    mainWindow()->requestActivate();
    showPage(searchName);
    return true;
}

QString OceanUIManager::GetAllModule()
{
    auto message = this->message();
    setDelayedReply(true);
    QMetaObject::invokeMethod(this, &OceanUIManager::doGetAllModule, Qt::QueuedConnection, message);
    return QString();
}

QAbstractItemModel *OceanUIManager::navModel() const
{
    return m_navModel;
}

QSortFilterProxyModel *OceanUIManager::searchModel() const
{
    return m_searchModel;
}

void OceanUIManager::show()
{
    QWindow *w = mainWindow();
    if (w->windowStates() == Qt::WindowMinimized || !w->isVisible())
        w->showNormal();
    w->requestActivate();
}

void OceanUIManager::initConfig()
{
    if (!m_dconfig->isValid()) {
        qCWarning(oceanuiLog()) << QString("DConfig is invalide, name:[%1], subpath[%2].").arg(m_dconfig->name(), m_dconfig->subpath());
        return;
    }

    updateModuleConfig(HideConfig);
    updateModuleConfig(DisableConfig);
    connect(m_dconfig, &DConfig::valueChanged, this, &OceanUIManager::updateModuleConfig);
}

bool OceanUIManager::contains(const QSet<QString> &urls, const OceanUIObject *obj)
{
    for (auto &&url : urls) {
        if (isEqual(url, obj))
            return true;
    }
    return false;
}

bool OceanUIManager::isEqual(const QString &url, const OceanUIObject *obj)
{
    QString path = "/" + url;
    QString objPath = "/" + obj->parentName() + "/" + obj->name();
    for (auto it = path.rbegin(), itObj = objPath.rbegin(); it != path.rend() && itObj != objPath.rend(); ++it, ++itObj) {
        if (*it != *itObj) {
            return false;
        }
    }
    return true;
}

OceanUIObject *OceanUIManager::findObject(const QString &url, bool onlyRoot)
{
    if (!m_root) {
        return nullptr;
    }
    QString path = url;
    if (path.startsWith("/")) {
        path = path.mid(1);
    }
    QVector<QVector<OceanUIObject *>> objs;
    objs.append({ m_root });
    if (!onlyRoot) {
        objs.append(m_hideObjects->getChildren());
    }
    while (!objs.isEmpty()) {
        QVector<OceanUIObject *> subObjs = objs.takeFirst();
        while (!subObjs.isEmpty()) {
            OceanUIObject *obj = subObjs.takeFirst();
            if (isEqual(path, obj)) {
                return obj;
            }
            subObjs.append(obj->getChildren());
        }
    }
    return nullptr;
}

OceanUIObject *OceanUIManager::findParent(const OceanUIObject *obj)
{
    const QString &path = obj->parentName();
    OceanUIObject *p = qobject_cast<OceanUIObject *>(obj->parent());
    if (p) {
        if (isEqual(path, p)) {
            return p;
        }
        p = qobject_cast<OceanUIObject *>(p->parent());
        if (p && isEqual(path, p)) {
            return p;
        }
    }
    return findObject(path);
}

void OceanUIManager::waitShowPage(const QString &url, const QDBusMessage message)
{
    qCInfo(oceanuiLog()) << "show page:" << url;
    OceanUIObject *obj = nullptr;
    QString cmd;
    if (url.isEmpty()) {
        obj = m_root;
        showPage(obj, QString());
    } else {
        int i = url.indexOf('?');
        cmd = i != -1 ? url.mid(i + 1) : QString();
        QString path = url.mid(0, i).split('/', Qt::SkipEmptyParts).join('/'); // 移除多余的/
        obj = findObject(path, true);
        if (obj) {
            showPage(obj, cmd);
        } else if (!m_plugins->loadFinished()) {
            QEventLoop loop;
            QTimer timer;
            connect(&timer, &QTimer::timeout, this, [this, &path, &loop]() {
                OceanUIObject *obj = findObject(path, true);
                if (obj) {
                    loop.quit();
                }
            });
            connect(m_plugins, &PluginManager::loadAllFinished, &loop, &QEventLoop::quit);
            timer.start(10);
            loop.exec();
            obj = findObject(path, true);
            if (obj) {
                showPage(obj, cmd);
            }
        }
    }
    if (message.type() != QDBusMessage::InvalidMessage) {
        if (obj) {
            if (cmd.isEmpty()) {
                show();
            }
            QDBusConnection::sessionBus().send(message.createReply());
        } else {
            QDBusConnection::sessionBus().send(message.createErrorReply(QDBusError::InvalidArgs, QString("not found url:") + url));
        }
    }
}

void OceanUIManager::doShowPage(OceanUIObject *obj, const QString &cmd)
{
    if (m_plugins->isDeleting() || !obj) {
        return;
    }
    // 禁用首页
    if (obj == m_root) {
        if (m_root->getChildren().isEmpty()) {
            return;
        }
        obj = m_root->getChildren().first();
    }
    if (m_activeObject == obj && cmd.isEmpty()) {
        return;
    }
    if (!cmd.isEmpty()) {
        Q_EMIT obj->active(cmd);
        return;
    }
    QList<OceanUIObject *> modules;
    OceanUIObject *triggeredObj = obj;
    if (triggeredObj->pageType() == OceanUIObject::MenuEditor && !triggeredObj->getChildren().isEmpty()) {
        triggeredObj = triggeredObj->getChildren().first();
    }
    OceanUIObject *tmpObj = triggeredObj;
    while (tmpObj && (tmpObj->pageType() != OceanUIObject::Menu)) { // 页面中的控件，则激活项为父项
        tmpObj = OceanUIObject::Private::FromObject(tmpObj)->getParent();
    }
    if (!tmpObj) {
        return;
    }
    modules.append(tmpObj);
    OceanUIObject *p = OceanUIObject::Private::FromObject(tmpObj)->getParent();
    while (p) {
        p->setCurrentObject(tmpObj);
        Q_EMIT p->active(QString());
        modules.prepend(p);
        tmpObj = p;
        p = OceanUIObject::Private::FromObject(p)->getParent();
    }
    while (!m_currentObjects.isEmpty()) {
        OceanUIObject *oldObj = m_currentObjects.takeLast();
        if (!modules.contains(oldObj)) {
            oldObj->setCurrentObject(nullptr);
        }
        if (oldObj != m_root && oldObj != modules.last()) {
            Q_EMIT oldObj->deactive();
        }
    }
    if (!cmd.isEmpty()) {
        Q_EMIT triggeredObj->active(cmd);
    }
    m_currentObjects = modules;
    if (m_currentObjects.last() != m_activeObject) {
        m_activeObject = m_currentObjects.last();
        Q_EMIT activeObjectChanged(m_activeObject);
    }
    m_navModel->setNavigationObject(m_currentObjects);
    qCInfo(oceanuiLog) << "trigger object:" << triggeredObj->name() << " active object:" << m_activeObject->name() << (void *)(triggeredObj->parentItem());
    if (!(triggeredObj->pageType() & OceanUIObject::Menu) && triggeredObj->parentItem()) {
        Q_EMIT activeItemChanged(triggeredObj->parentItem());
    }
}

QSet<QString> findAddItems(QSet<QString> *oldSet, QSet<QString> *newSet)
{
    QSet<QString> addSet;
    for (auto &&key : *newSet) {
        if (!oldSet->contains(key)) {
            addSet.insert(key);
        }
    }
    return addSet;
}

void OceanUIManager::updateModuleConfig(const QString &key)
{
    QSet<QString> oldModuleConfig;
    QSet<QString> *newModuleConfig = nullptr;
    uint32_t type = DCC_CONFIG_HIDDEN;
    if (key == HideConfig) {
        type = DCC_CONFIG_HIDDEN;
        oldModuleConfig = m_hideModule;
        newModuleConfig = &m_hideModule;
    } else if (key == DisableConfig) {
        type = DCC_CONFIG_DISABLED;
        oldModuleConfig = m_disableModule;
        newModuleConfig = &m_disableModule;
    } else {
        return;
    }
    const auto &list = m_dconfig->value(key).toStringList();
    *newModuleConfig = QSet<QString>(list.begin(), list.end());
    QSet<QString> addModuleConfig = findAddItems(&oldModuleConfig, newModuleConfig);
    QSet<QString> removeModuleConfig = findAddItems(newModuleConfig, &oldModuleConfig);
    for (auto &&url : addModuleConfig) {
        OceanUIObject *obj = findObject(url);
        if (obj) {
            OceanUIObject::Private::FromObject(obj)->setFlagState(type, true);
        }
    }
    for (auto &&url : removeModuleConfig) {
        OceanUIObject *obj = findObject(url);
        if (obj) {
            OceanUIObject::Private::FromObject(obj)->setFlagState(type, false);
        }
    }
    if (newModuleConfig == &m_hideModule && (!addModuleConfig.isEmpty() || !removeModuleConfig.isEmpty())) {
        Q_EMIT hideModuleChanged(m_hideModule);
    }
}

void OceanUIManager::onVisible(bool visible)
{
    if (!m_root) {
        return;
    }
    OceanUIObject *obj = qobject_cast<OceanUIObject *>(sender());
    if (!obj) {
        return;
    }
    if (visible) {
        QVector<OceanUIObject *> objs;
        objs.append(obj->getChildren());
        while (!objs.isEmpty()) {
            auto o = objs.takeFirst();
            if (o->isVisibleToApp()) {
                objs.append(o->getChildren());
            } else {
                connect(o, &OceanUIObject::visibleToAppChanged, this, &OceanUIManager::onVisible, Qt::ConnectionType(Qt::QueuedConnection | Qt::UniqueConnection));
                removeObjectFromParent(o);
                OceanUIObject::Private::FromObject(m_hideObjects)->addChild(o, false);
            }
        }
        OceanUIObject::Private::FromObject(m_hideObjects)->removeChild(obj);
        addObjectToParent(obj);
    } else {
        removeObjectFromParent(obj);
        OceanUIObject::Private::FromObject(m_hideObjects)->addChild(obj, false);
    }
}

void OceanUIManager::onObjectAoceand(OceanUIObject *obj)
{
    if (!m_root) {
        return;
    }
    m_searchModel->addSearchData(obj, QString(), QString());
    QVector<OceanUIObject *> objs;
    objs.append(obj);
    while (!objs.isEmpty()) {
        auto o = objs.takeFirst();
        connect(o, &OceanUIObject::childAoceand, this, &OceanUIManager::onObjectAoceand);
        connect(o, &OceanUIObject::childRemoved, this, &OceanUIManager::onObjectRemoved);
        connect(o, &OceanUIObject::displayNameChanged, this, &OceanUIManager::onObjectDisplayChanged);
        connect(o, &OceanUIObject::visibleToAppChanged, this, &OceanUIManager::onVisible, Qt::ConnectionType(Qt::QueuedConnection | Qt::UniqueConnection));
        objs.append(o->getChildren());
    }
}

void OceanUIManager::onObjectRemoved(OceanUIObject *obj)
{
    if (!m_root) {
        return;
    }
    QVector<OceanUIObject *> objs;
    objs.append(obj);
    while (!objs.isEmpty()) {
        auto o = objs.takeFirst();
        disconnect(o, &OceanUIObject::childAoceand, this, nullptr);
        disconnect(o, &OceanUIObject::childRemoved, this, nullptr);
        disconnect(o, &OceanUIObject::displayNameChanged, this, nullptr);
        m_searchModel->removeSearchData(o, QString());
        objs.append(o->getChildren());
    }
    OceanUIObject *parentObj = m_root;
    for (auto &&o : m_currentObjects) {
        if (o == obj) {
            doShowPage(parentObj, QString());
            break;
        }
        parentObj = o;
    }
}

void OceanUIManager::onObjectDisplayChanged()
{
    if (!m_root) {
        return;
    }
    OceanUIObject *obj = qobject_cast<OceanUIObject *>(sender());
    if (obj) {
        m_searchModel->removeSearchData(obj, QString());
        m_searchModel->addSearchData(obj, QString(), QString());
    }
}

bool OceanUIManager::addObjectToParent(OceanUIObject *obj)
{
    if (OceanUIObject *parentObj = findParent(obj)) {
        OceanUIObject::Private::FromObject(parentObj)->addChild(obj);
        return true;
    }
    return false;
}

bool OceanUIManager::removeObjectFromParent(OceanUIObject *obj)
{
    OceanUIObject *parentObj = OceanUIObject::Private::FromObject(obj)->getParent();
    if (parentObj) {
        OceanUIObject::Private::FromObject(parentObj)->removeChild(obj);
        return true;
    }
    return false;
}

void OceanUIManager::onQuit()
{
    if (m_plugins->isDeleting()) {
        return;
    }
    m_plugins->beginDelete();
    int width_remember = m_window->width();
    int height_remember = m_window->height();

    if (m_dconfig->isValid()) {
        m_dconfig->setValue(WidthConfig, width_remember);
        m_dconfig->setValue(HeightConfig, height_remember);
    }
    m_window->hide();
    m_window->close();
    // doShowPage(m_root, QString());

    // #ifdef QT_DEBUG
    // TODO: delete m_engine会有概率崩溃
    qCDebug(oceanuiLog()) << "delete root begin";
    OceanUIObject *root = m_root;
    m_root = nullptr;
    Q_EMIT rootChanged(m_root);
    delete root;
    qCDebug(oceanuiLog()) << "delete root end";

    qCDebug(oceanuiLog()) << "delete clearData hide:" << m_hideObjects->getChildren().size() << "noAdd:" << m_noAddObjects->getChildren().size() << "noParent" << m_noParentObjects->getChildren().size();
    delete m_noAddObjects;
    delete m_noParentObjects;
    delete m_hideObjects;
    qCDebug(oceanuiLog()) << "delete oceanuiobject";
    qCDebug(oceanuiLog()) << "delete QmlEngine";
    delete m_engine;
    qCDebug(oceanuiLog()) << "clear QmlEngine";
    m_engine = nullptr;
    // #endif
}

void OceanUIManager::waitLoadFinished() const
{
    if (!m_plugins->loadFinished()) {
        QEventLoop loop;
        QTimer timer;
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        connect(m_plugins, &PluginManager::loadAllFinished, &loop, &QEventLoop::quit);
        timer.start(5000);
        loop.exec();
    }
}

void OceanUIManager::doGetAllModule(const QDBusMessage message) const
{
    waitLoadFinished();
    OceanUIObject *root = m_root;
    QList<QPair<OceanUIObject *, QStringList>> modules;
    for (auto &&child : root->getChildren()) {
        modules.append({ child, { child->name(), child->displayName() } });
    }

    QJsonArray arr;
    while (!modules.isEmpty()) {
        const auto &urlInfo = modules.takeFirst();
        QJsonObject obj;
        obj.insert("url", urlInfo.second.at(0));
        obj.insert("displayName", urlInfo.second.at(1));
        obj.insert("weight", (int)(urlInfo.first->weight()));
        arr.append(obj);
        const QList<OceanUIObject *> &children = urlInfo.first->getChildren();
        for (auto it = children.crbegin(); it != children.crend(); ++it)
            modules.prepend({ *it, { urlInfo.second.at(0) + "/" + (*it)->name(), urlInfo.second.at(1) + "/" + (*it)->displayName() } });
    }

    QJsonDocument doc;
    doc.setArray(arr);
    QString json = doc.toJson(QJsonDocument::Compact);
    QDBusConnection::sessionBus().send(message.createReply(json));
}

} // namespace oceanuiV25
