// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "zonechooseview.h"

#include "global.h"
#include "utils/timezone.h"

#include <DPinyin>
#include <DLabel>
#include <DSearchEdit>
#include <QDebug>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QAbstractButton>
#include <QTimer>

DWIDGET_USE_NAMESPACE
namespace dwclock {
static QString getTimezoneCity(const QString &timezone)
{
    static const QString locale = QLocale::system().name();
    return installer::GetLocalTimezoneName(timezone, locale);
}
static QStringList timezones()
{
    QStringList zones;
    for (const auto &zoneInfo : installer::GetZoneInfoList()) {
        auto timezone = zoneInfo.timezone;
        zones << timezone;
    }
    return zones;
}

static bool isChineseSystem()
{
    static const QString locale = QLocale::system().name();
    static const QStringList chineseLocales{u8"zh_CN"};
    static bool isChinese = chineseLocales.contains(locale);
    return isChinese;
}

static QString toChinesePinyin(const QString &s)
{
    static const QList<char> toneNumber{'1', '2', '3', '4'};
    QString tmp = Dtk::Core::Chinese2Pinyin(s);
    // TODO performance optimization
    for (auto item : toneNumber)
        tmp = tmp.replace(item, "");

    return tmp;
}

TimezoneListModel::TimezoneListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int TimezoneListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    if (m_data.isEmpty())
        m_data = timezones();

    return m_data.count();
}

QVariant TimezoneListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const auto row = index.row();

    // TODO performance optimization
    if (role == Qt::EditRole || role == Qt::DisplayRole || role == TimezoneListModel::CityName) {
        return getTimezoneCity(m_data[row]);
    } else if (role == TimezoneListModel::ZoneName) {
        return m_data[row];
    }

    return QVariant();
}

ZoneProxyModel::ZoneProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool ZoneProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const auto &rowIndex = sourceModel()->index(source_row, 0, source_parent);
    if (!rowIndex.isValid())
        return true;

    const QString &display = sourceModel()->data(rowIndex).toString();

    if (filterRegExp().isEmpty())
        return true;

    if (display.contains(filterRegExp()))
        return true;

    if (isChineseSystem()) {
        if (toChinesePinyin(display).contains(filterRegExp()))
            return true;
    }
    return false;
}

ZoneSectionProxyModel::ZoneSectionProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

void ZoneSectionProxyModel::setSectionKey(const QString &sectionKey)
{
    if (m_sectionKey == sectionKey)
        return;

    m_sectionKey = sectionKey;
    invalidateFilter();
}

bool ZoneSectionProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const auto &rowIndex = sourceModel()->index(source_row, 0, source_parent);
    if (!rowIndex.isValid())
        return true;

    const QString &display = sourceModel()->data(rowIndex).toString();
    if (display.isEmpty())
        return false;

    QString key;
    if (isChineseSystem()) {
        key = toChinesePinyin(display[0]);
    } else {
        key = sourceModel()->data(rowIndex, TimezoneListModel::ZoneName).toString();
    }

    if (key.isEmpty())
        return false;

    if (key.startsWith(m_sectionKey, Qt::CaseInsensitive))
        return true;

    return false;
}

ZoneSection::ZoneSection(const QString &sectionKey, QWidget *parent)
    : QWidget(parent)
    , m_sectionKey(sectionKey)
{
    auto layout = new QVBoxLayout(this);
    auto margins = layout->contentsMargins();
    margins.setLeft(UI::clock::defaultMargins.left());
    margins.setRight(UI::clock::defaultMargins.right());
    layout->setContentsMargins(margins);

    auto title = new DLabel(sectionKey);
    title->setIndent(UI::edit::sectionTitleIndent);
    layout->addWidget(title);

    m_zones = new ZoneSectionView();
    layout->addWidget(m_zones);

    layout->addStretch();

    connect(m_zones, SIGNAL(currentChanged(QModelIndex)), this, SIGNAL(currentChanged(QModelIndex)));
}

void ZoneSection::setModel(QAbstractItemModel *model)
{
    ZoneSectionProxyModel *proxyModel = new ZoneSectionProxyModel();
    proxyModel->setSourceModel(model);
    proxyModel->setSectionKey(m_sectionKey);

    m_zones->setModel(proxyModel);
}

int ZoneSection::rowCount() const
{
    return m_zones->model()->rowCount();
}

void ZoneSection::clearCurrentIndex()
{
    m_zones->setCurrentIndex(QModelIndex());
}

QString ZoneSection::currentZone() const
{
    const auto &index = m_zones->currentIndex();
    if (!index.isValid())
        return QString();

    return m_zones->model()->data(index, TimezoneListModel::ZoneName).toString();
}

ZoneSectionView::ZoneSectionView(QWidget *parent)
    : DListView(parent)
{
    setEditTriggers(QListView::NoEditTriggers);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setBackgroundType(DStyledItemDelegate::ClipCornerBackground);
    setItemSize(UI::edit::itemSize);
}

bool ZoneSectionView::event(QEvent *event)
{
    // TODO: need to setFixedHeight Mini Height, other it's default contentSize is 150 px.
    static const int MinHeight = 1;
    switch (event->type()) {
    case QEvent::Polish:
        setFixedHeight(MinHeight);
        break;
    case QEvent::Paint:
    case QEvent::LayoutRequest: {
        if (model() && model()->rowCount() <= 0) {
            setFixedHeight(MinHeight);
        } else {
            setFixedHeight(contentsSize().height());
        }
    } break;
    default:
        break;
    }
    return DListView::event(event);
}

ZoneChooseView::ZoneChooseView(QWidget *parent)
    : DDialog(parent)
{
    setFixedSize(UI::edit::chooseDialogSize);

    setTitle(tr("Modify City"));
    addSpacing(UI::edit::spacingTitle);
    m_search = new DSearchEdit();
    addContent(m_search);
    addButton(tr("Cancel"));
    const int okBtnIndex = addButton(tr("OK"));
    m_okBtn = getButton(okBtnIndex);
    m_okBtn->setEnabled(false);

    auto scrollView = new QScrollArea();
    auto pt = scrollView->palette();
    pt.setBrush(QPalette::Window, Qt::transparent);
    scrollView->setPalette(pt);

    scrollView->setWidgetResizable(true);
    scrollView->setFrameShape(QFrame::NoFrame);

    m_proxyModel = new ZoneProxyModel();
    auto model = new TimezoneListModel();
    m_proxyModel->setSourceModel(model);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    auto zonesView = fillZones();

    scrollView->setWidget(zonesView);
    addContent(scrollView);

    connect(m_search, &DSearchEdit::textChanged, this, &ZoneChooseView::onSearchTextChanged);
    // using QueuedConnection way to fresh section's visible, because it's section are alwayse hide before exec.
    QTimer::singleShot(0, this, [this]() {
        onSearchTextChanged(QString());
    });
}

QWidget *ZoneChooseView::fillZones()
{
    auto zonesView = new QWidget();
    auto layout = new QVBoxLayout(zonesView);
    layout->setContentsMargins(UI::clock::defaultMargins);

    for (char sectionIndex = 'A'; sectionIndex <= 'Z'; sectionIndex++) {
        ZoneSection *section = new ZoneSection(QString(sectionIndex));
        section->setModel(m_proxyModel);
        m_sections << section;
        connect(section, &ZoneSection::currentChanged, this, &ZoneChooseView::onCurrentChanged);
        layout->addWidget(section);
    }

    layout->addStretch(1);

    return zonesView;
}

QString ZoneChooseView::currentZone() const
{
    if (!m_currentSection)
        return QString();

    return m_currentSection->currentZone();
}

void ZoneChooseView::onSearchTextChanged(const QString &text)
{
    m_proxyModel->setFilterRegExp(text);
    for (auto section : m_sections) {
        bool shouldShow = section->rowCount() > 0;
        if (shouldShow == section->isVisible())
            continue;

        section->setVisible(shouldShow);
    }
}

void ZoneChooseView::onCurrentChanged(const QModelIndex &previous)
{
    Q_UNUSED(previous);

    auto section = qobject_cast<ZoneSection *>(sender());
    if (section == m_currentSection)
        return;

    m_currentSection = section;
    for (auto item : m_sections) {
        if (section == item)
            continue;

        item->clearCurrentIndex();
    }

    m_okBtn->setEnabled(!currentZone().isEmpty());
}
}
