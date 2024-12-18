// SPDX-FileCopyrightText: 2021 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/private/folderlistwidget_p.h"
#include "views/folderlistwidget.h"
#include "views/folderviewdelegate.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/application/application.h>

#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QKeyEvent>

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_titlebar;
DFMBASE_USE_NAMESPACE

static constexpr int kMaxFolderCount = 8;
static constexpr int kFolderBatchSize = 2000;
static constexpr int kFolderListItemMargin { 6 };

FolderListWidgetPrivate::FolderListWidgetPrivate(FolderListWidget *qq)
    : QObject(qq), q(qq)
{
    q->resize(172, kFolderListItemMargin * 2 + kFolderItemHeight * 8);
    q->setMouseTracking(true);
    layout = new QVBoxLayout(qq);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    folderModel = new QStandardItemModel(this);
    folderView = new DListView(qq);
    folderView->setMouseTracking(true);
    folderView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    folderView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    folderView->setViewportMargins(kFolderListItemMargin, kFolderListItemMargin, kFolderListItemMargin, kFolderListItemMargin);
    folderView->setUniformItemSizes(true);
    folderView->setViewMode(QListView::ListMode);
    folderView->setResizeMode(QListView::Adjust);
    folderView->setMovement(QListView::Static);
    folderView->setLayoutMode(QListView::Batched);
    folderView->setBatchSize(kFolderBatchSize);

    folderView->setModel(folderModel);
    folderDelegate = new FolderViewDelegate(folderView);
    folderView->setItemDelegate(folderDelegate);

    layout->addWidget(folderView);
    q->setLayout(layout);

    initConnect();
}

FolderListWidgetPrivate::~FolderListWidgetPrivate()
{
}

void FolderListWidgetPrivate::initConnect()
{
    connect(folderView, &DListView::clicked, this, &FolderListWidgetPrivate::clicked);
}

void FolderListWidgetPrivate::clicked(const QModelIndex &index)
{
    q->hide();

    if (index.isValid()) {
        int row = index.data(Qt::UserRole).toInt();
        if (row >= 0 && row < crumbDatas.size()) {
            Q_EMIT q->urlButtonActivated(crumbDatas[row].url);
        }
    }
}

void FolderListWidgetPrivate::returnPressed()
{
    if (!folderView) {
        q->hide();
        return;
    }
    clicked(folderView->currentIndex());
}

void FolderListWidgetPrivate::selectUp()
{
    if (!folderView) {
        q->hide();
        return;
    }
    auto curIndex = folderView->currentIndex();
    if (!curIndex.isValid()) {
        if (folderModel->rowCount() > 0)
            curIndex = folderModel->index(folderModel->rowCount() - 1, 0);
    } else {
        int row = curIndex.row() - 1;
        if (row < 0)
            row = folderModel->rowCount() - 1;
        curIndex = folderModel->index(row, 0);
    }
    if (curIndex.isValid())
        folderView->setCurrentIndex(curIndex);
}

void FolderListWidgetPrivate::selectDown()
{
    if (!folderView) {
        q->hide();
        return;
    }
    auto curIndex = folderView->currentIndex();
    if (!curIndex.isValid()) {
        if (folderModel->rowCount() > 0)
            curIndex = folderModel->index(0, 0);
    } else {
        int row = curIndex.row() + 1;
        if (row >= folderModel->rowCount())
            row = 0;
        curIndex = folderModel->index(row, 0);
    }
    if (curIndex.isValid())
        folderView->setCurrentIndex(curIndex);
}

FolderListWidget::FolderListWidget(QWidget *parent)
    : DBlurEffectWidget(parent), d(new FolderListWidgetPrivate(this))
{
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setBlurEnabled(true);
    setMode(DBlurEffectWidget::GaussianBlur);
}

FolderListWidget::~FolderListWidget() = default;

void FolderListWidget::setFolderList(const QList<CrumbData> &datas, bool stacked)
{
    d->folderModel->clear();
    int dataNum = 0;
    d->crumbDatas = datas;
    bool isShowedHiddenFiles = true;
    if (!stacked)
        isShowedHiddenFiles = Application::instance()->genericAttribute(Application::kShowedHiddenFiles).toBool();

    // Calculate max text width
    int maxTextWidth = 0;
    QFontMetrics fm(font());
    for (int i = 0; i < datas.size(); ++i) {
        auto info = InfoFactory::create<FileInfo>(datas[i].url);
        if (!info.isNull() && (isShowedHiddenFiles || !info->isAttributes(FileInfo::FileIsType::kIsHidden))) {
            QStandardItem *item = new QStandardItem(info->fileIcon(), datas[i].displayText);
            item->setData(i, Qt::UserRole);
            d->folderModel->insertRow(dataNum, item);
            dataNum++;

            // Calculate text width
            int textWidth = fm.horizontalAdvance(datas[i].displayText);
            maxTextWidth = qMax(maxTextWidth, textWidth);
        }
    }

    // Set fixed width considering icon and margins
    int width = maxTextWidth + kTextLeftPadding + kItemMargin * 2;
    width = qBound(172, width, 800);
    setFixedWidth(width);

    int folderCount = dataNum > kMaxFolderCount ? kMaxFolderCount : dataNum;
    setFixedHeight(kFolderListItemMargin * 2 + kFolderItemHeight * folderCount);
}

void FolderListWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Up) {
        d->selectUp();
    } else if (event->key() == Qt::Key_Down) {
        d->selectDown();
    } else if (event->key() == Qt::Key_Return) {
        d->returnPressed();
    }
    DBlurEffectWidget::keyPressEvent(event);
}

void FolderListWidget::hideEvent(QHideEvent *event)
{
    emit hioceann();
    DBlurEffectWidget::hideEvent(event);
}
