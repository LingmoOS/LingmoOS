// SPDX-FileCopyrightText: 2015 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "kblayoutlistview.h"

#include <DStyle>

const int ITEM_WIDTH = 200;
const int ITEM_HEIGHT = 34;
const int ITEM_SPACING = 2;

KBLayoutListView::KBLayoutListView(const QString &language, QWidget *parent)
    : DListView(parent)
    , m_xkbParse(new XkbParser(this))
    , m_buttonModel(new QStandardItemModel(this))
    , m_curLanguage(language)
    , m_clickState(false)
{
    initUI();
}

KBLayoutListView::~KBLayoutListView()
{
}

void KBLayoutListView::initData(const QStringList &buttons)
{
    if (buttons == m_buttons)
        return;

    m_buttons = buttons;
    m_kbdParseList = m_xkbParse->lookUpKeyboardList(m_buttons);

    if (m_kbdParseList.isEmpty())
        m_kbdParseList = buttons;

    // 获取当前输入法名称-全名,而不是简写的英文字符串
    if (!m_xkbParse->lookUpKeyboardList(QStringList(m_curLanguage)).isEmpty())
        m_curLanguage = m_xkbParse->lookUpKeyboardList(QStringList(m_curLanguage)).at(0);

    m_buttonModel->clear();

    resize(width(), DDESESSIONCC::LAYOUTBUTTON_HEIGHT * m_kbdParseList.count());
    for (int i = 0; i < m_kbdParseList.size(); i++)
        addItem(m_kbdParseList[i]);

    updateSelectState(m_curLanguage);

    setFixedHeight(m_buttonModel->rowCount() * ITEM_HEIGHT + m_buttonModel->rowCount() * 2 * ITEM_SPACING);
}

void KBLayoutListView::initUI()
{
    setFrameShape(QFrame::NoFrame);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setViewportMargins(0, 0, 0, 0);
    setSpacing(ITEM_SPACING);
    setItemSpacing(0);
    setItemSize(QSize(ITEM_WIDTH, ITEM_HEIGHT));

    setModel(m_buttonModel);

    connect(this, &KBLayoutListView::clicked, this, &KBLayoutListView::onItemClick);
}

void KBLayoutListView::updateSelectState(const QString &name)
{
    for (int i = 0; i < m_buttonModel->rowCount(); i++) {
        auto item = static_cast<DStandardItem *>(m_buttonModel->item(i));
        auto action = item->actionList(Qt::Edge::RightEdge).first();
        if (item->text() != m_curLanguage) {
            action->setIcon(QIcon());
            update(item->index());
            continue;
        }

        auto dstyle = qobject_cast<DStyle *>(style());
        QIcon icon = dstyle ? dstyle->standardIcon(DStyle::SP_MarkElement) : QIcon();
        action->setIcon(icon);
        setCurrentIndex(item->index());
        update(item->index());

        // 告知外部,更新当前输入法显示内容
        QString kbd = m_buttons[m_kbdParseList.indexOf(name)];
        if (m_clickState)
            emit itemClicked(kbd);
    }
}

/**选中当前输入法
 * @brief KBLayoutListView::updateList
 * @param str
 */
void KBLayoutListView::updateList(const QString &str)
{
    if (!m_xkbParse->lookUpKeyboardList(QStringList(str)).isEmpty())
        m_curLanguage = m_xkbParse->lookUpKeyboardList(QStringList(str)).at(0);

    m_clickState = false;
    updateSelectState(m_curLanguage);
}

void KBLayoutListView::onItemClick(const QModelIndex &index)
{
    const QString &name = index.data().toString();
    m_curLanguage = name;
    m_clickState = true;

    // 更新选中状态
    updateSelectState(name);
}

void KBLayoutListView::addItem(const QString &name)
{
    DStandardItem *item = new DStandardItem(name);
    item->setFontSize(DFontSizeManager::T6);
    QSize iconSize(12, 10);
    auto rightAction = new DViewItemAction(Qt::AlignVCenter, iconSize, iconSize, true);
    QIcon icon;
    if (name != m_curLanguage) {
        icon = QIcon();
    } else {
        icon = qobject_cast<DStyle *>(style())->standardIcon(DStyle::SP_MarkElement);
    }

    rightAction->setIcon(icon);
    item->setActionList(Qt::Edge::RightEdge, { rightAction });
    m_buttonModel->appendRow(item);
}