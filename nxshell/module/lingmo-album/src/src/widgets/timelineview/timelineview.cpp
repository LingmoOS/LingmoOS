// SPDX-FileCopyrightText: 2020 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "timelineview.h"
#include "unionimage/baseutils.h"
#include "unionimage/imageutils.h"
#include "widgets/widgtes/noresultwidget.h"

#include <QScrollBar>
#include <QScroller>
#include <QMimeData>
#include <QGraphicsOpacityEffect>

#include <DPushButton>
#include <DTableView>
#include <DPaletteHelper>
#include <DCheckBox>
#include <DHiDPIHelper>

namespace  {
const int VIEW_IMPORT = 0;
const int VIEW_TIMELINE = 1;
const int VIEW_SEARCH = 2;
const int TITLEHEIGHT = 0;
const int TIMELINE_TITLEHEIGHT = 36;
const int SUSPENSION_WIDGET_HEIGHT = 87;//悬浮控件高度
} //namespace

TimeLineView::TimeLineView(QmlWidget *parent)
    : m_mainLayout(nullptr)
    , allnum(0)
    , m_oe(nullptr), m_oet(nullptr)
    , m_timeLineViewWidget(nullptr)
    , m_selPicNum(0)
{
    m_qquickContainer = parent;
    //setAcceptDrops(true);
    QVBoxLayout *pMainBoxLayout = new QVBoxLayout(this);
    pMainBoxLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(pMainBoxLayout);

    m_timeLineViewWidget = new DWidget(this);
    pMainBoxLayout->addWidget(m_timeLineViewWidget);
    
    m_oe = new QGraphicsOpacityEffect(this);
    m_oet = new QGraphicsOpacityEffect(this);
    m_oe->setOpacity(0.5);
    m_oet->setOpacity(0.75);

    initTimeLineViewWidget();
    initConnections();

    themeChangeSlot(DGuiApplicationHelper::instance()->themeType());
}

void TimeLineView::initConnections()
{
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &TimeLineView::themeChangeSlot);
}

void TimeLineView::themeChangeSlot(DGuiApplicationHelper::ColorType themeType)
{
    DPalette pa1 = DPaletteHelper::instance()->palette(m_timeLineViewWidget);
    pa1.setBrush(DPalette::Base, pa1.color(DPalette::Window));
    m_timeLineViewWidget->setPalette(pa1);

    m_dateNumItemWidget->setForegroundRole(DPalette::Window);
    m_dateNumItemWidget->setPalette(pa1);

    DPalette pa = DPaletteHelper::instance()->palette(m_dateLabel);
    pa.setBrush(DPalette::Text, themeType == DGuiApplicationHelper::LightType ? lightTextColor : darkTextColor);
    m_dateLabel->setForegroundRole(DPalette::Text);
    m_dateLabel->setPalette(pa);

    DPalette pal1 = DPaletteHelper::instance()->palette(m_numCheckBox);
    if (themeType == DGuiApplicationHelper::LightType) {
        pal1.setBrush(DPalette::Text, lightTextColor);
        m_numCheckBox->setForegroundRole(DPalette::Text);
        m_numCheckBox->setPalette(pal1);
        m_numLabel->setForegroundRole(DPalette::Text);
        m_numLabel->setPalette(pal1);
    } else if (themeType == DGuiApplicationHelper::DarkType) {
        pal1.setBrush(DPalette::Text, darkTextColor);
        m_numCheckBox->setForegroundRole(DPalette::Text);
        m_numCheckBox->setPalette(pal1);
        m_numLabel->setForegroundRole(DPalette::Text);
        m_numLabel->setPalette(pal1);
    }
}

ThumbnailListView *TimeLineView::getThumbnailListView()
{
    return m_timeLineThumbnailListView;
}

void TimeLineView::clearAllSelection()
{
    m_timeLineThumbnailListView->clearSelection();
}

void TimeLineView::initTimeLineViewWidget()
{
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_timeLineViewWidget->setLayout(m_mainLayout);

    m_timeLineThumbnailListView = new ThumbnailListView(ThumbnailDelegate::TimeLineViewType, -1, "timelineview", m_timeLineViewWidget);
    m_timeLineThumbnailListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_timeLineThumbnailListView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_timeLineThumbnailListView->setContentsMargins(0, 0, 0, 0);
    m_timeLineThumbnailListView->setFrameShape(DTableView::NoFrame);
    m_mainLayout->addWidget(m_timeLineThumbnailListView);

    //初始化筛选无结果窗口
    m_noResultWidget = new NoResultWidget(m_timeLineViewWidget);
    m_noResultWidget->setVisible(false);
    m_mainLayout->addWidget(m_noResultWidget);

    //滑动列表，刷新上方悬浮标题
    connect(m_timeLineThumbnailListView, &ThumbnailListView::sigTimeLineDataAndNum, this, &TimeLineView::slotTimeLineDataAndNum);
    //筛选显示，当先列表中内容为无结果
    connect(m_timeLineThumbnailListView, &ThumbnailListView::sigNoPicOrNoVideo, this, &TimeLineView::slotNoPicOrNoVideo);

    //添加悬浮title
    m_dateNumItemWidget = new DWidget(m_timeLineViewWidget);
    m_dateNumItemWidget->setFocusPolicy(Qt::ClickFocus);
    QVBoxLayout *titleViewLayout = new QVBoxLayout();
    titleViewLayout->setContentsMargins(18, 10, 0, 0);
    m_dateNumItemWidget->setLayout(titleViewLayout);

    //时间线
    QHBoxLayout *hDateLayout = new QHBoxLayout();
    m_dateLabel = new DLabel();
    hDateLayout->addWidget(m_dateLabel);
    DFontSizeManager::instance()->bind(m_dateLabel, DFontSizeManager::T3, QFont::Normal);
    QFont ft3 = DFontSizeManager::instance()->get(DFontSizeManager::T3);
    ft3.setFamily("Noto Sans CJK SC");

    //bug76892藏语占用更大高度
    if (QLocale::system().language() == QLocale::Tibetan) {
        m_dateLabel->setFixedHeight(TIMELINE_TITLEHEIGHT + 25);
    } else {
        m_dateLabel->setFixedHeight(TIMELINE_TITLEHEIGHT);
    }
    m_dateLabel->setFont(ft3);

    hDateLayout->addStretch(1);
    hDateLayout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    hDateLayout->setContentsMargins(0, 0, 19, 0);

    //时间线下的计数
    QHBoxLayout *hNumLayout = new QHBoxLayout();
    m_numCheckBox = new DCheckBox();
    connect(m_numCheckBox, &DCheckBox::clicked, this, &TimeLineView::onCheckBoxClicked);
    hNumLayout->addWidget(m_numCheckBox);
    DFontSizeManager::instance()->bind(m_numCheckBox, DFontSizeManager::T6, QFont::Normal);
    QFont ft6 = DFontSizeManager::instance()->get(DFontSizeManager::T6);
    ft6.setFamily("Noto Sans CJK SC");

    m_numCheckBox->setFixedHeight(TIMELINE_TITLEHEIGHT);
    m_numCheckBox->setFont(ft6);

    m_numLabel = new DLabel();
    m_numLabel->setFixedHeight(TIMELINE_TITLEHEIGHT);
    DFontSizeManager::instance()->bind(m_numLabel, DFontSizeManager::T6, QFont::Normal);
    m_numLabel->setFont(ft6);
    hNumLayout->addWidget(m_numLabel);

    connect(m_timeLineThumbnailListView, &ThumbnailListView::sigShowCheckBox, this, &TimeLineView::onShowCheckBox);
    onShowCheckBox(false);

    initDropDown();

    hNumLayout->addStretch(100);
    hNumLayout->addWidget(m_ToolButton);

    titleViewLayout->addLayout(hDateLayout);
    titleViewLayout->addLayout(hNumLayout);
    titleViewLayout->addStretch(100);

    DPalette ppal_light = DPaletteHelper::instance()->palette(m_dateNumItemWidget);
    ppal_light.setBrush(DPalette::Window, ppal_light.color(DPalette::Base));
    QGraphicsOpacityEffect *opacityEffect_light = new QGraphicsOpacityEffect;
    opacityEffect_light->setOpacity(0.95);
    m_dateNumItemWidget->setPalette(ppal_light);
    m_dateNumItemWidget->setGraphicsEffect(opacityEffect_light);
    m_dateNumItemWidget->setAutoFillBackground(true);
    m_dateNumItemWidget->setContentsMargins(0, 0, 0, 0);
    m_dateNumItemWidget->setGeometry(0, 0, this->width() - 15, SUSPENSION_WIDGET_HEIGHT);
}

void TimeLineView::clearAndStartLayout()
{
    //由于绘制需要使用listview的宽度，但是加载的时候listview还没有显示出来，宽度是不对的，所以在显示出来后用信号通知加载，记载完成后断开信号，
    //后面的listview就有了正确的宽度，该信号槽就不需要再连接
    qDebug() << "------" << __FUNCTION__ << "";
//    m_spinner->hide();
//    m_spinner->stop();
    //获取所有时间线
    m_timelines = DBManager::instance()->getDays();
    addTimelineLayout();

    if (m_qquickContainer) {
        int filterType = m_qquickContainer->filterType();
        ExpansionPanel::FilteData data;
        if (filterType == Types::All)
            data.type = ItemType::ItemTypeNull;
        else if (filterType == Types::Picture)
            data.type = ItemType::ItemTypePic;
        else if (filterType == Types::Video)
            data.type = ItemType::ItemTypeVideo;
        sltCurrentFilterChanged(data);
    }
}

void TimeLineView::slotTimeLineDataAndNum(QString data, QString num, QString text)
{
    if (!data.isEmpty()) {
        m_dateLabel->setText(data);
    }
    if (!num.isEmpty()) {
        m_numCheckBox->setText(num);
        m_numLabel->setText(num);
    }

    m_numCheckBox->setChecked(text != QObject::tr("Select"));
}

void TimeLineView::sltCurrentFilterChanged(ExpansionPanel::FilteData &data)
{
    int filterType = Types::All;
    if (data.type == ItemType::ItemTypeNull) {
        //显示全部
        m_timeLineThumbnailListView->showAppointTypeItem(ItemType::ItemTypeNull);
        filterType = Types::All;
    } else if (data.type == ItemType::ItemTypePic) {
        //显示图片
        m_timeLineThumbnailListView->showAppointTypeItem(ItemType::ItemTypePic);
        filterType = Types::Picture;
    } else if (data.type == ItemType::ItemTypeVideo) {
        //显示视频
        m_timeLineThumbnailListView->showAppointTypeItem(ItemType::ItemTypeVideo);
        filterType = Types::Video;
    }
    if (m_qquickContainer) {
        m_qquickContainer->setFilterType(filterType);
    }
    clearAllSelection();
    //如果过滤会后数量<=0，则不可用
    m_ToolButton->setEnabled(m_timeLineThumbnailListView->getAppointTypeItemCount(m_ToolButton->getFilteType()) > 0);
    m_timeLineThumbnailListView->setFocus();
}

void TimeLineView::onShowCheckBox(bool bShow)
{
    m_numCheckBox->setVisible(bShow);
    m_numLabel->setVisible(!bShow);
}

void TimeLineView::addTimelineLayout()
{
    m_timeLineThumbnailListView->clearSelection();
    m_timeLineThumbnailListView->clearAll();
    DBImgInfoList importList;

    for (int timelineIndex = 0; timelineIndex < m_timelines.size(); timelineIndex++) {
        //获取当前时间照片
        DBImgInfoList ImgInfoList = DBManager::instance()->getInfosByDay(m_timelines.at(timelineIndex));

        //加时间线标题
        QString date;
        QStringList datelist = m_timelines.at(timelineIndex).split("-");
        if (datelist.count() > 2) {
            date = QString(QObject::tr("%1/%2/%3")).arg(datelist[0]).arg(datelist[1]).arg(datelist[2]);
        }
        int photoCount = 0;
        int videoCount = 0;
        for (int i = 0; i < ImgInfoList.size(); i++) {
            if (ImgInfoList.at(i).itemType == ItemTypePic) {
                photoCount++;
            } else if (ImgInfoList.at(i).itemType == ItemTypeVideo) {
                videoCount++;
            }
        }

        QString num;
        if (photoCount == 1 && videoCount == 0) {
            num = tr("1 photo");
        } else if (photoCount == 0 && videoCount == 1) {
            num = tr("1 video");
        } else if (photoCount > 1 && videoCount == 0) {
            num = tr("%n photos", "", photoCount);
        } else if (photoCount == 0 && videoCount > 1) {
            num = tr("%n videos", "", videoCount);
        } else if (photoCount >= 1 && videoCount >= 1) {
            num = tr("%n items", "", (photoCount + videoCount));
        }

        if (timelineIndex == 0) {
            m_dateLabel->setText(date);
            m_numCheckBox->setText(num);
            m_numLabel->setText(num);

            //加空白栏
            DBImgInfo info;
            info.itemType = ItemTypeBlank;
            info.imgWidth = m_timeLineThumbnailListView->width();
            m_timeLineThumbnailListView->m_blankItemHeight = SUSPENSION_WIDGET_HEIGHT;
            info.imgHeight = SUSPENSION_WIDGET_HEIGHT;
            info.date = date;
            info.num = num;
            importList.append(info);
        } else {
            //加时间线标题
            DBImgInfo info;
            info.itemType = ItemTypeTimeLineTitle;
            info.imgWidth = m_timeLineThumbnailListView->width();
            info.imgHeight = 90;
            info.date = date;
            info.num = num;
            importList.append(info);
        }
        //加当前时间下的图片
        for (auto &eachInfo : ImgInfoList) {
            //存入当前所属时间线的日期和照片数量信息
            eachInfo.date = date;
            eachInfo.num = num;
        }
        importList.append(ImgInfoList);
    }

    m_timeLineThumbnailListView->insertThumbnails(importList);

    // 加空白底栏
    //m_timeLineThumbnailListView->insertBlankOrTitleItem(ItemTypeBlank, "", "", /*m_pStatusBar->height()*/25);
}

void TimeLineView::initDropDown()
{
    m_expansionMenu = new ExpansionMenu(this);
    m_ToolButton = m_expansionMenu->mainWidget();
    m_ToolButton->setText(QObject::tr("All"));
    //m_ToolButton->setIcon(DHiDPIHelper::loadNxPixmap(":/icons/lingmo/builtin/icons/light/album_all_16px.svg"));
    m_ToolButton->setIcon(DHiDPIHelper::loadNxPixmap(":/icons/lingmo/builtin/icons/darkalbum_all_16px.svg"));
    ExpansionPanel::FilteData data;

    data.icon_r_light = QIcon::fromTheme("album_all");
    data.icon_r_dark  = QIcon::fromTheme("album_all_hover");
    data.icon_r_path  = "album_all";
    data.text = QObject::tr("All");
    data.type = ItemType::ItemTypeNull;
    m_expansionMenu->setDefaultFilteData(data);
    m_expansionMenu->addNewButton(data);

    data.icon_r_light = QIcon::fromTheme("album_pic");
    data.icon_r_dark  = QIcon::fromTheme("album_pic_hover");
    data.icon_r_path  = "album_pic";
    data.text = QObject::tr("Photos");
    data.type = ItemType::ItemTypePic;
    m_expansionMenu->addNewButton(data);

    data.icon_r_light = QIcon::fromTheme("album_video");
    data.icon_r_dark  = QIcon::fromTheme("album_video_hover");
    data.icon_r_path  = "album_video";
    data.text = QObject::tr("Videos");
    data.type = ItemType::ItemTypeVideo;
    m_expansionMenu->addNewButton(data);

    connect(m_ToolButton, &FilterWidget::currentItemChanged, this, &TimeLineView::sltCurrentFilterChanged);
}

void TimeLineView::on_AddLabel(QString date, QString num)
{
    if ((nullptr != m_dateNumItemWidget)) {
        QList<QLabel *> labelList = m_dateNumItemWidget->findChildren<QLabel *>();
        labelList[0]->setText(date);
        labelList[1]->setText(num);
        m_dateNumItemWidget->setVisible(true);
        m_dateNumItemWidget->move(0, TITLEHEIGHT);
    }
}

void TimeLineView::onCheckBoxClicked()
{
    bool isSelect = m_numCheckBox->isChecked();
    QString date_str = m_dateLabel->text();
    //选中当前时间内的所有图片
    m_timeLineThumbnailListView->timeLimeFloatBtnClicked(date_str, isSelect);
}

void TimeLineView::resizeEvent(QResizeEvent *ev)
{
    Q_UNUSED(ev);
    //m_spinner->move(width() / 2 - 20, (height() - 50) / 2 - 20);
    m_dateNumItemWidget->setGeometry(0, 0, width() - 15, SUSPENSION_WIDGET_HEIGHT);
//    m_pStatusBar->setFixedWidth(this->width());
//    m_pStatusBar->move(0, this->height() - m_pStatusBar->height());
}

void TimeLineView::dragEnterEvent(QDragEnterEvent *e)
{
    if (!Libutils::base::checkMimeUrls(e->mimeData()->urls())) {
        return;
    }
    e->setDropAction(Qt::CopyAction);
    e->accept();
}

void TimeLineView::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty()) {
        return;
    }
    //ImageEngineApi::instance()->ImportImagesFromUrlList(urls, "", -1, this);
    event->accept();
}

void TimeLineView::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
}

void TimeLineView::dragLeaveEvent(QDragLeaveEvent *e)
{
    Q_UNUSED(e);
}

void TimeLineView::mousePressEvent(QMouseEvent *e)
{
    if (QApplication::keyboardModifiers() != Qt::ControlModifier && e->button() == Qt::LeftButton) {
        m_timeLineThumbnailListView->clearSelection();
    }
    DWidget::mousePressEvent(e);
}

void TimeLineView::slotNoPicOrNoVideo(bool isNoResult)
{
    m_noResultWidget->setVisible(isNoResult);
    m_timeLineThumbnailListView->setVisible(!isNoResult);
    if (isNoResult) {
        m_dateLabel->setText("");
        m_numCheckBox->setText("");
        m_numLabel->setText("");
    }
}

QPaintEngine *TimeLineView::paintEngine() const
{
    return nullptr;
}

void TimeLineView::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    if (m_qquickContainer)
        m_qquickContainer->update(event->rect());
}
