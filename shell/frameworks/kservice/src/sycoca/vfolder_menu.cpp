/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kbuildsycocainterface_p.h"
#include "kservicefactory_p.h"
#include "sycocadebug.h"
#include "vfolder_menu_p.h"

#include <kservice.h>

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QMap>
#include <QStandardPaths>

static void foldNode(QDomElement &docElem, QDomElement &e, QMap<QString, QDomElement> &dupeList, QString s = QString()) // krazy:exclude=passbyvalue
{
    if (s.isEmpty()) {
        s = e.text();
    }
    auto it = dupeList.find(s);
    if (it != dupeList.end()) {
        // qCDebug(SYCOCA) << e.tagName() << "and" << s << "requires combining!";

        docElem.removeChild(*it);
        dupeList.erase(it);
    }
    dupeList.insert(s, e);
}

static void replaceNode(QDomElement &docElem, QDomNode &node, const QStringList &list, const QString &tag)
{
    for (const QString &str : list) {
        QDomElement element = docElem.ownerDocument().createElement(tag);
        const QDomText txt = docElem.ownerDocument().createTextNode(str);
        element.appendChild(txt);
        docElem.insertAfter(element, node);
    }

    QDomNode next = node.nextSibling();
    docElem.removeChild(node);
    node = next;
    //   qCDebug(SYCOCA) << "Next tag = " << n.toElement().tagName();
}

void VFolderMenu::registerFile(const QString &file)
{
    int i = file.lastIndexOf(QLatin1Char('/'));
    if (i < 0) {
        return;
    }

    QString dir = file.left(i + 1); // Include trailing '/'
    registerDirectory(dir);
}

void VFolderMenu::registerDirectory(const QString &directory)
{
    m_allDirectories.append(directory);
}

QStringList VFolderMenu::allDirectories()
{
    if (m_allDirectories.isEmpty()) {
        return m_allDirectories;
    }
    m_allDirectories.sort();

    QStringList::Iterator it = m_allDirectories.begin();
    QString previous = *it++;
    for (; it != m_allDirectories.end();) {
#ifndef Q_OS_WIN
        if ((*it).startsWith(previous))
#else
        if ((*it).startsWith(previous, Qt::CaseInsensitive))
#endif
        {
            it = m_allDirectories.erase(it);
        } else {
            previous = *it;
            ++it;
        }
    }
    return m_allDirectories;
}

static void track(const QString &menuId,
                  const QString &menuName,
                  const QHash<QString, KService::Ptr> &includeList,
                  const QHash<QString, KService::Ptr> &excludeList,
                  const QHash<QString, KService::Ptr> &itemList,
                  const QString &comment)
{
    if (itemList.contains(menuId)) {
        printf("%s: %s INCL %d EXCL %d\n",
               qPrintable(menuName),
               qPrintable(comment),
               includeList.contains(menuId) ? 1 : 0,
               excludeList.contains(menuId) ? 1 : 0);
    }
}

void VFolderMenu::includeItems(QHash<QString, KService::Ptr> &items1, const QHash<QString, KService::Ptr> &items2)
{
    for (const KService::Ptr &p : items2) {
        items1.insert(p->menuId(), p);
    }
}

void VFolderMenu::matchItems(QHash<QString, KService::Ptr> &items1, const QHash<QString, KService::Ptr> &items2)
{
    const QHash<QString, KService::Ptr> tmpItems1 = items1;
    for (const KService::Ptr &p : tmpItems1) {
        QString id = p->menuId();
        if (!items2.contains(id)) {
            items1.remove(id);
        }
    }
}

void VFolderMenu::excludeItems(QHash<QString, KService::Ptr> &items1, const QHash<QString, KService::Ptr> &items2)
{
    for (const KService::Ptr &p : items2) {
        items1.remove(p->menuId());
    }
}

VFolderMenu::SubMenu *VFolderMenu::takeSubMenu(SubMenu *parentMenu, const QString &menuName)
{
    const int i = menuName.indexOf(QLatin1Char('/'));
    const QString s1 = i > 0 ? menuName.left(i) : menuName;
    const QString s2 = menuName.mid(i + 1);

    // Look up menu
    for (QList<SubMenu *>::Iterator it = parentMenu->subMenus.begin(); it != parentMenu->subMenus.end(); ++it) {
        SubMenu *menu = *it;
        if (menu->name == s1) {
            if (i == -1) {
                // Take it out
                parentMenu->subMenus.erase(it);
                return menu;
            } else {
                return takeSubMenu(menu, s2);
            }
        }
    }
    return nullptr; // Not found
}

void VFolderMenu::mergeMenu(SubMenu *menu1, SubMenu *menu2, bool reversePriority)
{
    if (m_track) {
        track(m_trackId, menu1->name, menu1->items, menu1->excludeItems, menu2->items, QStringLiteral("Before MenuMerge w. %1 (incl)").arg(menu2->name));
        track(m_trackId, menu1->name, menu1->items, menu1->excludeItems, menu2->excludeItems, QStringLiteral("Before MenuMerge w. %1 (excl)").arg(menu2->name));
    }
    if (reversePriority) {
        // Merge menu1 with menu2, menu1 takes precedent
        excludeItems(menu2->items, menu1->excludeItems);
        includeItems(menu1->items, menu2->items);
        excludeItems(menu2->excludeItems, menu1->items);
        includeItems(menu1->excludeItems, menu2->excludeItems);
    } else {
        // Merge menu1 with menu2, menu2 takes precedent
        excludeItems(menu1->items, menu2->excludeItems);
        includeItems(menu1->items, menu2->items);
        includeItems(menu1->excludeItems, menu2->excludeItems);
        menu1->isDeleted = menu2->isDeleted;
    }
    while (!menu2->subMenus.isEmpty()) {
        SubMenu *subMenu = menu2->subMenus.takeFirst();
        insertSubMenu(menu1, subMenu->name, subMenu, reversePriority);
    }

    if (reversePriority) {
        // Merge menu1 with menu2, menu1 takes precedent
        if (menu1->directoryFile.isEmpty()) {
            menu1->directoryFile = menu2->directoryFile;
        }
        if (menu1->defaultLayoutNode.isNull()) {
            menu1->defaultLayoutNode = menu2->defaultLayoutNode;
        }
        if (menu1->layoutNode.isNull()) {
            menu1->layoutNode = menu2->layoutNode;
        }
    } else {
        // Merge menu1 with menu2, menu2 takes precedent
        if (!menu2->directoryFile.isEmpty()) {
            menu1->directoryFile = menu2->directoryFile;
        }
        if (!menu2->defaultLayoutNode.isNull()) {
            menu1->defaultLayoutNode = menu2->defaultLayoutNode;
        }
        if (!menu2->layoutNode.isNull()) {
            menu1->layoutNode = menu2->layoutNode;
        }
    }

    if (m_track) {
        track(m_trackId, menu1->name, menu1->items, menu1->excludeItems, menu2->items, QStringLiteral("After MenuMerge w. %1 (incl)").arg(menu2->name));
        track(m_trackId, menu1->name, menu1->items, menu1->excludeItems, menu2->excludeItems, QStringLiteral("After MenuMerge w. %1 (excl)").arg(menu2->name));
    }

    delete menu2;
}

void VFolderMenu::insertSubMenu(SubMenu *parentMenu, const QString &menuName, SubMenu *newMenu, bool reversePriority)
{
    const int i = menuName.indexOf(QLatin1Char('/'));
    const QString s1 = menuName.left(i);
    const QString s2 = menuName.mid(i + 1);

    // Look up menu
    for (SubMenu *menu : std::as_const(parentMenu->subMenus)) {
        if (menu->name == s1) {
            if (i == -1) {
                mergeMenu(menu, newMenu, reversePriority);
                return;
            } else {
                insertSubMenu(menu, s2, newMenu, reversePriority);
                return;
            }
        }
    }
    if (i == -1) {
        // Add it here
        newMenu->name = menuName;
        parentMenu->subMenus.append(newMenu);
    } else {
        SubMenu *menu = new SubMenu;
        menu->name = s1;
        parentMenu->subMenus.append(menu);
        insertSubMenu(menu, s2, newMenu);
    }
}

void VFolderMenu::insertService(SubMenu *parentMenu, const QString &name, KService::Ptr newService)
{
    const int i = name.indexOf(QLatin1Char('/'));

    if (i == -1) {
        // Add it here
        parentMenu->items.insert(newService->menuId(), newService);
        return;
    }

    QString s1 = name.left(i);
    QString s2 = name.mid(i + 1);

    // Look up menu
    for (SubMenu *menu : std::as_const(parentMenu->subMenus)) {
        if (menu->name == s1) {
            insertService(menu, s2, newService);
            return;
        }
    }

    SubMenu *menu = new SubMenu;
    menu->name = s1;
    parentMenu->subMenus.append(menu);
    insertService(menu, s2, newService);
}

VFolderMenu::VFolderMenu(KServiceFactory *serviceFactory, KBuildSycocaInterface *kbuildsycocaInterface)
    : m_appsInfo(nullptr)
    , m_rootMenu(nullptr)
    , m_currentMenu(nullptr)
    , m_track(false)
    , m_serviceFactory(serviceFactory)
    , m_kbuildsycocaInterface(kbuildsycocaInterface)
{
    m_usedAppsDict.reserve(797);
    initDirs();
}

VFolderMenu::~VFolderMenu()
{
    qDeleteAll(m_appsInfoList);
    delete m_rootMenu;
}
// clang-format off
#define FOR_ALL_APPLICATIONS(it) \
    for (AppsInfo *info : std::as_const(m_appsInfoStack)) \
    { \
        QHashIterator<QString,KService::Ptr> it = info->applications; \
        while (it.hasNext()) \
        { \
            it.next();
#define FOR_ALL_APPLICATIONS_END } }

#define FOR_CATEGORY(category, it) \
    for (AppsInfo *info : std::as_const(m_appsInfoStack)) \
    { \
        const KService::List list = info->dictCategories.value(category); \
        for(KService::List::ConstIterator it = list.constBegin(); \
                it != list.constEnd(); ++it) \
        {
#define FOR_CATEGORY_END } }
// clang-format on

KService::Ptr VFolderMenu::findApplication(const QString &relPath)
{
    for (AppsInfo *info : std::as_const(m_appsInfoStack)) {
        if (info->applications.contains(relPath)) {
            KService::Ptr s = info->applications[relPath];
            if (s) {
                return s;
            }
        }
    }
    return KService::Ptr();
}

void VFolderMenu::addApplication(const QString &id, KService::Ptr service)
{
    service->setMenuId(id);
    m_appsInfo->applications.insert(id, service); // replaces, if already there
    m_serviceFactory->addEntry(KSycocaEntry::Ptr(service));
}

void VFolderMenu::buildApplicationIndex(bool unusedOnly)
{
    for (AppsInfo *info : std::as_const(m_appsInfoList)) {
        info->dictCategories.clear();
        QMutableHashIterator<QString, KService::Ptr> it = info->applications;
        while (it.hasNext()) {
            KService::Ptr s = it.next().value();
            if (unusedOnly && m_usedAppsDict.contains(s->menuId())) {
                // Remove and skip this one
                it.remove();
                continue;
            }

            const auto categories = s->categories();
            for (const QString &cat : categories) {
                info->dictCategories[cat].append(s); // find or insert entry in hash
            }
        }
    }
}

void VFolderMenu::createAppsInfo()
{
    if (m_appsInfo) {
        return;
    }

    m_appsInfo = new AppsInfo;
    m_appsInfoStack.prepend(m_appsInfo);
    m_appsInfoList.append(m_appsInfo);
    m_currentMenu->apps_info = m_appsInfo;
}

void VFolderMenu::loadAppsInfo()
{
    m_appsInfo = m_currentMenu->apps_info;
    if (!m_appsInfo) {
        return; // No appsInfo for this menu
    }

    if (!m_appsInfoStack.isEmpty() && m_appsInfoStack.first() == m_appsInfo) {
        return; // Already added (By createAppsInfo?)
    }

    m_appsInfoStack.prepend(m_appsInfo); // Add
}

void VFolderMenu::unloadAppsInfo()
{
    m_appsInfo = m_currentMenu->apps_info;
    if (!m_appsInfo) {
        return; // No appsInfo for this menu
    }

    if (m_appsInfoStack.first() != m_appsInfo) {
        return; // Already removed (huh?)
    }

    m_appsInfoStack.removeAll(m_appsInfo); // Remove
    m_appsInfo = nullptr;
}

QString VFolderMenu::absoluteDir(const QString &_dir, const QString &baseDir, bool keepRelativeToCfg)
{
    QString dir = _dir;
    if (QDir::isRelativePath(dir)) {
        dir = baseDir + dir;
    }

    bool relative = QDir::isRelativePath(dir);
    if (relative && !keepRelativeToCfg) {
        relative = false;
        dir = QStandardPaths::locate(QStandardPaths::GenericConfigLocation, QLatin1String("menus/") + dir, QStandardPaths::LocateDirectory);
    }

    if (!relative) {
        QString resolved = QDir(dir).canonicalPath();
        if (!resolved.isEmpty()) {
            dir = resolved;
        }
    }

    if (!dir.endsWith(QLatin1Char('/'))) {
        dir += QLatin1Char('/');
    }

    return dir;
}

static void tagBaseDir(QDomDocument &doc, const QString &tag, const QString &dir)
{
    QDomNodeList mergeFileList = doc.elementsByTagName(tag);
    for (int i = 0; i < mergeFileList.count(); i++) {
        QDomAttr attr = doc.createAttribute(QStringLiteral("__BaseDir"));
        attr.setValue(dir);
        mergeFileList.item(i).toElement().setAttributeNode(attr);
    }
}

static void tagBasePath(QDomDocument &doc, const QString &tag, const QString &path)
{
    QDomNodeList mergeFileList = doc.elementsByTagName(tag);
    for (int i = 0; i < mergeFileList.count(); i++) {
        QDomAttr attr = doc.createAttribute(QStringLiteral("__BasePath"));
        attr.setValue(path);
        mergeFileList.item(i).toElement().setAttributeNode(attr);
    }
}

QDomDocument VFolderMenu::loadDoc()
{
    QDomDocument doc;
    if (m_docInfo.path.isEmpty()) {
        return doc;
    }
    QFile file(m_docInfo.path);
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(SYCOCA) << "Could not open " << m_docInfo.path;
        return doc;
    }
    if (file.size() == 0) {
        return doc;
    }
    const auto result = doc.setContent(&file);
    if (!result) {
        qCWarning(SYCOCA) << "Parse error in " << m_docInfo.path << ", line " << result.errorLine << ", col " << result.errorColumn << ": "
                          << result.errorMessage;
        file.close();
        return doc;
    }
    file.close();

    tagBaseDir(doc, QStringLiteral("MergeFile"), m_docInfo.baseDir);
    tagBasePath(doc, QStringLiteral("MergeFile"), m_docInfo.path);
    tagBaseDir(doc, QStringLiteral("MergeDir"), m_docInfo.baseDir);
    tagBaseDir(doc, QStringLiteral("DirectoryDir"), m_docInfo.baseDir);
    tagBaseDir(doc, QStringLiteral("AppDir"), m_docInfo.baseDir);
    tagBaseDir(doc, QStringLiteral("LegacyDir"), m_docInfo.baseDir);

    return doc;
}

void VFolderMenu::mergeFile(QDomElement &parent, const QDomNode &mergeHere)
{
    // qCDebug(SYCOCA) << m_docInfo.path;
    QDomDocument doc = loadDoc();

    QDomElement docElem = doc.documentElement();
    QDomNode n = docElem.firstChild();
    QDomNode last = mergeHere;
    while (!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        QDomNode next = n.nextSibling();

        if (e.isNull()) {
            // Skip
        }
        // The spec says we must ignore any Name nodes
        else if (e.tagName() != QLatin1String("Name")) {
            parent.insertAfter(n, last);
            last = n;
        }

        docElem.removeChild(n);
        n = next;
    }
}

void VFolderMenu::mergeMenus(QDomElement &docElem, QString &name)
{
    QMap<QString, QDomElement> menuNodes;
    QMap<QString, QDomElement> directoryNodes;
    QMap<QString, QDomElement> appDirNodes;
    QMap<QString, QDomElement> directoryDirNodes;
    QMap<QString, QDomElement> legacyDirNodes;
    QDomElement defaultLayoutNode;
    QDomElement layoutNode;

    QDomNode n = docElem.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if (e.isNull()) {
            // qCDebug(SYCOCA) << "Empty node";
        } else if (e.tagName() == QLatin1String("DefaultAppDirs")) {
            // Replace with m_defaultAppDirs
            replaceNode(docElem, n, m_defaultAppDirs, QStringLiteral("AppDir"));
            continue;
        } else if (e.tagName() == QLatin1String("DefaultDirectoryDirs")) {
            // Replace with m_defaultDirectoryDirs
            replaceNode(docElem, n, m_defaultDirectoryDirs, QStringLiteral("DirectoryDir"));
            continue;
        } else if (e.tagName() == QLatin1String("DefaultMergeDirs")) {
            // Replace with m_defaultMergeDirs
            replaceNode(docElem, n, m_defaultMergeDirs, QStringLiteral("MergeDir"));
            continue;
        } else if (e.tagName() == QLatin1String("AppDir")) {
            // Filter out dupes
            foldNode(docElem, e, appDirNodes);
        } else if (e.tagName() == QLatin1String("DirectoryDir")) {
            // Filter out dupes
            foldNode(docElem, e, directoryDirNodes);
        } else if (e.tagName() == QLatin1String("LegacyDir")) {
            // Filter out dupes
            foldNode(docElem, e, legacyDirNodes);
        } else if (e.tagName() == QLatin1String("Directory")) {
            // Filter out dupes
            foldNode(docElem, e, directoryNodes);
        } else if (e.tagName() == QLatin1String("Move")) {
            // Filter out dupes
            QString orig;
            QDomNode n2 = e.firstChild();
            while (!n2.isNull()) {
                QDomElement e2 = n2.toElement(); // try to convert the node to an element.
                if (e2.tagName() == QLatin1String("Old")) {
                    orig = e2.text();
                    break;
                }
                n2 = n2.nextSibling();
            }
            foldNode(docElem, e, appDirNodes, orig);
        } else if (e.tagName() == QLatin1String("Menu")) {
            QString name;
            mergeMenus(e, name);
            QMap<QString, QDomElement>::iterator it = menuNodes.find(name);
            if (it != menuNodes.end()) {
                QDomElement docElem2 = *it;
                QDomNode n2 = docElem2.firstChild();
                QDomNode first = e.firstChild();
                while (!n2.isNull()) {
                    QDomElement e2 = n2.toElement(); // try to convert the node to an element.
                    QDomNode n3 = n2.nextSibling();
                    e.insertBefore(n2, first);
                    docElem2.removeChild(n2);
                    n2 = n3;
                }
                // We still have duplicated Name entries
                // but we don't care about that

                docElem.removeChild(docElem2);
                menuNodes.erase(it);
            }
            menuNodes.insert(name, e);
        } else if (e.tagName() == QLatin1String("MergeFile")) {
            if ((e.attribute(QStringLiteral("type")) == QLatin1String("parent"))) {
                // Ignore e.text(), as per the standard. We'll just look up the parent (more global) xml file.
                pushDocInfoParent(e.attribute(QStringLiteral("__BasePath")), e.attribute(QStringLiteral("__BaseDir")));
            } else {
                pushDocInfo(e.text(), e.attribute(QStringLiteral("__BaseDir")));
            }

            if (!m_docInfo.path.isEmpty()) {
                mergeFile(docElem, n);
            }
            popDocInfo();

            QDomNode last = n;
            n = n.nextSibling();
            docElem.removeChild(last); // Remove the MergeFile node
            continue;
        } else if (e.tagName() == QLatin1String("MergeDir")) {
            const QString dir = absoluteDir(e.text(), e.attribute(QStringLiteral("__BaseDir")), true);
            Q_ASSERT(dir.endsWith(QLatin1Char('/')));

            const bool relative = QDir::isRelativePath(dir);
            const QStringList dirs =
                QStandardPaths::locateAll(QStandardPaths::GenericConfigLocation, QLatin1String("menus/") + dir, QStandardPaths::LocateDirectory);
            for (const QString &menuDir : dirs) {
                registerDirectory(menuDir);
            }

            QStringList fileList;
            for (const QString &menuDir : dirs) {
                const QStringList fileNames = QDir(menuDir).entryList(QStringList() << QStringLiteral("*.menu"));
                for (const QString &file : fileNames) {
                    const QString fileToAdd = relative ? dir + file : menuDir + file;
                    if (!fileList.contains(fileToAdd)) {
                        fileList.append(fileToAdd);
                    }
                }
            }

            for (const QString &file : std::as_const(fileList)) {
                pushDocInfo(file);
                mergeFile(docElem, n);
                popDocInfo();
            }

            QDomNode last = n;
            n = n.nextSibling();
            docElem.removeChild(last); // Remove the MergeDir node

            continue;
        } else if (e.tagName() == QLatin1String("Name")) {
            name = e.text();
        } else if (e.tagName() == QLatin1String("DefaultLayout")) {
            if (!defaultLayoutNode.isNull()) {
                docElem.removeChild(defaultLayoutNode);
            }
            defaultLayoutNode = e;
        } else if (e.tagName() == QLatin1String("Layout")) {
            if (!layoutNode.isNull()) {
                docElem.removeChild(layoutNode);
            }
            layoutNode = e;
        }
        n = n.nextSibling();
    }
}

static QString makeRelative(const QString &dir)
{
    const QString canonical = QDir(dir).canonicalPath();
    const auto list = QStandardPaths::locateAll(QStandardPaths::GenericConfigLocation, QStringLiteral("menus"), QStandardPaths::LocateDirectory);
    for (const QString &base : list) {
        if (canonical.startsWith(base)) {
            return canonical.mid(base.length() + 1);
        }
    }
    return dir;
}

void VFolderMenu::pushDocInfo(const QString &fileName, const QString &baseDir)
{
    m_docInfoStack.push(m_docInfo);
    if (!baseDir.isEmpty()) {
        if (!QDir::isRelativePath(baseDir)) {
            m_docInfo.baseDir = makeRelative(baseDir);
        } else {
            m_docInfo.baseDir = baseDir;
        }
    }

    QString baseName = fileName;
    if (!QDir::isRelativePath(baseName)) {
        registerFile(baseName);
    } else {
        baseName = m_docInfo.baseDir + baseName;
    }

    m_docInfo.path = locateMenuFile(fileName);
    if (m_docInfo.path.isEmpty()) {
        m_docInfo.baseDir.clear();
        m_docInfo.baseName.clear();
        qCDebug(SYCOCA) << "Menu" << fileName << "not found.";
        return;
    }
    qCDebug(SYCOCA) << "Found menu file" << m_docInfo.path;
    int i;
    i = baseName.lastIndexOf(QLatin1Char('/'));
    if (i > 0) {
        m_docInfo.baseDir = baseName.left(i + 1);
        m_docInfo.baseName = baseName.mid(i + 1, baseName.length() - i - 6);
    } else {
        m_docInfo.baseDir.clear();
        m_docInfo.baseName = baseName.left(baseName.length() - 5);
    }
}

void VFolderMenu::pushDocInfoParent(const QString &basePath, const QString &baseDir)
{
    m_docInfoStack.push(m_docInfo);

    m_docInfo.baseDir = baseDir;

    QString fileName = basePath.mid(basePath.lastIndexOf(QLatin1Char('/')) + 1);
    m_docInfo.baseName = fileName.left(fileName.length() - 5); // without ".menu"
    QString baseName = QDir::cleanPath(m_docInfo.baseDir + fileName);

    QStringList result = QStandardPaths::locateAll(QStandardPaths::GenericConfigLocation, QLatin1String("menus/") + baseName);

    // Remove anything "more local" than basePath.
    while (!result.isEmpty() && (result.at(0) != basePath)) {
        result.removeFirst();
    }

    if (result.count() <= 1) {
        m_docInfo.path.clear(); // No parent found
        return;
    }
    // Now result.at(0) == basePath, take the next one, i.e. the one in the parent dir.
    m_docInfo.path = result.at(1);
}

void VFolderMenu::popDocInfo()
{
    m_docInfo = m_docInfoStack.pop();
}

QString VFolderMenu::locateMenuFile(const QString &fileName)
{
    if (!QDir::isRelativePath(fileName)) {
        if (QFile::exists(fileName)) {
            return fileName;
        }
        return QString();
    }

    QString result;

    QString xdgMenuPrefix = QString::fromLocal8Bit(qgetenv("XDG_MENU_PREFIX"));
    if (!xdgMenuPrefix.isEmpty()) {
        QFileInfo fileInfo(fileName);

        QString fileNameOnly = fileInfo.fileName();
        if (!fileNameOnly.startsWith(xdgMenuPrefix)) {
            fileNameOnly = xdgMenuPrefix + fileNameOnly;
        }

        QString baseName = QDir::cleanPath(m_docInfo.baseDir + fileInfo.path() + QLatin1Char('/') + fileNameOnly);
        result = QStandardPaths::locate(QStandardPaths::GenericConfigLocation, QLatin1String("menus/") + baseName);
    }

    if (result.isEmpty()) {
        QString baseName = QDir::cleanPath(m_docInfo.baseDir + fileName);
        result = QStandardPaths::locate(QStandardPaths::GenericConfigLocation, QLatin1String("menus/") + baseName);
    }

    return result;
}

QString VFolderMenu::locateDirectoryFile(const QString &fileName)
{
    if (fileName.isEmpty()) {
        return QString();
    }

    if (!QDir::isRelativePath(fileName)) {
        if (QFile::exists(fileName)) {
            return fileName;
        }
        return QString();
    }

    // First location in the list wins
    for (QStringList::ConstIterator it = m_directoryDirs.constBegin(); it != m_directoryDirs.constEnd(); ++it) {
        QString tmp = (*it) + fileName;
        if (QFile::exists(tmp)) {
            return tmp;
        }
    }

    return QString();
}

void VFolderMenu::initDirs()
{
    m_defaultAppDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    m_defaultDirectoryDirs =
        QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("desktop-directories"), QStandardPaths::LocateDirectory);
}

void VFolderMenu::loadMenu(const QString &fileName)
{
    m_defaultMergeDirs.clear();

    if (!fileName.endsWith(QLatin1String(".menu"))) {
        return;
    }

    pushDocInfo(fileName);
    m_defaultMergeDirs << QStringLiteral("applications-merged/");
    m_doc = loadDoc();
    popDocInfo();

    if (m_doc.isNull()) {
        if (m_docInfo.path.isEmpty()) {
            qCritical() << fileName << " not found in " << m_allDirectories;
        } else {
            qCWarning(SYCOCA) << "Load error (" << m_docInfo.path << ")";
        }
        return;
    }

    QDomElement e = m_doc.documentElement();
    QString name;
    mergeMenus(e, name);
}

void VFolderMenu::processCondition(QDomElement &domElem, QHash<QString, KService::Ptr> &items)
{
    if (domElem.tagName() == QLatin1String("And")) {
        QDomNode n = domElem.firstChild();
        // Look for the first child element
        while (!n.isNull()) { // loop in case of comments
            QDomElement e = n.toElement();
            n = n.nextSibling();
            if (!e.isNull()) {
                processCondition(e, items);
                break; // we only want the first one
            }
        }

        QHash<QString, KService::Ptr> andItems;
        while (!n.isNull()) {
            QDomElement e = n.toElement();
            if (e.tagName() == QLatin1String("Not")) {
                // Special handling for "and not"
                QDomNode n2 = e.firstChild();
                while (!n2.isNull()) {
                    QDomElement e2 = n2.toElement();
                    andItems.clear();
                    processCondition(e2, andItems);
                    excludeItems(items, andItems);
                    n2 = n2.nextSibling();
                }
            } else {
                andItems.clear();
                processCondition(e, andItems);
                matchItems(items, andItems);
            }
            n = n.nextSibling();
        }
    } else if (domElem.tagName() == QLatin1String("Or")) {
        QDomNode n = domElem.firstChild();
        // Look for the first child element
        while (!n.isNull()) { // loop in case of comments
            QDomElement e = n.toElement();
            n = n.nextSibling();
            if (!e.isNull()) {
                processCondition(e, items);
                break; // we only want the first one
            }
        }

        QHash<QString, KService::Ptr> orItems;
        while (!n.isNull()) {
            QDomElement e = n.toElement();
            if (!e.isNull()) {
                orItems.clear();
                processCondition(e, orItems);
                includeItems(items, orItems);
            }
            n = n.nextSibling();
        }
    } else if (domElem.tagName() == QLatin1String("Not")) {
        FOR_ALL_APPLICATIONS(it)
        {
            KService::Ptr s = it.value();
            items.insert(s->menuId(), s);
        }
        FOR_ALL_APPLICATIONS_END

        QHash<QString, KService::Ptr> notItems;
        QDomNode n = domElem.firstChild();
        while (!n.isNull()) {
            QDomElement e = n.toElement();
            if (!e.isNull()) {
                notItems.clear();
                processCondition(e, notItems);
                excludeItems(items, notItems);
            }
            n = n.nextSibling();
        }
    } else if (domElem.tagName() == QLatin1String("Category")) {
        FOR_CATEGORY(domElem.text(), it)
        {
            KService::Ptr s = *it;
            items.insert(s->menuId(), s);
        }
        FOR_CATEGORY_END
    } else if (domElem.tagName() == QLatin1String("All")) {
        FOR_ALL_APPLICATIONS(it)
        {
            KService::Ptr s = it.value();
            items.insert(s->menuId(), s);
        }
        FOR_ALL_APPLICATIONS_END
    } else if (domElem.tagName() == QLatin1String("Filename")) {
        const QString filename = domElem.text();
        // qCDebug(SYCOCA) << "Adding file" << filename;
        KService::Ptr s = findApplication(filename);
        if (s) {
            items.insert(filename, s);
        }
    }
}

void VFolderMenu::loadApplications(const QString &dir, const QString &prefix)
{
    qCDebug(SYCOCA) << "Looking up applications under" << dir;

    QDirIterator it(dir);
    while (it.hasNext()) {
        it.next();
        const QFileInfo fi = it.fileInfo();
        const QString fn = fi.fileName();
        if (fi.isDir() && !fi.isSymLink() && !fi.isBundle()) { // same check as in ksycocautils_p.h
            if (fn == QLatin1String(".") || fn == QLatin1String("..")) {
                continue;
            }
            loadApplications(fi.filePath(), prefix + fn + QLatin1Char('-'));
            continue;
        }
        if (fi.isFile()) {
            if (!fn.endsWith(QLatin1String(".desktop"))) {
                continue;
            }
            KService::Ptr service = m_kbuildsycocaInterface->createService(fi.absoluteFilePath());
#ifndef NDEBUG
            if (fn.contains(QLatin1String("fake"))) {
                qCDebug(SYCOCA) << "createService" << fi.absoluteFilePath() << "returned" << (service ? "valid service" : "NULL SERVICE");
            }
#endif
            if (service) {
                addApplication(prefix + fn, service);
            }
        }
    }
}

void VFolderMenu::processLegacyDir(const QString &dir, const QString &relDir, const QString &prefix)
{
    // qCDebug(SYCOCA).nospace() << "processLegacyDir(" << dir << ", " << relDir << ", " << prefix << ")";

    QHash<QString, KService::Ptr> items;
    QDirIterator it(dir);
    while (it.hasNext()) {
        it.next();
        const QFileInfo fi = it.fileInfo();
        const QString fn = fi.fileName();
        if (fi.isDir()) {
            if (fn == QLatin1String(".") || fn == QLatin1String("..")) {
                continue;
            }
            SubMenu *parentMenu = m_currentMenu;

            m_currentMenu = new SubMenu;
            m_currentMenu->name = fn;
            m_currentMenu->directoryFile = fi.absoluteFilePath() + QLatin1String("/.directory");

            parentMenu->subMenus.append(m_currentMenu);

            processLegacyDir(fi.filePath(), relDir + fn + QLatin1Char('/'), prefix);
            m_currentMenu = parentMenu;
            continue;
        }
        if (fi.isFile() /*&& !fi.isSymLink() ?? */) {
            if (!fn.endsWith(QLatin1String(".desktop"))) {
                continue;
            }
            KService::Ptr service = m_kbuildsycocaInterface->createService(fi.absoluteFilePath());
            if (service) {
                const QString id = prefix + fn;

                // TODO: Add legacy category
                addApplication(id, service);
                items.insert(service->menuId(), service);

                if (service->categories().isEmpty()) {
                    m_currentMenu->items.insert(id, service);
                }
            }
        }
    }
    markUsedApplications(items);
}

void VFolderMenu::processMenu(QDomElement &docElem, int pass)
{
    SubMenu *parentMenu = m_currentMenu;
    int oldDirectoryDirsCount = m_directoryDirs.count();

    QString name;
    QString directoryFile;
    bool onlyUnallocated = false;
    bool isDeleted = false;
    QDomElement defaultLayoutNode;
    QDomElement layoutNode;

    QDomElement query;
    QDomNode n = docElem.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if (e.tagName() == QLatin1String("Name")) {
            name = e.text();
        } else if (e.tagName() == QLatin1String("Directory")) {
            directoryFile = e.text();
        } else if (e.tagName() == QLatin1String("DirectoryDir")) {
            QString dir = absoluteDir(e.text(), e.attribute(QStringLiteral("__BaseDir")));

            m_directoryDirs.prepend(dir);
        } else if (e.tagName() == QLatin1String("OnlyUnallocated")) {
            onlyUnallocated = true;
        } else if (e.tagName() == QLatin1String("NotOnlyUnallocated")) {
            onlyUnallocated = false;
        } else if (e.tagName() == QLatin1String("Deleted")) {
            isDeleted = true;
        } else if (e.tagName() == QLatin1String("NotDeleted")) {
            isDeleted = false;
        } else if (e.tagName() == QLatin1String("DefaultLayout")) {
            defaultLayoutNode = e;
        } else if (e.tagName() == QLatin1String("Layout")) {
            layoutNode = e;
        }
        n = n.nextSibling();
    }

    // Setup current menu entry
    if (pass == 0) {
        m_currentMenu = nullptr;
        // Look up menu
        if (parentMenu) {
            for (SubMenu *menu : std::as_const(parentMenu->subMenus)) {
                if (menu->name == name) {
                    m_currentMenu = menu;
                    break;
                }
            }
        }

        if (!m_currentMenu) { // Not found?
            // Create menu
            m_currentMenu = new SubMenu;
            m_currentMenu->name = name;

            if (parentMenu) {
                parentMenu->subMenus.append(m_currentMenu);
            } else {
                m_rootMenu = m_currentMenu;
            }
        }
        if (directoryFile.isEmpty()) {
            // qCDebug(SYCOCA) << "Menu" << name << "does not specify a directory file.";
        }

        // Override previous directoryFile iff available
        QString tmp = locateDirectoryFile(directoryFile);
        if (!tmp.isEmpty()) {
            m_currentMenu->directoryFile = tmp;
        }
        m_currentMenu->isDeleted = isDeleted;

        m_currentMenu->defaultLayoutNode = defaultLayoutNode;
        m_currentMenu->layoutNode = layoutNode;
    } else {
        // Look up menu
        if (parentMenu) {
            for (SubMenu *menu : std::as_const(parentMenu->subMenus)) {
                if (menu->name == name) {
                    m_currentMenu = menu;
                    break;
                }
            }
        } else {
            m_currentMenu = m_rootMenu;
        }
    }

    // Process AppDir and LegacyDir
    if (pass == 0) {
        QDomElement query;
        QDomNode n = docElem.firstChild();
        while (!n.isNull()) {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if (e.tagName() == QLatin1String("AppDir")) {
                createAppsInfo();
                QString dir = absoluteDir(e.text(), e.attribute(QStringLiteral("__BaseDir")));

                registerDirectory(dir);

                loadApplications(dir, QString());
            } else if (e.tagName() == QLatin1String("LegacyDir")) {
                createAppsInfo();
                QString dir = absoluteDir(e.text(), e.attribute(QStringLiteral("__BaseDir")));

                QString prefix = e.attributes().namedItem(QStringLiteral("prefix")).toAttr().value();

                SubMenu *oldMenu = m_currentMenu;
                m_currentMenu = new SubMenu;

                registerDirectory(dir);

                processLegacyDir(dir, QString(), prefix);

                m_legacyNodes.insert(dir, m_currentMenu);
                m_currentMenu = oldMenu;
            }
            n = n.nextSibling();
        }
    }

    loadAppsInfo(); // Update the scope wrt the list of applications

    if (((pass == 1) && !onlyUnallocated) || ((pass == 2) && onlyUnallocated)) {
        n = docElem.firstChild();

        while (!n.isNull()) {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if (e.tagName() == QLatin1String("Include")) {
                QHash<QString, KService::Ptr> items;

                QDomNode n2 = e.firstChild();
                while (!n2.isNull()) {
                    QDomElement e2 = n2.toElement();
                    items.clear();
                    processCondition(e2, items);
                    if (m_track) {
                        track(m_trackId, m_currentMenu->name, m_currentMenu->items, m_currentMenu->excludeItems, items, QStringLiteral("Before <Include>"));
                    }
                    includeItems(m_currentMenu->items, items);
                    excludeItems(m_currentMenu->excludeItems, items);
                    markUsedApplications(items);

                    if (m_track) {
                        track(m_trackId, m_currentMenu->name, m_currentMenu->items, m_currentMenu->excludeItems, items, QStringLiteral("After <Include>"));
                    }

                    n2 = n2.nextSibling();
                }
            }

            else if (e.tagName() == QLatin1String("Exclude")) {
                QHash<QString, KService::Ptr> items;

                QDomNode n2 = e.firstChild();
                while (!n2.isNull()) {
                    QDomElement e2 = n2.toElement();
                    items.clear();
                    processCondition(e2, items);
                    if (m_track) {
                        track(m_trackId, m_currentMenu->name, m_currentMenu->items, m_currentMenu->excludeItems, items, QStringLiteral("Before <Exclude>"));
                    }
                    excludeItems(m_currentMenu->items, items);
                    includeItems(m_currentMenu->excludeItems, items);
                    if (m_track) {
                        track(m_trackId, m_currentMenu->name, m_currentMenu->items, m_currentMenu->excludeItems, items, QStringLiteral("After <Exclude>"));
                    }
                    n2 = n2.nextSibling();
                }
            }

            n = n.nextSibling();
        }
    }

    n = docElem.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if (e.tagName() == QLatin1String("Menu")) {
            processMenu(e, pass);
        }
        // We insert legacy dir in pass 0, this way the order in the .menu-file determines
        // which .directory file gets used, but the menu-entries of legacy-menus will always
        // have the lowest priority.
        //      else if (((pass == 1) && !onlyUnallocated) || ((pass == 2) && onlyUnallocated))
        else if (pass == 0) {
            if (e.tagName() == QLatin1String("LegacyDir")) {
                // Add legacy nodes to Menu structure
                QString dir = absoluteDir(e.text(), e.attribute(QStringLiteral("__BaseDir")));
                SubMenu *legacyMenu = m_legacyNodes[dir];
                if (legacyMenu) {
                    mergeMenu(m_currentMenu, legacyMenu);
                }
            }
        }
        n = n.nextSibling();
    }

    if (pass == 2) {
        n = docElem.firstChild();
        while (!n.isNull()) {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if (e.tagName() == QLatin1String("Move")) {
                QString orig;
                QString dest;
                QDomNode n2 = e.firstChild();
                while (!n2.isNull()) {
                    QDomElement e2 = n2.toElement(); // try to convert the node to an element.
                    if (e2.tagName() == QLatin1String("Old")) {
                        orig = e2.text();
                    }
                    if (e2.tagName() == QLatin1String("New")) {
                        dest = e2.text();
                    }
                    n2 = n2.nextSibling();
                }
                // qCDebug(SYCOCA) << "Moving" << orig << "to" << dest;
                if (!orig.isEmpty() && !dest.isEmpty()) {
                    SubMenu *menu = takeSubMenu(m_currentMenu, orig);
                    if (menu) {
                        insertSubMenu(m_currentMenu, dest, menu, true); // Destination has priority
                    }
                }
            }
            n = n.nextSibling();
        }
    }

    unloadAppsInfo(); // Update the scope wrt the list of applications

    while (m_directoryDirs.count() > oldDirectoryDirsCount) {
        m_directoryDirs.pop_front();
    }

    m_currentMenu = parentMenu;
}

static QString parseAttribute(const QDomElement &e)
{
    QString option;

    const QString SHOW_EMPTY = QStringLiteral("show_empty");
    if (e.hasAttribute(SHOW_EMPTY)) {
        QString str = e.attribute(SHOW_EMPTY);
        if (str == QLatin1String("true")) {
            option = QStringLiteral("ME ");
        } else if (str == QLatin1String("false")) {
            option = QStringLiteral("NME ");
        } else {
            // qCDebug(SYCOCA)<<" Error in parsing show_empty attribute :"<<str;
        }
    }
    const QString INLINE = QStringLiteral("inline");
    if (e.hasAttribute(INLINE)) {
        QString str = e.attribute(INLINE);
        if (str == QLatin1String("true")) {
            option += QLatin1String("I ");
        } else if (str == QLatin1String("false")) {
            option += QLatin1String("NI ");
        } else {
            qCDebug(SYCOCA) << " Error in parsing inline attribute :" << str;
        }
    }
    if (e.hasAttribute(QStringLiteral("inline_limit"))) {
        bool ok;
        int value = e.attribute(QStringLiteral("inline_limit")).toInt(&ok);
        if (ok) {
            option += QStringLiteral("IL[%1] ").arg(value);
        }
    }
    if (e.hasAttribute(QStringLiteral("inline_header"))) {
        QString str = e.attribute(QStringLiteral("inline_header"));
        if (str == QLatin1String("true")) {
            option += QLatin1String("IH ");
        } else if (str == QLatin1String("false")) {
            option += QLatin1String("NIH ");
        } else {
            qCDebug(SYCOCA) << " Error in parsing of inline_header attribute :" << str;
        }
    }
    if (e.hasAttribute(QStringLiteral("inline_alias")) && e.attribute(QStringLiteral("inline_alias")) == QLatin1String("true")) {
        QString str = e.attribute(QStringLiteral("inline_alias"));
        if (str == QLatin1String("true")) {
            option += QLatin1String("IA");
        } else if (str == QLatin1String("false")) {
            option += QLatin1String("NIA");
        } else {
            qCDebug(SYCOCA) << " Error in parsing inline_alias attribute :" << str;
        }
    }
    if (!option.isEmpty()) {
        option.prepend(QStringLiteral(":O"));
    }
    return option;
}

QStringList VFolderMenu::parseLayoutNode(const QDomElement &docElem) const
{
    QStringList layout;

    QString optionDefaultLayout;
    if (docElem.tagName() == QLatin1String("DefaultLayout")) {
        optionDefaultLayout = parseAttribute(docElem);
    }
    if (!optionDefaultLayout.isEmpty()) {
        layout.append(optionDefaultLayout);
    }

    bool mergeTagExists = false;
    QDomNode n = docElem.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if (e.tagName() == QLatin1String("Separator")) {
            layout.append(QStringLiteral(":S"));
        } else if (e.tagName() == QLatin1String("Filename")) {
            layout.append(e.text());
        } else if (e.tagName() == QLatin1String("Menuname")) {
            layout.append(QLatin1Char('/') + e.text());
            QString option = parseAttribute(e);
            if (!option.isEmpty()) {
                layout.append(option);
            }
        } else if (e.tagName() == QLatin1String("Merge")) {
            QString type = e.attributeNode(QStringLiteral("type")).value();
            if (type == QLatin1String("files")) {
                layout.append(QStringLiteral(":F"));
            } else if (type == QLatin1String("menus")) {
                layout.append(QStringLiteral(":M"));
            } else if (type == QLatin1String("all")) {
                layout.append(QStringLiteral(":A"));
            }
            mergeTagExists = true;
        }

        n = n.nextSibling();
    }

    if (!mergeTagExists) {
        layout.append(QStringLiteral(":M"));
        layout.append(QStringLiteral(":F"));
        qCWarning(SYCOCA) << "The menu spec file (" << m_docInfo.path
                          << ") contains a Layout or DefaultLayout tag without the mandatory Merge tag inside. Please fix it.";
    }
    return layout;
}

void VFolderMenu::layoutMenu(VFolderMenu::SubMenu *menu, QStringList defaultLayout) // krazy:exclude=passbyvalue
{
    if (!menu->defaultLayoutNode.isNull()) {
        defaultLayout = parseLayoutNode(menu->defaultLayoutNode);
    }

    if (menu->layoutNode.isNull()) {
        menu->layoutList = defaultLayout;
    } else {
        menu->layoutList = parseLayoutNode(menu->layoutNode);
        if (menu->layoutList.isEmpty()) {
            menu->layoutList = defaultLayout;
        }
    }

    for (VFolderMenu::SubMenu *subMenu : std::as_const(menu->subMenus)) {
        layoutMenu(subMenu, defaultLayout);
    }
}

void VFolderMenu::markUsedApplications(const QHash<QString, KService::Ptr> &items)
{
    for (const KService::Ptr &p : items) {
        m_usedAppsDict.insert(p->menuId());
    }
}

VFolderMenu::SubMenu *VFolderMenu::parseMenu(const QString &file)
{
    m_appsInfo = nullptr;

    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericConfigLocation, QStringLiteral("menus"), QStandardPaths::LocateDirectory);
    for (QStringList::ConstIterator it = dirs.begin(); it != dirs.end(); ++it) {
        registerDirectory(*it);
    }

    loadMenu(file);

    delete m_rootMenu;
    m_rootMenu = m_currentMenu = nullptr;

    QDomElement docElem = m_doc.documentElement();

    for (int pass = 0; pass <= 2; pass++) {
        // pass 0: load application desktop files
        // pass 1: the normal processing
        // pass 2: process <OnlyUnallocated> to put unused files into "Lost & Found".
        processMenu(docElem, pass);

        switch (pass) {
        case 0:
            // Fill the dictCategories for each AppsInfo in m_appsInfoList,
            // in preparation for processMenu pass 1.
            buildApplicationIndex(false);
            break;
        case 1:
            // Fill the dictCategories for each AppsInfo in m_appsInfoList,
            // with only the unused apps, in preparation for processMenu pass 2.
            buildApplicationIndex(true /* unusedOnly */);
            break;
        case 2: {
            QStringList defaultLayout;
            defaultLayout << QStringLiteral(":M"); // Sub-Menus
            defaultLayout << QStringLiteral(":F"); // Individual entries
            layoutMenu(m_rootMenu, defaultLayout);
            break;
        }
        default:
            break;
        }
    }

    return m_rootMenu;
}

void VFolderMenu::setTrackId(const QString &id)
{
    m_track = !id.isEmpty();
    m_trackId = id;
}

#include "moc_vfolder_menu_p.cpp"
