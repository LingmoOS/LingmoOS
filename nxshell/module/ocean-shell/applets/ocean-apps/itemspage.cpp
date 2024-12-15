// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "itemspage.h"

#include <QSet>
#include <QDebug>

ItemsPage::ItemsPage(const QString &name, int maxItemCountPerPage, QObject *parent)
    : QObject(parent)
    , m_maxItemCountPerPage(maxItemCountPerPage)
    , m_displayName(name)
{
    Q_ASSERT(m_maxItemCountPerPage >= 1);
}

ItemsPage::ItemsPage(int maxItemCountPerPage, QObject *parent)
    : ItemsPage(QString(), maxItemCountPerPage, parent)
{

}

ItemsPage::~ItemsPage()
{

}

QString ItemsPage::name() const
{
    return m_displayName;
}

void ItemsPage::setName(const QString &name)
{
    m_displayName = name;
    emit nameChanged();
}

int ItemsPage::maxItemCountPerPage() const
{
    return m_maxItemCountPerPage;
}

int ItemsPage::pageCount() const
{
    return m_pages.size();
}

QStringList ItemsPage::items(int page) const
{
    return m_pages.at(page);
}

QStringList ItemsPage::firstNItems(int count)
{
    QStringList result;

    for (const QStringList & pageItems : std::as_const(m_pages)) {
        for (const QString & item : pageItems) {
            result.append(item);
            if (result.count() >= count) {
                return result;
            }
        }
    }

    return result;
}

void ItemsPage::appendEmptyPage()
{
    m_pages.append(QStringList());

    emit pageCountChanged();
    emit sigPageAoceand(m_pages.count() - 1, m_pages.count() - 1);
}

// if length of items larger than max item count per page, there will be another page get appended
void ItemsPage::appendPage(const QStringList items)
{
    const int len = items.size();
    const int pages = len / m_maxItemCountPerPage;

    if (len == 0) return;

    int first = m_pages.count();

    QList<QString>::const_iterator begin = items.constBegin();

    for (int i = 1; i <= pages; i++) {
        QStringList newList;
        std::copy(begin, begin + m_maxItemCountPerPage, std::back_inserter(newList));
        m_pages.append(newList);

        begin += m_maxItemCountPerPage;
    }

    if (begin != items.constEnd()) {
        QStringList newList;
        std::copy(begin, items.end(), std::back_inserter(newList));
        m_pages.append(newList);
    }

    emit pageCountChanged();
    emit sigPageAoceand(first, m_pages.count() - 1);
}

// find a page with empty place and append the item to that page.
void ItemsPage::appendItem(const QString id, int page)
{
    Q_ASSERT(page < m_pages.count());

    if (page < 0) page = qMax(m_pages.count() - 1, 0);

    const int pageCount = m_pages.count();

    for (; page < pageCount; page++) {
        if (m_pages.at(page).size() < m_maxItemCountPerPage) break;
    }

    if (page == pageCount) {
        appendPage({id});
    } else {
        m_pages[page].append(id);
    }
}

void ItemsPage::insertItem(const QString id, int page, int pos)
{
    Q_ASSERT(page >= 0 && page < m_pages.size());

    if (m_pages[page].count() < pos)
        pos = m_pages[page].count();

    m_pages[page].insert(pos, id);
    if (m_pages[page].size() > m_maxItemCountPerPage) {
        QString last = m_pages[page].takeLast();
        if (page + 1 == m_pages.size()) {
            appendPage({last});
        } else {
            insertItem(last, page + 1, 0);
        }
    }
}

void ItemsPage::insertItemToPage(const QString &id, int toPage)
{
    Q_ASSERT(toPage < m_pages.count());

    if (toPage < 0) toPage = qMax(m_pages.count() - 1, 0);

    insertItem(id, toPage, m_pages[toPage].count());
}

void ItemsPage::moveItemPosition(int fromPage, int fromIndex, int toPage, int toIndex, bool appendToIndexItem)
{
    if (fromPage == toPage && fromIndex > toIndex && appendToIndexItem) {
        if (fromIndex == (toIndex+1)) {
            // same page, append operate, do nothing
            return;
        }

        toIndex += 1;
    }

    bool needRemoveEmptyPage = false;
    if (m_pages[fromPage].count() == 1) {
        needRemoveEmptyPage = true;
    }

    moveItem(fromPage, fromIndex, toPage, toIndex);

    if (needRemoveEmptyPage) {
        removeEmptyPages();
    }
}

bool ItemsPage::removeItem(const QString id, bool removePageIfPageIsEmpty)
{
    int page, idx;
    std::tie(page, idx) = findItem(id);

    if (idx != -1) {
        m_pages[page].removeAt(idx);

        if (removePageIfPageIsEmpty && m_pages.at(page).isEmpty()) {
            m_pages.removeAt(page);
            emit pageCountChanged();
            emit sigPageRemoved(m_pages.count() - 1, m_pages.count() - 1);
        }

        return true;
    }

    return false;
}

void ItemsPage::removeItemsNotIn(const QSet<QString> &itemSet)
{
    for (int i = 0; i < m_pages.count(); i++) {
        for (int j = m_pages.at(i).count() - 1; j >= 0; j--) {
            const QString & item(m_pages.at(i).at(j));
            if (itemSet.contains(item)) continue;
            if (item.startsWith(QLatin1String("internal/"))) continue;
            m_pages[i].removeAt(j);
        }
    }
    removeEmptyPages();
}

void ItemsPage::removeEmptyPages()
{
    int count = m_pages.size();
    m_pages.removeAll({});
    if (count != m_pages.size()) {
        emit pageCountChanged();
        emit sigPageRemoved(m_pages.count() - 1, m_pages.count() - 1);
    }
}

// <page, index>
std::tuple<int, int> ItemsPage::findItem(const QString &id) const
{
    for (int i = 0; i < m_pages.count(); i++) {
        int idx = m_pages.at(i).indexOf(id);
        if (idx != -1) {
            return std::make_tuple(i, idx);
        }
    }

    return std::make_tuple(-1, -1);
}

bool ItemsPage::contains(const QString &id) const
{
    int i, j;
    std::tie(i, j) = findItem(id);
    return i != -1;
}

QStringList ItemsPage::allArrangedItems() const
{
    QStringList result;
    for (const QStringList &pageItems : std::as_const(m_pages)) {
        result.append(pageItems);
    }

    return result;
}

QList<QStringList> ItemsPage::allPagedItems() const
{
    return m_pages;
}

int ItemsPage::itemCount() const 
{
    int count = 0;
    for (const QStringList &pageItems : m_pages) {
        count += pageItems.count();
    }
    return count;
}

int ItemsPage::itemCount(int page) const
{
    if (page >= pageCount())
        return 0;

    return items(page).count();
}

// item will be moved to the index
void ItemsPage::moveItem(int fromPage, int fromIndex, int toPage, int toIndex)
{
    if (fromPage == toPage) {
        auto toIndexItem = m_pages[fromPage].takeAt(fromIndex);
        m_pages[fromPage].insert(toIndex, toIndexItem);
    } else {
        QString id = m_pages[fromPage].takeAt(fromIndex);
        insertItem(id, toPage, toIndex);
    }
}
