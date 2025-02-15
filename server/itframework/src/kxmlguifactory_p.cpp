/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2001 Simon Hausmann <hausmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kxmlguifactory_p.h"

#include "ktoolbar.h"
#include "kxmlguibuilder.h"
#include "kxmlguiclient.h"

#include <QVector>
#include <QWidget>

#include "debug.h"
#include <cassert>

using namespace KXMLGUI;

void ActionList::plug(QWidget *container, int index) const
{
    QAction *before = nullptr; // Insert after end of widget's current actions (default).

    if ((index < 0) || (index > container->actions().count())) {
        qCWarning(DEBUG_KXMLGUI) << "Index " << index << " is not within range (0 - " << container->actions().count() << ")";
    } else if (index != container->actions().count()) {
        before = container->actions().at(index); // Insert before indexed action.
    }

    for (QAction *action : *this) {
        container->insertAction(before, action);
        // before = action; // BUG FIX: do not insert actions in reverse order.
    }
}

ContainerNode::ContainerNode(QWidget *_container,
                             const QString &_tagName,
                             const QString &_name,
                             ContainerNode *_parent,
                             KXMLGUIClient *_client,
                             KXMLGUIBuilder *_builder,
                             QAction *_containerAction,
                             const QString &_mergingName,
                             const QString &_groupName,
                             const QStringList &customTags,
                             const QStringList &containerTags)
    : parent(_parent)
    , client(_client)
    , builder(_builder)
    , builderCustomTags(customTags)
    , builderContainerTags(containerTags)
    , container(_container)
    , containerAction(_containerAction)
    , tagName(_tagName)
    , name(_name)
    , groupName(_groupName)
    , index(0)
    , mergingName(_mergingName)
{
    if (parent) {
        parent->children.append(this);
    }
}

ContainerNode::~ContainerNode()
{
    qDeleteAll(children);
    qDeleteAll(clients);
}

void ContainerNode::removeChild(ContainerNode *child)
{
    children.removeAll(child);
    deleteChild(child);
}

void ContainerNode::deleteChild(ContainerNode *child)
{
    MergingIndexList::iterator mergingIt = findIndex(child->mergingName);
    adjustMergingIndices(-1, mergingIt, QString());
    delete child;
}

/*
 * Find a merging index with the given name. Used to find an index defined by <Merge name="blah"/>
 * or by a <DefineGroup name="foo" /> tag.
 */
MergingIndexList::iterator ContainerNode::findIndex(const QString &name)
{
    return std::find_if(mergingIndices.begin(), mergingIndices.end(), [&name](const MergingIndex &idx) {
        return idx.mergingName == name;
    });
}

/*
 * Find a container recursively with the given name. Either compares _name with the
 * container's tag name or the value of the container's name attribute. Specified by
 * the tag bool .
 */
ContainerNode *ContainerNode::findContainer(const QString &_name, bool tag)
{
    if ((tag && tagName == _name) || (!tag && name == _name)) {
        return this;
    }

    for (ContainerNode *child : std::as_const(children)) {
        ContainerNode *res = child->findContainer(_name, tag);
        if (res) {
            return res;
        }
    }

    return nullptr;
}

/*
 * Finds a child container node (not recursively) with the given name and tagname. Explicitly
 * leaves out container widgets specified in the exludeList . Also ensures that the containers
 * belongs to currClient.
 */
ContainerNode *ContainerNode::findContainer(const QString &name, const QString &tagName, const QList<QWidget *> *excludeList, KXMLGUIClient * /*currClient*/)
{
    if (!name.isEmpty()) {
        auto it = std::find_if(children.cbegin(), children.cend(), [&name, excludeList](ContainerNode *node) {
            return node->name == name && !excludeList->contains(node->container);
        });
        return it != children.cend() ? *it : nullptr;
    }

    if (!tagName.isEmpty()) {
        // It is a bad idea to also compare the client (node->client == currClient),
        // because we don't want to do so in situations like these:
        //
        // <MenuBar>
        //   <Menu>
        //     ...
        //
        // other client:
        // <MenuBar>
        //   <Menu>
        //    ...
        auto it = std::find_if(children.cbegin(), children.cend(), [&tagName, excludeList](ContainerNode *node) {
            return node->tagName == tagName && !excludeList->contains(node->container);
        });
        return it != children.cend() ? *it : nullptr;
    };

    return {};
}

ContainerClient *
ContainerNode::findChildContainerClient(KXMLGUIClient *currentGUIClient, const QString &groupName, const MergingIndexList::iterator &mergingIdx)
{
    auto it = std::find_if(clients.cbegin(), clients.cend(), [&groupName, currentGUIClient](ContainerClient *cl) {
        return cl->client == currentGUIClient && (groupName.isEmpty() || groupName == cl->groupName);
    });
    if (it != clients.cend()) {
        return *it;
    }

    ContainerClient *client = new ContainerClient;
    client->client = currentGUIClient;
    client->groupName = groupName;

    if (mergingIdx != mergingIndices.end()) {
        client->mergingName = (*mergingIdx).mergingName;
    }

    clients.append(client);

    return client;
}

void ContainerNode::plugActionList(BuildState &state)
{
    MergingIndexList::iterator mIt(mergingIndices.begin());
    MergingIndexList::iterator mEnd(mergingIndices.end());
    for (; mIt != mEnd; ++mIt) {
        plugActionList(state, mIt);
    }

    for (ContainerNode *child : std::as_const(children)) {
        child->plugActionList(state);
    }
}

void ContainerNode::plugActionList(BuildState &state, const MergingIndexList::iterator &mergingIdxIt)
{
    const QLatin1String tagActionList("actionlist");

    const MergingIndex &mergingIdx = *mergingIdxIt;
    if (mergingIdx.clientName != state.clientName) {
        return;
    }
    if (!mergingIdx.mergingName.startsWith(tagActionList)) {
        return;
    }
    const QString k = mergingIdx.mergingName.mid(tagActionList.size());
    if (k != state.actionListName) {
        return;
    }

    ContainerClient *client = findChildContainerClient(state.guiClient, QString(), mergingIndices.end());

    client->actionLists.insert(k, state.actionList);

    state.actionList.plug(container, mergingIdx.value);

    adjustMergingIndices(state.actionList.count(), mergingIdxIt, QString());
}

void ContainerNode::unplugActionList(BuildState &state)
{
    MergingIndexList::iterator mIt(mergingIndices.begin());
    MergingIndexList::iterator mEnd(mergingIndices.end());
    for (; mIt != mEnd; ++mIt) {
        unplugActionList(state, mIt);
    }

    for (ContainerNode *child : std::as_const(children)) {
        child->unplugActionList(state);
    }
}

void ContainerNode::unplugActionList(BuildState &state, const MergingIndexList::iterator &mergingIdxIt)
{
    const QLatin1String tagActionList("actionlist");

    MergingIndex mergingIdx = *mergingIdxIt;

    QString k = mergingIdx.mergingName;

    if (k.indexOf(tagActionList) == -1) {
        return;
    }

    k.remove(0, tagActionList.size());

    if (mergingIdx.clientName != state.clientName) {
        return;
    }

    if (k != state.actionListName) {
        return;
    }

    ContainerClient *client = findChildContainerClient(state.guiClient, QString(), mergingIndices.end());

    ActionListMap::Iterator lIt(client->actionLists.find(k));
    if (lIt == client->actionLists.end()) {
        return;
    }

    removeActions(lIt.value());

    client->actionLists.erase(lIt);
}

void ContainerNode::adjustMergingIndices(int offset, const MergingIndexList::iterator &it, const QString &currentClientName)
{
    MergingIndexList::iterator mergingIt = it;
    MergingIndexList::iterator mergingEnd = mergingIndices.end();

    for (; mergingIt != mergingEnd; ++mergingIt) {
        if ((*mergingIt).clientName != currentClientName) {
            (*mergingIt).value += offset;
        }
    }

    index += offset;
}

bool ContainerNode::destruct(
    QDomElement element,
    BuildState &state) // krazy:exclude=passbyvalue (this is correct QDom usage, and a ref wouldn't allow passing doc.documentElement() as argument)
{
    destructChildren(element, state);

    unplugActions(state);

    // remove all merging indices the client defined
    QMutableVectorIterator<MergingIndex> cmIt = mergingIndices;
    while (cmIt.hasNext()) {
        if (cmIt.next().clientName == state.clientName) {
            cmIt.remove();
        }
    }

    // ### check for merging index count, too?
    if (clients.isEmpty() && children.isEmpty() && container && client == state.guiClient) {
        QWidget *parentContainer = nullptr;
        if (parent && parent->container) {
            parentContainer = parent->container;
        }

        Q_ASSERT(builder);
        builder->removeContainer(container, parentContainer, element, containerAction);

        client = nullptr;
        return true;
    }

    if (client == state.guiClient) {
        client = nullptr;
    }

    return false;
}

void ContainerNode::destructChildren(const QDomElement &element, BuildState &state)
{
    QMutableListIterator<ContainerNode *> childIt = children;
    while (childIt.hasNext()) {
        ContainerNode *childNode = childIt.next();

        QDomElement childElement = findElementForChild(element, childNode);

        // destruct returns true in case the container really got deleted
        if (childNode->destruct(childElement, state)) {
            deleteChild(childNode);
            childIt.remove();
        }
    }
}

QDomElement ContainerNode::findElementForChild(const QDomElement &baseElement, ContainerNode *childNode)
{
    // ### slow
    for (QDomNode n = baseElement.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement e = n.toElement();
        if (e.tagName().toLower() == childNode->tagName //
            && e.attribute(QStringLiteral("name")) == childNode->name) {
            return e;
        }
    }

    return QDomElement();
}

void ContainerNode::unplugActions(BuildState &state)
{
    if (!container) {
        return;
    }

    QMutableListIterator<ContainerClient *> clientIt(clients);
    while (clientIt.hasNext()) {
        // only unplug the actions of the client we want to remove, as the container might be owned
        // by a different client
        ContainerClient *cClient = clientIt.next();
        if (cClient->client == state.guiClient) {
            unplugClient(cClient);
            delete cClient;
            clientIt.remove();
        }
    }
}

void ContainerNode::removeActions(const QList<QAction *> &actions)
{
    for (QAction *action : actions) {
        const int pos = container->actions().indexOf(action);
        if (pos != -1) {
            container->removeAction(action);
            for (MergingIndex &idx : mergingIndices) {
                if (idx.value > pos) {
                    --idx.value;
                }
            }
            --index;
        }
    }
}

void ContainerNode::unplugClient(ContainerClient *client)
{
    assert(builder);

    KToolBar *bar = qobject_cast<KToolBar *>(container);
    if (bar) {
        bar->removeXMLGUIClient(client->client);
    }

    // now quickly remove all custom elements (i.e. separators) and unplug all actions
    removeActions(client->customElements);
    removeActions(client->actions);

    // unplug all actionslists

    for (const auto &actionList : std::as_const(client->actionLists)) {
        removeActions(actionList);
    }
}

void ContainerNode::reset()
{
    for (ContainerNode *child : std::as_const(children)) {
        child->reset();
    }

    if (client) {
        client->setFactory(nullptr);
    }
}

int ContainerNode::calcMergingIndex(const QString &mergingName, MergingIndexList::iterator &it, BuildState &state, bool ignoreDefaultMergingIndex)
{
    const MergingIndexList::iterator mergingIt = findIndex(mergingName.isEmpty() ? state.clientName : mergingName);
    const MergingIndexList::iterator mergingEnd = mergingIndices.end();

    if (ignoreDefaultMergingIndex || (mergingIt == mergingEnd && state.currentDefaultMergingIt == mergingEnd)) {
        it = mergingEnd;
        return index;
    }

    if (mergingIt != mergingEnd) {
        it = mergingIt;
    } else {
        it = state.currentDefaultMergingIt;
    }

    return (*it).value;
}

void ContainerNode::dump(int offset)
{
    QString indent;
    indent.fill(QLatin1Char(' '), offset);
    qCDebug(DEBUG_KXMLGUI) << qPrintable(indent) << name << tagName << groupName << mergingName << mergingIndices;
    for (ContainerNode *child : std::as_const(children)) {
        child->dump(offset + 2);
    }
}

// TODO: return a struct with 3 members rather than 1 ret val + 2 out params
int BuildHelper::calcMergingIndex(const QDomElement &element, MergingIndexList::iterator &it, QString &group)
{
    const QLatin1String attrGroup("group");

    bool haveGroup = false;
    group = element.attribute(attrGroup);
    if (!group.isEmpty()) {
        group.prepend(attrGroup);
        haveGroup = true;
    }

    int idx;
    if (haveGroup) {
        idx = parentNode->calcMergingIndex(group, it, m_state, ignoreDefaultMergingIndex);
    } else {
        it = m_state.currentClientMergingIt;
        if (it == parentNode->mergingIndices.end()) {
            idx = parentNode->index;
        } else {
            idx = (*it).value;
        }
    }

    return idx;
}

BuildHelper::BuildHelper(BuildState &state, ContainerNode *node)
    : containerClient(nullptr)
    , ignoreDefaultMergingIndex(false)
    , m_state(state)
    , parentNode(node)
{
    // create a list of supported container and custom tags
    customTags = m_state.builderCustomTags;
    containerTags = m_state.builderContainerTags;

    if (parentNode->builder != m_state.builder) {
        customTags += parentNode->builderCustomTags;
        containerTags += parentNode->builderContainerTags;
    }

    if (m_state.clientBuilder) {
        customTags = m_state.clientBuilderCustomTags + customTags;
        containerTags = m_state.clientBuilderContainerTags + containerTags;
    }

    m_state.currentDefaultMergingIt = parentNode->findIndex(QStringLiteral("<default>"));
    parentNode->calcMergingIndex(QString(), m_state.currentClientMergingIt, m_state, /*ignoreDefaultMergingIndex*/ false);
}

void BuildHelper::build(const QDomElement &element)
{
    for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement e = n.toElement();
        if (e.isNull()) {
            continue;
        }
        processElement(e);
    }
}

void BuildHelper::processElement(const QDomElement &e)
{
    QString tag(e.tagName().toLower());
    QString currName(e.attribute(QStringLiteral("name")));

    const bool isActionTag = (tag == QLatin1String("action"));

    if (isActionTag || customTags.indexOf(tag) != -1) {
        processActionOrCustomElement(e, isActionTag);
    } else if (containerTags.indexOf(tag) != -1) {
        processContainerElement(e, tag, currName);
    } else if (tag == QLatin1String("merge") || tag == QLatin1String("definegroup") || tag == QLatin1String("actionlist")) {
        processMergeElement(tag, currName, e);
    } else if (tag == QLatin1String("state")) {
        processStateElement(e);
    }
}

void BuildHelper::processActionOrCustomElement(const QDomElement &e, bool isActionTag)
{
    if (!parentNode->container) {
        return;
    }

    MergingIndexList::iterator it(m_state.currentClientMergingIt);

    QString group;
    int idx = calcMergingIndex(e, it, group);

    containerClient = parentNode->findChildContainerClient(m_state.guiClient, group, it);

    bool guiElementCreated = false;
    if (isActionTag) {
        guiElementCreated = processActionElement(e, idx);
    } else {
        guiElementCreated = processCustomElement(e, idx);
    }

    if (guiElementCreated) {
        // adjust any following merging indices and the current running index for the container
        parentNode->adjustMergingIndices(1, it, m_state.clientName);
    }
}

bool BuildHelper::processActionElement(const QDomElement &e, int idx)
{
    assert(m_state.guiClient);

    // look up the action and plug it in
    QAction *action = m_state.guiClient->action(e);

    if (!action) {
        return false;
    }

    // qCDebug(DEBUG_KXMLGUI) << e.attribute(QStringLiteral("name")) << "->" << action << "inserting at idx=" << idx;

    QAction *before = nullptr;
    if (idx >= 0 && idx < parentNode->container->actions().count()) {
        before = parentNode->container->actions().at(idx);
    }

    parentNode->container->insertAction(before, action);

    // save a reference to the plugged action, in order to properly unplug it afterwards.
    containerClient->actions.append(action);

    return true;
}

bool BuildHelper::processCustomElement(const QDomElement &e, int idx)
{
    assert(parentNode->builder);

    QAction *action = parentNode->builder->createCustomElement(parentNode->container, idx, e);
    if (!action) {
        return false;
    }

    containerClient->customElements.append(action);
    return true;
}

void BuildHelper::processStateElement(const QDomElement &element)
{
    QString stateName = element.attribute(QStringLiteral("name"));

    if (stateName.isEmpty()) {
        return;
    }

    for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement e = n.toElement();
        if (e.isNull()) {
            continue;
        }

        QString tagName = e.tagName().toLower();

        if (tagName != QLatin1String("enable") && tagName != QLatin1String("disable")) {
            continue;
        }

        const bool processingActionsToEnable = (tagName == QLatin1String("enable"));

        // process action names
        for (QDomNode n2 = n.firstChild(); !n2.isNull(); n2 = n2.nextSibling()) {
            QDomElement actionEl = n2.toElement();
            if (actionEl.tagName().compare(QLatin1String("action"), Qt::CaseInsensitive) != 0) {
                continue;
            }

            QString actionName = actionEl.attribute(QStringLiteral("name"));
            if (actionName.isEmpty()) {
                return;
            }

            if (processingActionsToEnable) {
                m_state.guiClient->addStateActionEnabled(stateName, actionName);
            } else {
                m_state.guiClient->addStateActionDisabled(stateName, actionName);
            }
        }
    }
}

void BuildHelper::processMergeElement(const QString &tag, const QString &name, const QDomElement &e)
{
    const QLatin1String tagDefineGroup("definegroup");
    const QLatin1String tagActionList("actionlist");
    const QLatin1String defaultMergingName("<default>");
    const QLatin1String attrGroup("group");

    QString mergingName(name);
    if (mergingName.isEmpty()) {
        if (tag == tagDefineGroup) {
            qCCritical(DEBUG_KXMLGUI) << "cannot define group without name!";
            return;
        }
        if (tag == tagActionList) {
            qCCritical(DEBUG_KXMLGUI) << "cannot define actionlist without name!";
            return;
        }
        mergingName = defaultMergingName;
    }

    if (tag == tagDefineGroup) {
        mergingName.prepend(attrGroup); // avoid possible name clashes by prepending
                                        // "group" to group definitions
    } else if (tag == tagActionList) {
        mergingName.prepend(tagActionList);
    }

    if (parentNode->findIndex(mergingName) != parentNode->mergingIndices.end()) {
        return; // do not allow the redefinition of merging indices!
    }

    MergingIndexList::iterator mIt(parentNode->mergingIndices.end());

    QString group(e.attribute(attrGroup));
    if (!group.isEmpty()) {
        group.prepend(attrGroup);
    }

    // calculate the index of the new merging index. Usually this does not need any calculation,
    // we just want the last available index (i.e. append) . But in case the <Merge> tag appears
    // "inside" another <Merge> tag from a previously build client, then we have to use the
    // "parent's" index. That's why we call calcMergingIndex here.
    MergingIndex newIdx;
    newIdx.value = parentNode->calcMergingIndex(group, mIt, m_state, ignoreDefaultMergingIndex);
    newIdx.mergingName = mergingName;
    newIdx.clientName = m_state.clientName;

    // if that merging index is "inside" another one, then append it right after the "parent".
    if (mIt != parentNode->mergingIndices.end()) {
        parentNode->mergingIndices.insert(++mIt, newIdx);
    } else {
        parentNode->mergingIndices.append(newIdx);
    }

    if (mergingName == defaultMergingName) {
        ignoreDefaultMergingIndex = true;
    }

    // re-calculate the running default and client merging indices
    // (especially important in case the QList data got reallocated due to growing!)
    m_state.currentDefaultMergingIt = parentNode->findIndex(defaultMergingName);
    parentNode->calcMergingIndex(QString(), m_state.currentClientMergingIt, m_state, ignoreDefaultMergingIndex);
}

void BuildHelper::processContainerElement(const QDomElement &e, const QString &tag, const QString &name)
{
    ContainerNode *containerNode = parentNode->findContainer(name, tag, &containerList, m_state.guiClient);

    if (!containerNode) {
        MergingIndexList::iterator it(m_state.currentClientMergingIt);
        QString group;

        int idx = calcMergingIndex(e, it, group);

        QAction *containerAction;

        KXMLGUIBuilder *builder;

        QWidget *container = createContainer(parentNode->container, idx, e, containerAction, &builder);

        // no container? (probably some <text> tag or so ;-)
        if (!container) {
            return;
        }

        parentNode->adjustMergingIndices(1, it, m_state.clientName);

        // Check that the container widget is not already in parentNode.
        Q_ASSERT(std::find_if(parentNode->children.constBegin(),
                              parentNode->children.constEnd(),
                              [container](ContainerNode *child) {
                                  return child->container == container;
                              })
                 == parentNode->children.constEnd());

        containerList.append(container);

        QString mergingName;
        if (it != parentNode->mergingIndices.end()) {
            mergingName = (*it).mergingName;
        }

        QStringList cusTags = m_state.builderCustomTags;
        QStringList conTags = m_state.builderContainerTags;
        if (builder != m_state.builder) {
            cusTags = m_state.clientBuilderCustomTags;
            conTags = m_state.clientBuilderContainerTags;
        }

        containerNode = new ContainerNode(container, tag, name, parentNode, m_state.guiClient, builder, containerAction, mergingName, group, cusTags, conTags);
    } else {
        if (tag == QLatin1String("toolbar")) {
            KToolBar *bar = qobject_cast<KToolBar *>(containerNode->container);
            if (bar) {
                if (m_state.guiClient && !m_state.guiClient->xmlFile().isEmpty()) {
                    bar->addXMLGUIClient(m_state.guiClient);
                }
            } else {
                qCWarning(DEBUG_KXMLGUI) << "toolbar container is not a KToolBar";
            }
        }
    }

    BuildHelper(m_state, containerNode).build(e);

    // and re-calculate running values, for better performance
    m_state.currentDefaultMergingIt = parentNode->findIndex(QStringLiteral("<default>"));
    parentNode->calcMergingIndex(QString(), m_state.currentClientMergingIt, m_state, ignoreDefaultMergingIndex);
}

QWidget *BuildHelper::createContainer(QWidget *parent, int index, const QDomElement &element, QAction *&containerAction, KXMLGUIBuilder **builder)
{
    QWidget *res = nullptr;

    if (m_state.clientBuilder) {
        res = m_state.clientBuilder->createContainer(parent, index, element, containerAction);

        if (res) {
            *builder = m_state.clientBuilder;
            return res;
        }
    }

    KXMLGUIClient *oldClient = m_state.builder->builderClient();

    m_state.builder->setBuilderClient(m_state.guiClient);

    res = m_state.builder->createContainer(parent, index, element, containerAction);

    m_state.builder->setBuilderClient(oldClient);

    if (res) {
        *builder = m_state.builder;
    }

    return res;
}

void BuildState::reset()
{
    clientName.clear();
    actionListName.clear();
    actionList.clear();
    guiClient = nullptr;
    clientBuilder = nullptr;

    currentDefaultMergingIt = currentClientMergingIt = MergingIndexList::iterator();
}

QDebug operator<<(QDebug stream, const MergingIndex &mi)
{
    QDebugStateSaver saver(stream);
    stream.nospace() << "clientName=" << mi.clientName << " mergingName=" << mi.mergingName << " value=" << mi.value;
    return stream;
}
