/*
 *   SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only
 *
 */

#include "menufile.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTextStream>

#include "kmenuedit_debug.h"
#include <KLocalizedString>

const QString MenuFile::MF_MENU = QStringLiteral("Menu");
const QString MenuFile::MF_PUBLIC_ID = QStringLiteral("-//freedesktop//DTD Menu 1.0//EN");
const QString MenuFile::MF_SYSTEM_ID = QStringLiteral("http://www.freedesktop.org/standards/menu-spec/1.0/menu.dtd");
const QString MenuFile::MF_NAME = QStringLiteral("Name");
const QString MenuFile::MF_INCLUDE = QStringLiteral("Include");
const QString MenuFile::MF_EXCLUDE = QStringLiteral("Exclude");
const QString MenuFile::MF_FILENAME = QStringLiteral("Filename");
const QString MenuFile::MF_DELETED = QStringLiteral("Deleted");
const QString MenuFile::MF_NOTDELETED = QStringLiteral("NotDeleted");
const QString MenuFile::MF_MOVE = QStringLiteral("Move");
const QString MenuFile::MF_OLD = QStringLiteral("Old");
const QString MenuFile::MF_NEW = QStringLiteral("New");
const QString MenuFile::MF_DIRECTORY = QStringLiteral("Directory");
const QString MenuFile::MF_LAYOUT = QStringLiteral("Layout");
const QString MenuFile::MF_MENUNAME = QStringLiteral("Menuname");
const QString MenuFile::MF_SEPARATOR = QStringLiteral("Separator");
const QString MenuFile::MF_MERGE = QStringLiteral("Merge");

MenuFile::MenuFile(const QString &file)
    : m_fileName(file)
    , m_bDirty(false)
{
    load();
}

MenuFile::~MenuFile()
{
}

bool MenuFile::load()
{
    if (m_fileName.isEmpty()) {
        return false;
    }

    QFile file(m_fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        if (file.exists()) {
            qCWarning(KMENUEDIT_LOG) << "Could not read " << m_fileName;
        }
        create();
        return false;
    }

    QString errorMsg;
    int errorRow;
    int errorCol;
    if (!m_doc.setContent(&file, &errorMsg, &errorRow, &errorCol)) {
        qCWarning(KMENUEDIT_LOG) << "Parse error in " << m_fileName << ", line " << errorRow << ", col " << errorCol << ": " << errorMsg;
        file.close();
        create();
        return false;
    }
    file.close();

    return true;
}

void MenuFile::create()
{
    QDomImplementation impl;
    QDomDocumentType docType = impl.createDocumentType(MF_MENU, MF_PUBLIC_ID, MF_SYSTEM_ID);
    m_doc = impl.createDocument(QString(), MF_MENU, docType);
}

bool MenuFile::save()
{
    QFile file(m_fileName);
    // create directory if it doesn't exist
    QFileInfo info(file);
    info.dir().mkpath(QStringLiteral("."));

    if (!file.open(QIODevice::WriteOnly)) {
        qCWarning(KMENUEDIT_LOG) << "Could not write " << m_fileName;
        m_error = i18n("Could not write to %1", m_fileName);
        return false;
    }
    QTextStream stream(&file);

    stream << m_doc.toString();

    file.close();

    if (file.error() != QFile::NoError) {
        qCWarning(KMENUEDIT_LOG) << "Could not close " << m_fileName;
        m_error = i18n("Could not write to %1", m_fileName);
        return false;
    }

    m_bDirty = false;

    return true;
}

QDomElement MenuFile::findMenu(QDomElement elem, const QString &menuName, bool create)
{
    QString menuNodeName;
    QString subMenuName;
    int i = menuName.indexOf(QLatin1Char('/'));
    if (i >= 0) {
        menuNodeName = menuName.left(i);
        subMenuName = menuName.mid(i + 1);
    } else {
        menuNodeName = menuName;
    }
    if (i == 0) {
        return findMenu(elem, subMenuName, create);
    }

    if (menuNodeName.isEmpty()) {
        return elem;
    }

    QDomNode n = elem.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if (e.tagName() == MF_MENU) {
            QString name;

            QDomNode n2 = e.firstChild();
            while (!n2.isNull()) {
                QDomElement e2 = n2.toElement();
                if (!e2.isNull() && e2.tagName() == MF_NAME) {
                    name = e2.text();
                    break;
                }
                n2 = n2.nextSibling();
            }

            if (name == menuNodeName) {
                if (subMenuName.isEmpty()) {
                    return e;
                } else {
                    return findMenu(e, subMenuName, create);
                }
            }
        }
        n = n.nextSibling();
    }

    if (!create) {
        return QDomElement();
    }

    // Create new node.
    QDomElement newElem = m_doc.createElement(MF_MENU);
    QDomElement newNameElem = m_doc.createElement(MF_NAME);
    newNameElem.appendChild(m_doc.createTextNode(menuNodeName));
    newElem.appendChild(newNameElem);
    elem.appendChild(newElem);

    if (subMenuName.isEmpty()) {
        return newElem;
    } else {
        return findMenu(newElem, subMenuName, create);
    }
}

static QString relativeToDesktopDirsLocation(const QString &file)
{
    const QString canonical = QFileInfo(file).canonicalFilePath();
    const QStringList dirs = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (const QString &dir : dirs) {
        const QString base = dir + QStringLiteral("/desktop-directories");
        if (canonical.startsWith(base)) {
            return canonical.mid(base.length() + 1);
        }
    }
    return QString();
}

static QString entryToDirId(const QString &path)
{
    // See also KDesktopFile::locateLocal
    QString local;
    if (QFileInfo(path).isAbsolute()) {
        // XDG Desktop menu items come with absolute paths, we need to
        // extract their relative path and then build a local path.
        local = relativeToDesktopDirsLocation(path);
    }

    if (local.isEmpty() || local.startsWith(QLatin1Char('/'))) {
        // What now? Use filename only and hope for the best.
        local = path.mid(path.lastIndexOf(QLatin1Char('/')) + 1);
    }
    return local;
}

static void purgeIncludesExcludes(const QDomElement &elem, const QString &appId, QDomElement &excludeNode, QDomElement &includeNode)
{
    // Remove any previous includes/excludes of appId
    QDomNode n = elem.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        bool bIncludeNode = (e.tagName() == MenuFile::MF_INCLUDE);
        bool bExcludeNode = (e.tagName() == MenuFile::MF_EXCLUDE);
        if (bIncludeNode) {
            includeNode = e;
        }
        if (bExcludeNode) {
            excludeNode = e;
        }
        if (bIncludeNode || bExcludeNode) {
            QDomNode n2 = e.firstChild();
            while (!n2.isNull()) {
                QDomNode next = n2.nextSibling();
                QDomElement e2 = n2.toElement();
                if (!e2.isNull() && e2.tagName() == MenuFile::MF_FILENAME) {
                    if (e2.text() == appId) {
                        e.removeChild(e2);
                        break;
                    }
                }
                n2 = next;
            }
        }
        n = n.nextSibling();
    }
}

static void purgeDeleted(QDomElement elem)
{
    // Remove any previous includes/excludes of appId
    QDomNode n = elem.firstChild();
    while (!n.isNull()) {
        QDomNode next = n.nextSibling();
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if ((e.tagName() == MenuFile::MF_DELETED) || (e.tagName() == MenuFile::MF_NOTDELETED)) {
            elem.removeChild(e);
        }
        n = next;
    }
}

static void purgeLayout(QDomElement elem)
{
    // Remove any previous includes/excludes of appId
    QDomNode n = elem.firstChild();
    while (!n.isNull()) {
        QDomNode next = n.nextSibling();
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if (e.tagName() == MenuFile::MF_LAYOUT) {
            elem.removeChild(e);
        }
        n = next;
    }
}

void MenuFile::addEntry(const QString &menuName, const QString &menuId)
{
    m_bDirty = true;

    m_removedEntries.removeAll(menuId);

    QDomElement elem = findMenu(m_doc.documentElement(), menuName, true);

    QDomElement excludeNode;
    QDomElement includeNode;

    purgeIncludesExcludes(elem, menuId, excludeNode, includeNode);

    if (includeNode.isNull()) {
        includeNode = m_doc.createElement(MF_INCLUDE);
        elem.appendChild(includeNode);
    }

    QDomElement fileNode = m_doc.createElement(MF_FILENAME);
    fileNode.appendChild(m_doc.createTextNode(menuId));
    includeNode.appendChild(fileNode);
}

void MenuFile::setLayout(const QString &menuName, const QStringList &layout)
{
    m_bDirty = true;

    QDomElement elem = findMenu(m_doc.documentElement(), menuName, true);

    purgeLayout(elem);

    QDomElement layoutNode = m_doc.createElement(MF_LAYOUT);
    elem.appendChild(layoutNode);

    for (QStringList::ConstIterator it = layout.constBegin(); it != layout.constEnd(); ++it) {
        QString li = *it;
        if (li == QLatin1String(":S")) {
            layoutNode.appendChild(m_doc.createElement(MF_SEPARATOR));
        } else if (li == QLatin1String(":M")) {
            QDomElement mergeNode = m_doc.createElement(MF_MERGE);
            mergeNode.setAttribute(QStringLiteral("type"), QStringLiteral("menus"));
            layoutNode.appendChild(mergeNode);
        } else if (li == QLatin1String(":F")) {
            QDomElement mergeNode = m_doc.createElement(MF_MERGE);
            mergeNode.setAttribute(QStringLiteral("type"), QStringLiteral("files"));
            layoutNode.appendChild(mergeNode);
        } else if (li == QLatin1String(":A")) {
            QDomElement mergeNode = m_doc.createElement(MF_MERGE);
            mergeNode.setAttribute(QStringLiteral("type"), QStringLiteral("all"));
            layoutNode.appendChild(mergeNode);
        } else if (li.endsWith(QLatin1Char('/'))) {
            li.chop(1);
            QDomElement menuNode = m_doc.createElement(MF_MENUNAME);
            menuNode.appendChild(m_doc.createTextNode(li));
            layoutNode.appendChild(menuNode);
        } else {
            QDomElement fileNode = m_doc.createElement(MF_FILENAME);
            fileNode.appendChild(m_doc.createTextNode(li));
            layoutNode.appendChild(fileNode);
        }
    }
}

void MenuFile::removeEntry(const QString &menuName, const QString &menuId)
{
    m_bDirty = true;
    m_removedEntries.append(menuId);

    QDomElement elem = findMenu(m_doc.documentElement(), menuName, true);

    QDomElement excludeNode;
    QDomElement includeNode;

    purgeIncludesExcludes(elem, menuId, excludeNode, includeNode);

    if (excludeNode.isNull()) {
        excludeNode = m_doc.createElement(MF_EXCLUDE);
        elem.appendChild(excludeNode);
    }
    QDomElement fileNode = m_doc.createElement(MF_FILENAME);
    fileNode.appendChild(m_doc.createTextNode(menuId));
    excludeNode.appendChild(fileNode);
}

void MenuFile::addMenu(const QString &menuName, const QString &menuFile)
{
    m_bDirty = true;
    QDomElement elem = findMenu(m_doc.documentElement(), menuName, true);

    QDomElement dirElem = m_doc.createElement(MF_DIRECTORY);
    dirElem.appendChild(m_doc.createTextNode(entryToDirId(menuFile)));
    elem.appendChild(dirElem);
}

void MenuFile::moveMenu(const QString &oldMenu, const QString &newMenu)
{
    m_bDirty = true;

    // Undelete the new menu
    QDomElement elem = findMenu(m_doc.documentElement(), newMenu, true);
    purgeDeleted(elem);
    elem.appendChild(m_doc.createElement(MF_NOTDELETED));

    // TODO: GET RID OF COMMON PART, IT BREAKS STUFF
    // Find common part
    QStringList oldMenuParts = oldMenu.split(QLatin1Char('/'));
    QStringList newMenuParts = newMenu.split(QLatin1Char('/'));
    QString commonMenuName;
    int max = qMin(oldMenuParts.count(), newMenuParts.count());
    int i = 0;
    for (; i < max; i++) {
        if (oldMenuParts[i] != newMenuParts[i]) {
            break;
        }
        commonMenuName += QLatin1Char('/') + oldMenuParts[i];
    }
    QString oldMenuName;
    for (int j = i; j < oldMenuParts.count() - 1; j++) {
        if (i != j) {
            oldMenuName += QLatin1Char('/');
        }
        oldMenuName += oldMenuParts[j];
    }
    QString newMenuName;
    for (int j = i; j < newMenuParts.count() - 1; j++) {
        if (i != j) {
            newMenuName += QLatin1Char('/');
        }
        newMenuName += newMenuParts[j];
    }

    if (oldMenuName == newMenuName) {
        return; // Can happen
    }
    elem = findMenu(m_doc.documentElement(), commonMenuName, true);

    // Add instructions for moving
    QDomElement moveNode = m_doc.createElement(MF_MOVE);
    QDomElement node = m_doc.createElement(MF_OLD);
    node.appendChild(m_doc.createTextNode(oldMenuName));
    moveNode.appendChild(node);
    node = m_doc.createElement(MF_NEW);
    node.appendChild(m_doc.createTextNode(newMenuName));
    moveNode.appendChild(node);
    elem.appendChild(moveNode);
}

void MenuFile::removeMenu(const QString &menuName)
{
    m_bDirty = true;

    QDomElement elem = findMenu(m_doc.documentElement(), menuName, true);

    purgeDeleted(elem);
    elem.appendChild(m_doc.createElement(MF_DELETED));
}

/**
 * Returns a unique menu-name for a new menu under @p menuName
 * inspired by @p newMenu
 */
QString MenuFile::uniqueMenuName(const QString &menuName, const QString &newMenu, const QStringList &excludeList)
{
    QDomElement elem = findMenu(m_doc.documentElement(), menuName, false);

    QString result = newMenu;
    if (result.endsWith(QLatin1Char('/'))) {
        result.chop(1);
    }

    static const QRegularExpression re(QStringLiteral("(.*)(?=-\\d+)"));
    const QRegularExpressionMatch match = re.match(result);
    result = match.hasMatch() ? match.captured(1) : result;

    int trunc = result.length(); // Position of trailing '/'

    result.append(QLatin1Char('/'));

    for (int n = 1; ++n;) {
        if (findMenu(elem, result, false).isNull() && !excludeList.contains(result)) {
            return result;
        }

        result.truncate(trunc);
        result.append(QStringLiteral("-%1/").arg(n));
    }
    return QString(); // Never reached
}

void MenuFile::performAction(const ActionAtom *atom)
{
    switch (atom->action) {
    case ADD_ENTRY:
        addEntry(atom->arg1, atom->arg2);
        return;
    case REMOVE_ENTRY:
        removeEntry(atom->arg1, atom->arg2);
        return;
    case ADD_MENU:
        addMenu(atom->arg1, atom->arg2);
        return;
    case REMOVE_MENU:
        removeMenu(atom->arg1);
        return;
    case MOVE_MENU:
        moveMenu(atom->arg1, atom->arg2);
        return;
    }
}

MenuFile::ActionAtom *MenuFile::pushAction(MenuFile::ActionType action, const QString &arg1, const QString &arg2)
{
    ActionAtom *atom = new ActionAtom;
    atom->action = action;
    atom->arg1 = arg1;
    atom->arg2 = arg2;
    m_actionList.append(atom);
    return atom;
}

void MenuFile::popAction(ActionAtom *atom)
{
    if (m_actionList.last() != atom) {
        qWarning("MenuFile::popAction Error, action not last in list.");
        return;
    }
    m_actionList.removeLast();
    delete atom;
}

bool MenuFile::performAllActions()
{
    Q_FOREACH (ActionAtom *atom, m_actionList) {
        performAction(atom);
        delete atom;
    }
    m_actionList.clear();

    // Entries that have been removed from the menu are added to .hidden
    // so that they don't re-appear in Lost & Found
    QStringList removed = m_removedEntries;
    m_removedEntries.clear();
    for (QStringList::ConstIterator it = removed.constBegin(); it != removed.constEnd(); ++it) {
        addEntry(QStringLiteral("/.hidden/"), *it);
    }

    m_removedEntries.clear();

    if (!m_bDirty) {
        return true;
    }

    return save();
}

bool MenuFile::dirty() const
{
    return (m_actionList.count() != 0) || m_bDirty;
}

void MenuFile::restoreMenuSystem(const QString &filename)
{
    m_error.clear();

    m_fileName = filename;
    m_doc.clear();
    m_bDirty = false;
    Q_FOREACH (ActionAtom *atom, m_actionList) {
        delete atom;
    }
    m_actionList.clear();

    m_removedEntries.clear();
    create();
}
