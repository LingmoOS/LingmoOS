/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 */
#include <QProcess>
#include <QDomDocument>
#include <QProcessEnvironment>
#include <QWidget>
#include <QDBusReply>
#include <QDBusArgument>
#include <QFile>
#include <QDebug>
#include "settings-search-plugin.h"
#include "file-utils.h"
#include "data-collecter.h"

using namespace LingmoUISearch;
static SettingsMatch * settingMatchClass = nullptr;
size_t SettingsSearchPlugin::m_uniqueSymbolForSettings = 0;
QMutex SettingsSearchPlugin::m_mutex;

SettingsSearchPlugin::SettingsSearchPlugin(QObject *parent) : QObject(parent)
{
    SearchPluginIface::Actioninfo open { 0, tr("Open")};
    m_actionInfo << open;
    SettingsMatch::getInstance()->start();
    m_pool.setMaxThreadCount(1);
    m_pool.setExpiryTimeout(1000);
    initDetailPage();
}

const QString SettingsSearchPlugin::name()
{
    return "Settings Search";
}

const QString SettingsSearchPlugin::description()
{
    return tr("Settings search.");
}

QString SettingsSearchPlugin::getPluginName()
{
    return tr("Settings Search");
}

void LingmoUISearch::SettingsSearchPlugin::KeywordSearch(QString keyword, DataQueue<ResultInfo> *searchResult)
{
    m_mutex.lock();
    ++m_uniqueSymbolForSettings;
    m_mutex.unlock();
    SettingsSearch *settingSearch = new SettingsSearch(searchResult, keyword, m_uniqueSymbolForSettings);
    m_pool.start(settingSearch);
}

void SettingsSearchPlugin::stopSearch()
{
    m_mutex.lock();
    ++m_uniqueSymbolForSettings;
    m_mutex.unlock();
}

QList<SearchPluginIface::Actioninfo> SettingsSearchPlugin::getActioninfo(int type)
{
    Q_UNUSED(type)
    return m_actionInfo;
}

void SettingsSearchPlugin::openAction(int actionkey, QString key, int type)
{
    Q_UNUSED(type)
    //TODO add some return message here.
    switch (actionkey) {
    case 0:
    {
        DataCollecter::collectLaunchEvent(QStringLiteral("settingsSearch"), QStringLiteral("open"));
        bool res(false);
        QDBusInterface * appLaunchInterface = new QDBusInterface(QStringLiteral("com.lingmo.ProcessManager"),
                                                                 QStringLiteral("/com/lingmo/ProcessManager/AppLauncher"),
                                                                 QStringLiteral("com.lingmo.ProcessManager.AppLauncher"),
                                                                 QDBusConnection::sessionBus());
        if(!appLaunchInterface->isValid()) {
            qWarning() << qPrintable(QDBusConnection::sessionBus().lastError().message());
            res = false;
        } else {
            appLaunchInterface->setTimeout(10000);
            QDBusReply<void> reply = appLaunchInterface->call("LaunchAppWithArguments", "/usr/share/applications/lingmo-control-center.desktop", QStringList() << "-m" << key.toLower());
            if(reply.isValid()) {
                res = true;
            } else {
                qWarning() << "ProcessManager dbus called failed!" << reply.error();
                res = false;
            }
        }
        if (appLaunchInterface) {
            delete appLaunchInterface;
        }
        appLaunchInterface = nullptr;
        if (res)
            break;
        //打开控制面板对应页面
        QProcess::startDetached("lingmo-control-center", {"-m", key.toLower()});
        break;
    }
    default:
        break;
    }
}

QWidget *SettingsSearchPlugin::detailPage(const ResultInfo &ri)
{
    m_currentActionKey = ri.actionKey;
    m_iconLabel->setPixmap(ri.icon.pixmap(120, 120));
    QFontMetrics fontMetrics = m_nameLabel->fontMetrics();
    QString showname = fontMetrics.elidedText(ri.name, Qt::ElideRight, 215); //当字体长度超过215时显示为省略号
    m_nameLabel->setText(FileUtils::setAllTextBold(showname));
    if(QString::compare(showname, ri.name)) {
        m_nameLabel->setToolTip(ri.name);
    } else {
        m_nameLabel->setToolTip("");
    }
    return m_detailPage;
}

//bool SettingsSearchPlugin::isPreviewEnable(QString key, int type)
//{
//    return false;
//}

//QWidget *SettingsSearchPlugin::previewPage(QString key, int type, QWidget *parent = nullptr)
//{
//    return nullptr;
//}

void SettingsSearchPlugin::initDetailPage()
{
    m_detailPage = new QWidget();
    m_detailPage->setFixedWidth(360);
    m_detailPage->setAttribute(Qt::WA_TranslucentBackground);
    m_detailLyt = new QVBoxLayout(m_detailPage);
    m_detailLyt->setContentsMargins(8, 0, 16, 0);
    m_iconLabel = new QLabel(m_detailPage);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setFixedHeight(128);

    m_nameFrame = new QFrame(m_detailPage);
    m_nameFrameLyt = new QHBoxLayout(m_nameFrame);
    m_nameFrame->setLayout(m_nameFrameLyt);
    m_nameFrameLyt->setContentsMargins(8, 0, 0, 0);
    m_nameLabel = new QLabel(m_nameFrame);
    m_nameLabel->setMaximumWidth(280);
    m_pluginLabel = new QLabel(m_nameFrame);
    m_pluginLabel->setText(tr("Settings"));
    m_pluginLabel->setEnabled(false);
    m_nameFrameLyt->addWidget(m_nameLabel);
    m_nameFrameLyt->addStretch();
    m_nameFrameLyt->addWidget(m_pluginLabel);

    m_line_1 = new SeparationLine(m_detailPage);

    m_actionFrame = new QFrame(m_detailPage);
    m_actionFrameLyt = new QVBoxLayout(m_actionFrame);
    m_actionFrameLyt->setContentsMargins(8, 0, 0, 0);
    m_actionLabel1 = new ActionLabel(tr("Open"), m_currentActionKey, m_actionFrame);

    m_actionFrameLyt->addWidget(m_actionLabel1);;
    m_actionFrame->setLayout(m_actionFrameLyt);

    m_detailLyt->addSpacing(50);
    m_detailLyt->addWidget(m_iconLabel);
    m_detailLyt->addWidget(m_nameFrame);
    m_detailLyt->addWidget(m_line_1);
    m_detailLyt->addWidget(m_actionFrame);
    m_detailPage->setLayout(m_detailLyt);
    m_detailLyt->addStretch();

    connect(m_actionLabel1, &ActionLabel::actionTriggered, [ & ](){
        openAction(0, m_currentActionKey, 0);
    });
}

SettingsSearch::SettingsSearch(DataQueue<SearchPluginIface::ResultInfo> *searchResult, const QString &keyword, size_t uniqueSymbol)
{
    this->setAutoDelete(true);
    this->m_searchResult = searchResult;
    this->m_keyword = keyword;
    this->m_uniqueSymbol = uniqueSymbol;
}

void SettingsSearch::run()
{
    SettingsMatch::getInstance()->startSearch(m_keyword, m_uniqueSymbol, m_searchResult);
}

SettingsMatch *SettingsMatch::getInstance()
{
    if (!settingMatchClass) {
        settingMatchClass = new SettingsMatch;
    }
    return settingMatchClass;
}

void SettingsMatch::run()
{
//    this->initDataOfXml();
    this->initData();
}

SettingsMatch::SettingsMatch()
{
    m_interface = new QDBusInterface("org.lingmo.ukcc.search",
                                     "/",
                                     "org.lingmo.ukcc.search.interface", QDBusConnection::sessionBus(), this);
    if (m_interface->isValid()) {
        connect(m_interface, SIGNAL(searchItemsAdd(QVariantMap)), this, SLOT(addItem(QVariantMap)));
        connect(m_interface, SIGNAL(searchItemsDelete(QVariantMap)), this, SLOT(removeItem(QVariantMap)));
    } else {
        qCritical() << "Ukcc interface error:" << m_interface->lastError();
    }
}

/**
 * @brief SettingsMatch::initData
 * 将控制面板的dbus接口的设置项信息读到内存
 */

void SettingsMatch::initData()
{
    QDBusReply<QVariantMap> reply = m_interface->call("getSearchItems");
    if (reply.isValid()) {
        this->parsingArgsOfDbus(reply.value(), HandleType::Add);
    } else {
        qWarning() << "Fail to call ukcc's getSearchItems.";
    }
}

void SettingsMatch::parsingArgsOfDbus(QVariantMap replyMap, HandleTypes type)
{
    for (std::pair<QString, QVariant> it : replyMap.toStdMap()) {
        const QDBusArgument &dbusArgs = it.second.value<QDBusArgument>();
        dbusArgs.beginArray();
        while (!dbusArgs.atEnd()) {
            QVariant tmp;
            dbusArgs >> tmp;
            const QDBusArgument &args = tmp.value<QDBusArgument>();
            QVariantMap itemInfo;
            args >> itemInfo;
            this->handleData(itemInfo, type);
        }
        dbusArgs.endArray();
    }
}

void SettingsMatch::handleData(const QVariantMap &itemInfo, HandleTypes type)
{
    QString enNameOfModule = itemInfo.value("superordinate").toString();
    QString enNameOfPlugin = itemInfo.value("plugin").toString();
    QString enNameOfFunc = itemInfo.value("subEnglish").toString();

    QString dataKey= enNameOfModule + "/" + enNameOfPlugin;
    QString localName = itemInfo.value("plugin" + QLocale::system().name()).toString();
    QStringList englishSearchResult = m_searchMap[enNameOfModule].value(enNameOfPlugin);

    if (type == HandleType::Add) {
        englishSearchResult.append(enNameOfFunc);
        //二级目录
        this->add2DataMap(dataKey, enNameOfPlugin, localName);
        //三级目录
        dataKey = dataKey + "/" + enNameOfFunc;
        localName = itemInfo.value("sub" + QLocale::system().name()).toString();
        this->add2DataMap(dataKey, enNameOfFunc, localName);
    }

    if (type == HandleType::Delete) {
        englishSearchResult.removeOne(enNameOfFunc);
        m_dataMap.remove(dataKey + "/" + enNameOfFunc);
        if (englishSearchResult.isEmpty()) {
            m_dataMap.remove(dataKey);
            m_searchMap[enNameOfModule].remove(enNameOfPlugin);
            return;
        }
    }
    //update searchMap
    m_searchMap[enNameOfModule].insert(enNameOfPlugin, englishSearchResult);

}

void SettingsMatch::add2DataMap(const QString &key, const QString &enName, const QString &localName)
{
    if (m_dataMap.contains(key)) {
        return;
    }

    QStringList aItemInfo{enName, localName};
    if (QLocale::system().language() == QLocale::Chinese) {
        aItemInfo << FileUtils::findMultiToneWords(localName);
    }

    m_dataMap.insert(key, aItemInfo);
}

/**
 * @brief SettingsMatch::initDataOfXml
 * 将xml文件内容读到内存
 */

void SettingsMatch::initDataOfXml(){
    QFile file(QString::fromLocal8Bit("/usr/share/lingmo-control-center/shell/res/search.xml"));//xml文件所在路径
    if(!file.open(QIODevice::ReadOnly)) {
        return;
    }
    QDomDocument doc;
    doc.setContent(&file);
    file.close();

    QDomElement root = doc.documentElement();
    QDomNode node = root.previousSibling();
    node = root.firstChild();
    matchNodes(node);
}

bool SettingsMatch::matchCommonEnvironment(QDomNode childNode)
{
    if (childNode.nodeName() == QString::fromLocal8Bit("Environment")) {
        QString environment = QProcessEnvironment::systemEnvironment().value("XDG_SESSION_TYPE");
        QString version = childNode.toElement().text();
        if ((version == "v101" && environment == "wayland")
                || (version == "hw990" && environment == "x11")) {
            return false;
        }
    }
    return true;
}

void SettingsMatch::matchNodes(QDomNode node){
    QString chineseIndex;
    QString englishIndex;
    QStringList englishSearchResult;

    while (!node.isNull()) {
        QDomElement element = node.toElement();
        QDomNodeList list = element.childNodes();

        QString enNameOfModule = element.attribute("name");
        QString enNameOfPlugin = list.at(0).nextSiblingElement("EnglishFunc1").toElement().text();
        QString enNameOfFunc = list.at(0).nextSiblingElement("EnglishFunc2").toElement().text();

        englishSearchResult = m_searchMap[enNameOfModule].value(enNameOfPlugin);

        for (int i = 0; i < list.count(); ++i) {
            QDomNode childNode = list.at(i);
            QStringList aItemInfo;

            chineseIndex = childNode.toElement().text();

            if (!matchCommonEnvironment(childNode)) {
                break;
            }
            //TODO: Refactoring this part of code.
            if (childNode.nodeName() == QString::fromLocal8Bit("ChinesePlugin")) {
                englishIndex = enNameOfModule;
                aItemInfo << englishIndex << chineseIndex << FileUtils::findMultiToneWords(chineseIndex);
                m_dataMap.insert(enNameOfModule, aItemInfo);
            }

            if (childNode.nodeName() == QString::fromLocal8Bit("ChineseFunc1")) {
                englishIndex = enNameOfPlugin;
                aItemInfo << englishIndex << chineseIndex << FileUtils::findMultiToneWords(chineseIndex);
                m_dataMap.insert(enNameOfModule + "/" + enNameOfPlugin, aItemInfo);
            }

            if (childNode.nodeName() == QString::fromLocal8Bit("ChineseFunc2")) {
                englishIndex = enNameOfFunc;
                englishSearchResult.append(englishIndex);
                aItemInfo << englishIndex << chineseIndex << FileUtils::findMultiToneWords(chineseIndex);
                m_dataMap.insert(enNameOfModule + "/" + enNameOfPlugin+ "/" + englishIndex, aItemInfo);
            }
        }

        m_searchMap[enNameOfModule].insert(enNameOfPlugin, englishSearchResult);
        node = node.nextSibling();
    }
}

void SettingsMatch::startSearch(QString &keyword, size_t uniqueSymbol, DataQueue<SearchPluginIface::ResultInfo> *searchResult)
{
    if (keyword == "/") {
        return;
    }
    //根据searchMap的对应关系匹配dataMap中的数据信息
    QStringList thirdLvKeys;
    for (auto i = m_searchMap.constBegin(); i != m_searchMap.constEnd(); ++i) {
        QMap<QString, QStringList> regMatch = *i;
        QString moduleName = i.key();
        for (auto t = regMatch.begin(); t != regMatch.end(); ++t) {
            //匹配二级菜单信息
            QString pluginName = t.key();
            QString keyOfSecondLv =QString("%1/%2").arg(moduleName, pluginName);
            bool isSecondLv = matchDataMap(keyOfSecondLv, keyword);

            //匹配对应三级菜单信息
            for (const QString& funcName: t.value()) {
                QString keyOfThirdLv = keyOfSecondLv + QString("/%1").arg(funcName);
                if (isSecondLv) {
                    SearchPluginIface::ResultInfo resultInfo;
                    resultInfo.type = 0;
                    createResultInfo(resultInfo, m_dataMap.value(keyOfThirdLv), keyOfThirdLv);
                    if (!enqueueResultInfo(resultInfo, uniqueSymbol, searchResult)) {
                        return;
                    }
                } else if (matchDataMap(keyOfThirdLv, keyword)) {
                    thirdLvKeys.append(keyOfThirdLv);
                }
            }
        }
    }
    //单独处理三级
    for (const QString& key : thirdLvKeys) {
        SearchPluginIface::ResultInfo resultInfo;
        resultInfo.type = 0;
        createResultInfo(resultInfo, m_dataMap.value(key), key);
        if (!enqueueResultInfo(resultInfo, uniqueSymbol, searchResult)) {
            return;
        }
    }
}

void SettingsMatch::addItem(QVariantMap replyMap)
{
    this->parsingArgsOfDbus(replyMap, HandleType::Add);
}

void SettingsMatch::removeItem(QVariantMap replyMap)
{
    this->parsingArgsOfDbus(replyMap, HandleType::Delete);
}

bool SettingsMatch::matchDataMap(QString &key, QString &keyword)
{
    bool res(false);
    SearchPluginIface::ResultInfo resultInfo;
    resultInfo.type = 0;
    for (int counter = 0; counter < m_dataMap.value(key).size(); ++counter) {
        QString data = m_dataMap.value(key).at(counter);
        //关键字为单字不搜拼音
        if (keyword.size() < 2 and counter > 1 and counter % 2 == 0) {
            continue;
        }
        if (data.contains(keyword, Qt::CaseInsensitive)) {
            res = true;
            break;
        }
    }
    return res;
}

void SettingsMatch::createResultInfo(SearchPluginIface::ResultInfo &resultInfo, const QStringList &itemInfo, const QString &path)
{
    QLocale ql;
    resultInfo.name = ql.language() == QLocale::English ? itemInfo.at(0) : itemInfo.at(1);
    if (path.split("/").size() == 3) {
        QStringList topInfo = m_dataMap.value(path.left(path.lastIndexOf("/")));
        resultInfo.name.prepend("--");
        resultInfo.name.prepend(ql.language() == QLocale::English ? topInfo.at(0) : topInfo.at(1));
    }
    resultInfo.toolTip = resultInfo.name;
    resultInfo.resourceType = QStringLiteral("setting");
    resultInfo.icon = FileUtils::getSettingIcon();
    resultInfo.actionKey = path.section("/", 1, 1);
}

bool SettingsMatch::enqueueResultInfo(SearchPluginIface::ResultInfo &resultInfo, size_t uniqueSymbol, DataQueue<SearchPluginIface::ResultInfo> *searchResult) {
    bool res(false);
    SettingsSearchPlugin::m_mutex.lock();
    if (uniqueSymbol == SettingsSearchPlugin::m_uniqueSymbolForSettings) {
        res = true;
        searchResult->enqueue(resultInfo);
        SettingsSearchPlugin::m_mutex.unlock();
    } else {
        SettingsSearchPlugin::m_mutex.unlock();
    }
    return res;
}
