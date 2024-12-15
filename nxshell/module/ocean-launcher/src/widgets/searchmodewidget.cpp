// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchmodewidget.h"
#include "calculate_util.h"

#include <DFontSizeManager>

#include <QVBoxLayout>

SearchModeWidget::SearchModeWidget(QWidget *parent)
    : QWidget (parent)
    , m_nativeWidget(new QWidget(this))
    , m_outsideWidget(new QWidget(this))
    , m_emptyWidget(new QWidget(this))
    , m_outsideLabel(new QLabel(m_outsideWidget))
    , m_nativeView(new AppGridView(AppGridView::MainView, this))
    , m_outsideView(new AppGridView(AppGridView::MainView, this))
    , m_nativeModel(new AppsListModel(AppsListModel::Search))
    , m_outsideModel(new AppsListModel(AppsListModel::PluginSearch))
    , m_iconButton(new DIconButton(m_emptyWidget))
    , m_emptyText(new QLabel(m_emptyWidget))
{
    initAppView();
    initTitle();
    initUi();
    initConnection();
}

SearchModeWidget::~SearchModeWidget()
{
}

void SearchModeWidget::initUi()
{
#ifdef QT_DEBUG
    setStyleSheet("QWidget{border: 1px solid red;}");
#endif

    QMargins contentMargin(0, 0, 0, 0);

    // native app
    QVBoxLayout *nativeVLayout = new QVBoxLayout;
    nativeVLayout->setContentsMargins(contentMargin);
    nativeVLayout->setSpacing(10);
    nativeVLayout->addWidget(m_nativeView);

    QHBoxLayout *nativeHLayout = new QHBoxLayout;
    nativeHLayout->setContentsMargins(contentMargin);
    nativeHLayout->setSpacing(0);

    nativeHLayout->addLayout(nativeVLayout);
    nativeHLayout->setStretch(1, 1);

    // app store app
    QVBoxLayout *outsideVLayout = new QVBoxLayout;
    outsideVLayout->setContentsMargins(contentMargin);
    outsideVLayout->setSpacing(10);
    outsideVLayout->addWidget(m_outsideLabel);
    outsideVLayout->addWidget(m_outsideView);

    QHBoxLayout *outsideHLayout = new QHBoxLayout;
    outsideHLayout->setContentsMargins(contentMargin);
    outsideHLayout->setSpacing(0);

    outsideHLayout->addLayout(outsideVLayout);
    outsideHLayout->setStretch(1, 1);

    // 搜索为空时
    QVBoxLayout *emptyVLayout  = new QVBoxLayout;
    emptyVLayout->setContentsMargins(contentMargin);
    emptyVLayout->setSpacing(0);
    emptyVLayout->addStretch();
    emptyVLayout->addWidget(m_iconButton);
    emptyVLayout->addWidget(m_emptyText);
    emptyVLayout->addStretch();

    m_nativeWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_outsideWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_emptyWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 把他们放在一起,方便处理显示和隐藏
    m_nativeWidget->setLayout(nativeHLayout);
    m_outsideWidget->setLayout(outsideHLayout);
    m_emptyWidget->setLayout(emptyVLayout);

    QVBoxLayout *mainVLayout = new QVBoxLayout;
    mainVLayout->setContentsMargins(QMargins(0, 10, 0, 0));
    mainVLayout->addWidget(m_nativeWidget);
    mainVLayout->addWidget(m_outsideWidget);
    mainVLayout->addWidget(m_emptyWidget, 0, Qt::AlignCenter);
    setLayout(mainVLayout);

    m_emptyWidget->setVisible(false);
}

void SearchModeWidget::initTitle()
{
    QPalette pal = QPalette();
    if (CalculateUtil::instance()->fullscreen())
        pal.setColor(QPalette::WindowText, Qt::white);
    else
        pal.setColor(QPalette::WindowText, Qt::black);

    m_outsideLabel->setPalette(pal);

    // 搜索标题字体样式设置
    QFont searchTitleFont = m_outsideLabel->font();
    searchTitleFont.setBold(true);

    m_outsideLabel->setFont(searchTitleFont);

    // 搜索为空格的字体样式设置
    QFont noResultTitleFont = m_emptyText->font();
    noResultTitleFont.setBold(true);

    m_emptyText->setFont(noResultTitleFont);
    m_emptyText->setWindowOpacity(0.3);

    const qreal ratio = qApp->devicePixelRatio();
    m_iconButton->setIcon(DDciIcon::fromTheme("search_no_result"));
    m_iconButton->setFocusPolicy(Qt::NoFocus);
    m_iconButton->setIconSize(QSize(128, 128) / ratio);
    m_iconButton->setEnabled(false);
    m_iconButton->setFlat(true);

    m_emptyText->setText(tr("No search results"));
}

void SearchModeWidget::initAppView()
{
    QPalette outsidePal = m_outsideLabel->palette();
    outsidePal.setColor(QPalette::WindowText,Qt::white);
    m_outsideLabel->setPalette(outsidePal);

    AppItemDelegate *itemDelegate = new AppItemDelegate(this);

    m_nativeView->setFlow(QListView::LeftToRight);
    m_nativeView->setWrapping(true);
    m_nativeView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_nativeView->setSizeAdjustPolicy(QListView::AdjustToContents);
    m_nativeView->setModel(m_nativeModel);
    m_nativeView->setItemDelegate(itemDelegate);
    m_nativeView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_outsideView->setFlow(QListView::LeftToRight);
    m_outsideView->setWrapping(true);
    m_outsideView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_outsideView->setSizeAdjustPolicy(QListView::AdjustToContents);
    m_outsideView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_outsideView->setModel(m_outsideModel);
    m_outsideView->setItemDelegate(itemDelegate);
}

void SearchModeWidget::initConnection()
{
    connect(CalculateUtil::instance(), &CalculateUtil::layoutChanged, this, &SearchModeWidget::onLayoutChanged);
    QMetaObject::invokeMethod(this, "connectViewEvent", Qt::QueuedConnection, Q_ARG(AppGridView *, m_nativeView));
    QMetaObject::invokeMethod(this, "connectViewEvent", Qt::QueuedConnection, Q_ARG(AppGridView *, m_outsideView));
}

void SearchModeWidget::setItemDelegate(AppItemDelegate *delegate)
{
    m_nativeView->setItemDelegate(delegate);
    m_outsideView->setItemDelegate(delegate);
}

void SearchModeWidget::setSearchModel(QSortFilterProxyModel *model)
{
    updateTitleContent();
    updateTitlePos(CalculateUtil::instance()->fullscreen());

    m_nativeView->setModel(model);
    m_nativeWidget->setVisible(model->rowCount(QModelIndex()) > 0);
    m_outsideWidget->setVisible(m_outsideModel->rowCount(QModelIndex()) > 0);
    m_emptyWidget->setVisible(model->rowCount(QModelIndex()) <= 0);
}

void SearchModeWidget::updateTitleContent()
{
    ItemInfoList_v1 list = AppsManager::instance()->appsInfoList(AppsListModel::PluginSearch);
    ItemInfo_v1 info;
    if (list.size() > 0)
        info = list.at(0);

    m_outsideLabel->setText(tr("Apps found in App Store"));
}

void SearchModeWidget::updateTitlePos(bool alignCenter)
{
    if (alignCenter) {
        DFontSizeManager::instance()->bind(m_outsideLabel, DFontSizeManager::T2);
        m_outsideLabel->setAlignment(Qt::AlignCenter);
    } else {
        DFontSizeManager::instance()->bind(m_outsideLabel, DFontSizeManager::T5);
        m_outsideLabel->setAlignment(Qt::AlignLeft);
    }
}

void SearchModeWidget::addSpacerItem(QBoxLayout *layout)
{
    if (CalculateUtil::instance()->fullscreen())
        layout->addSpacerItem(new QSpacerItem(300, 5, QSizePolicy::Expanding, QSizePolicy::Expanding));
    else
        layout->addSpacerItem(new QSpacerItem(5, 5, QSizePolicy::Preferred, QSizePolicy::Expanding));
}

void SearchModeWidget::selectFirstItem() const
{
    if (m_nativeModel->rowCount(QModelIndex()) <= 0) {
        qDebug() << "model is null";
        return;
    }

    AppItemDelegate *itemDelegate = qobject_cast<AppItemDelegate *>(m_nativeView->itemDelegate());
    if (!itemDelegate) {
        qDebug() << "itemdelegate is null";
        return;
    }

    const QModelIndex targetIndex = m_nativeView->model()->index(0, 0);
    itemDelegate->setCurrentIndex(targetIndex);
    m_nativeView->update();
}

AppGridView *SearchModeWidget::getNativeView() const
{
    return m_nativeView;
}

void SearchModeWidget::onLayoutChanged()
{
    int itemSpacing = CalculateUtil::instance()->appItemSpacing();
    QMargins margin(0, 0, 0, 0);
    m_nativeView->setSpacing(itemSpacing);
    m_nativeView->setViewportMargins(margin);
    m_outsideView->setSpacing(itemSpacing);
    m_outsideView->setViewportMargins(margin);
}
