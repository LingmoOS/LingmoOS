// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "generalpreviewplugin_p.h"
#include "generalpreviewplugin.h"
#include "utils/utils.h"
#include "global/commontools.h"

#include <DFontSizeManager>
#include <DMessageBox>
#include <DGuiApplicationHelper>

#include <QIcon>
#include <QGuiApplication>
#include <QClipboard>
#include <QToolButton>
#include <QPainter>

#define ICON_SIZE               96
#define HOR_MARGIN_SIZE         10
#define MARGIN_SIZE             15
#define NAME_WIDTH              239

using namespace GrandSearch;

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

NameLabel::NameLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
    QLabel(text, parent, f)
{
    setObjectName("NameLabel");
    setFixedWidth(NAME_WIDTH);

    QFont titleFont = this->font();
    titleFont.setWeight(QFont::Medium);
    titleFont = DFontSizeManager::instance()->get(DFontSizeManager::T5, titleFont);
    this->setFont(titleFont);

    QColor textColor(0, 0, 0, int(255 * 0.9));
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        textColor = QColor(255, 255, 255, int(255 * 0.9));
    QPalette pa = palette();
    pa.setColor(QPalette::WindowText, textColor);
    setPalette(pa);

    setAlignment(Qt::AlignBottom | Qt::AlignLeft);
}

SizeLabel::SizeLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
    QLabel(text, parent, f)
{
    setObjectName("SizeLabel");
    setFixedWidth(NAME_WIDTH);
    QColor textColor(0, 0, 0, int(255 * 0.4));
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        textColor = QColor(255, 255, 255, int(255 * 0.4));
    QPalette pa = palette();
    pa.setColor(QPalette::WindowText, textColor);
    setPalette(pa);

    setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
}

GeneralPreviewPluginPrivate::GeneralPreviewPluginPrivate(GeneralPreviewPlugin *parent)
    : q_p(parent)
    , m_contentWidget(new QWidget())
{
    // 图标和名称
    m_iconLabel = new QLabel(m_contentWidget);
    m_iconLabel->setObjectName("IconLabel");
    m_iconLabel->setFixedSize(QSize(ICON_SIZE, ICON_SIZE));
    m_nameLabel = new NameLabel("", m_contentWidget);
    m_sizeLabel = new SizeLabel("", m_contentWidget);

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    vLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
    vLayout->addWidget(m_nameLabel);
    vLayout->addWidget(m_sizeLabel);
    vLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(HOR_MARGIN_SIZE - 3, MARGIN_SIZE, HOR_MARGIN_SIZE, MARGIN_SIZE);
    hLayout->setSpacing(HOR_MARGIN_SIZE);
    hLayout->addWidget(m_iconLabel);
    hLayout->addLayout(vLayout);

    m_vMainLayout = new QVBoxLayout(m_contentWidget);

    m_vMainLayout->setContentsMargins(MARGIN_SIZE, 0, MARGIN_SIZE, MARGIN_SIZE);
    m_vMainLayout->addLayout(hLayout);
}

GeneralPreviewPluginPrivate::~GeneralPreviewPluginPrivate()
{
    m_contentWidget->deleteLater();
}

GeneralPreviewPlugin::GeneralPreviewPlugin(QObject *parent)
    : QObject(parent)
    , PreviewPlugin()
    , d_p(new GeneralPreviewPluginPrivate(this))
{

}

GeneralPreviewPlugin::~GeneralPreviewPlugin()
{
    if (d_p->m_sizeWorker) {
        d_p->m_sizeWorker->stop();
        d_p->m_sizeWorker->wait();
        d_p->m_sizeWorker->deleteLater();
    }
}

void GeneralPreviewPlugin::init(QObject *proxyInter)
{
    Q_UNUSED(proxyInter)
}

bool GeneralPreviewPlugin::previewItem(const ItemInfo &info)
{
    MatchedItem item;
    item.item = info[PREVIEW_ITEMINFO_ITEM];
    item.name = info[PREVIEW_ITEMINFO_NAME];
    item.icon = info[PREVIEW_ITEMINFO_ICON];
    item.type = info[PREVIEW_ITEMINFO_TYPE];
    item.searcher = info[PREVIEW_ITEMINFO_SEARCHER];

    if (!item.item.isEmpty()
            && !item.name.isEmpty()
            && d_p->m_item.item == item.item
            && d_p->m_item.name == item.name)
        return true;

    d_p->m_item = item;

    // 设置图标
    QPixmap pixmap;
    const QSize iconSize(ICON_SIZE, ICON_SIZE);
    const QString &strIcon = item.icon;
    if (!strIcon.isEmpty()) {
        // 判断icon是路径还是图标名
        if (strIcon.contains('/')) {
            QFileInfo file(strIcon);
            if (file.exists())
                pixmap = QIcon(strIcon).pixmap(iconSize);
            else
                pixmap = Utils::defaultIcon(item).pixmap(iconSize);
        } else {
            QIcon icon = QIcon::fromTheme(strIcon);
            if (icon.isNull())
                pixmap = Utils::defaultIcon(item).pixmap(iconSize);
            else
                pixmap = icon.pixmap(iconSize);
        }
    } else {
        pixmap = Utils::defaultIcon(item).pixmap(iconSize);
    }
    d_p->m_iconLabel->setPixmap(pixmap);

    // 设置名称，并计算换行内容
    QString elidedText = CommonTools::lineFeed(item.name, d_p->m_nameLabel->width(), d_p->m_nameLabel->font(), 2);
    d_p->m_nameLabel->setText(elidedText);
    if (elidedText != item.name)
        d_p->m_nameLabel->setToolTip(item.name);

    QFileInfo fi(item.item);

    // 获取文件(夹)大小
    if (fi.isDir()) {
        if (!d_p->m_sizeWorker) {
            d_p->m_sizeWorker = new FileStatisticsThread(this);
            connect(d_p->m_sizeWorker, &FileStatisticsThread::dataNotify, this, &GeneralPreviewPlugin::updateFolderSize);
        } else if (d_p->m_sizeWorker->isRunning()) {
            d_p->m_sizeWorker->stop();
            d_p->m_sizeWorker->wait();
        }

        d_p->m_sizeLabel->setText(CommonTools::formatFileSize(0));
        d_p->m_sizeWorker->start(item.item);
    } else {
        d_p->m_sizeLabel->setText(CommonTools::formatFileSize(fi.size()));
    }

    // 设置属性详情信息
    d_p->m_detailInfos.clear();

    DetailTagInfo tagInfos;
    tagInfos.insert(DetailInfoProperty::Text, QVariant(tr("Location:")));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    DetailContentInfo contentInfos;
    contentInfos.insert(DetailInfoProperty::Text, QVariant(fi.absoluteFilePath()));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideMiddle));

    DetailInfo detailInfo = qMakePair(tagInfos, contentInfos);
    d_p->m_detailInfos.push_back(detailInfo);

    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(tr("Time modified:")));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(fi.lastModified().toString(CommonTools::dateTimeFormat())));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideMiddle));

    detailInfo = qMakePair(tagInfos, contentInfos);
    d_p->m_detailInfos.push_back(detailInfo);

    return true;
}

ItemInfo GeneralPreviewPlugin::item() const
{
    ItemInfo itemInfo;
    itemInfo[PREVIEW_ITEMINFO_ITEM] = d_p->m_item.item;
    itemInfo[PREVIEW_ITEMINFO_NAME] = d_p->m_item.name;
    itemInfo[PREVIEW_ITEMINFO_ICON] = d_p->m_item.icon;
    itemInfo[PREVIEW_ITEMINFO_TYPE] = d_p->m_item.type;
    itemInfo[PREVIEW_ITEMINFO_SEARCHER] = d_p->m_item.searcher;

    return itemInfo;
}

bool GeneralPreviewPlugin::stopPreview()
{
    return true;
}

QWidget *GeneralPreviewPlugin::contentWidget() const
{
    return d_p->m_contentWidget;
}

DetailInfoList GeneralPreviewPlugin::getAttributeDetailInfo() const
{
    return d_p->m_detailInfos;
}

QWidget *GeneralPreviewPlugin::toolBarWidget() const
{
    return nullptr;
}

bool GeneralPreviewPlugin::showToolBar() const
{
    return true;
}

void GeneralPreviewPlugin::updateFolderSize(qint64 size)
{
    d_p->m_sizeLabel->setText(CommonTools::formatFileSize(size));
}
