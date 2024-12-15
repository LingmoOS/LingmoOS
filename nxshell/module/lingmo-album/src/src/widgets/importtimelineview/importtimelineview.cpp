// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "importtimelineview.h"
#include "unionimage/baseutils.h"
#include "widgets/widgtes/noresultwidget.h"

#include <QScrollBar>
#include <QScroller>
#include <QMimeData>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include <QStackedWidget>

#include <DPushButton>
#include <DTableView>
#include <DCheckBox>
#include <DHiDPIHelper>
#include <DPaletteHelper>

const int MAINWINDOW_NEEDCUT_WIDTH = 775;
const int LISTVIEW_MINMUN_WIDTH = 520;
const int IMPORTTITLE_FIX_WIDTH = 90;
const int STATUSBAR_HEIGHT = 27;
const int TIMELINE_TITLEHEIGHT = 36;
const int SUSPENSION_WIDGET_HEIGHT = 87;//悬浮控件高度

ImportTimeLineView::ImportTimeLineView(QmlWidget *parent)
    : m_mainLayout(nullptr)
    , m_oe(nullptr), m_oet(nullptr), m_ctrlPress(false)
{
    m_qquickContainer = parent;
    //setAcceptDrops(true);
    QVBoxLayout *pMainBoxLayout = new QVBoxLayout(this);
    pMainBoxLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(pMainBoxLayout);

    m_timeLineViewWidget = new QWidget(this);
    pMainBoxLayout->addWidget(m_timeLineViewWidget);

    m_oe = new QGraphicsOpacityEffect(this);
    m_oet = new QGraphicsOpacityEffect(this);
    m_oe->setOpacity(0.5);
    m_oet->setOpacity(0.75);

    initTimeLineViewWidget();
    initConnections();

    themeChangeSlot(DGuiApplicationHelper::instance()->themeType());
}

void ImportTimeLineView::initConnections()
{
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &ImportTimeLineView::themeChangeSlot);
    // 字体改变时,不同尺寸下同步调整标题栏区域控件显示大小
    connect(qApp, &QGuiApplication::fontChanged, this, &ImportTimeLineView::updateSize);
}

void ImportTimeLineView::updateDateNumLabel()
{
    auto fullStr = dateFullStr + " " + numFullStr;
    m_dateNumCheckBox->setText(fullStr);
    m_dateNumLabel->setText(fullStr);
}

void ImportTimeLineView::themeChangeSlot(DGuiApplicationHelper::ColorType themeType)
{
    DPalette pa1 = DPaletteHelper::instance()->palette(m_timeLineViewWidget);
    pa1.setBrush(DPalette::Base, pa1.color(DPalette::Window));
    m_timeLineViewWidget->setPalette(pa1);

    m_importTitleItem->setForegroundRole(DPalette::Window);
    m_importTitleItem->setPalette(pa1);

    DPalette pa = DPaletteHelper::instance()->palette(m_importLabel);
    pa.setBrush(DPalette::Text, themeType == DGuiApplicationHelper::LightType ? lightTextColor : darkTextColor);
    m_importLabel->setForegroundRole(DPalette::Text);
    m_importLabel->setPalette(pa);

    DPalette pal1 = DPaletteHelper::instance()->palette(m_dateNumCheckBox);
    if (themeType == DGuiApplicationHelper::LightType) {
        pal1.setBrush(DPalette::Text, lightTextColor);
        m_dateNumCheckBox->setForegroundRole(DPalette::Text);
        m_dateNumCheckBox->setPalette(pal1);
        m_dateNumLabel->setForegroundRole(DPalette::Text);
        m_dateNumLabel->setPalette(pal1);
    } else if (themeType == DGuiApplicationHelper::DarkType) {
        pal1.setBrush(DPalette::Text, darkTextColor);
        m_dateNumCheckBox->setForegroundRole(DPalette::Text);
        m_dateNumCheckBox->setPalette(pal1);
        m_dateNumLabel->setForegroundRole(DPalette::Text);
        m_dateNumLabel->setPalette(pal1);
    }
}

ThumbnailListView *ImportTimeLineView::getListView()
{
    return m_importTimeLineListView;
}

void ImportTimeLineView::updateSize()
{
    m_importTitleItem->setGeometry(0, 0, width() - 15, SUSPENSION_WIDGET_HEIGHT);
}

void ImportTimeLineView::onCheckBoxClicked()
{
    bool isSelect = m_dateNumCheckBox->isChecked();
    //选中当前时间内的所有图片
    m_importTimeLineListView->timeLimeFloatBtnClicked(dateFullStr, isSelect);
}

void ImportTimeLineView::slotNoPicOrNoVideo(bool isNoResult)
{
    m_noResultWidget->setVisible(isNoResult);
    m_importTimeLineListView->setVisible(!isNoResult);

    if (isNoResult) {
        m_importLabel->setText("");
        m_dateNumCheckBox->setText("");
        m_dateNumLabel->setText("");
    }
}

void ImportTimeLineView::sltCurrentFilterChanged(ExpansionPanel::FilteData &data)
{
    int filterType = Types::All;
    if (data.type == ItemType::ItemTypeNull) {
        //显示全部
        m_importTimeLineListView->showAppointTypeItem(ItemType::ItemTypeNull);
        filterType = Types::All;
    } else if (data.type == ItemType::ItemTypePic) {
        //显示图片
        m_importTimeLineListView->showAppointTypeItem(ItemType::ItemTypePic);
        filterType = Types::Picture;
    } else if (data.type == ItemType::ItemTypeVideo) {
        //显示视频
        m_importTimeLineListView->showAppointTypeItem(ItemType::ItemTypeVideo);
        filterType = Types::Video;
    }
    if (m_qquickContainer) {
        m_qquickContainer->setFilterType(filterType);
    }
    clearAllSelection();
    //如果过滤会后数量<=0，则不可用
    m_ToolButton->setEnabled(m_importTimeLineListView->getAppointTypeItemCount(m_ToolButton->getFilteType()) > 0);
    m_importTimeLineListView->setFocus();
}

QStringList ImportTimeLineView::selectPaths()
{
    QStringList paths;
    paths << m_importTimeLineListView->selectedPaths();
    return paths;
}

void ImportTimeLineView::initTimeLineViewWidget()
{
    m_mainLayout = new QVBoxLayout();
    //左侧距离分界线20px，缩略图spacing+缩略图边框
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_timeLineViewWidget->setLayout(m_mainLayout);

    m_importTimeLineListView = new ThumbnailListView(ThumbnailDelegate::AlbumViewImportTimeLineViewType, -1, COMMON_STR_RECENT_IMPORTED, m_timeLineViewWidget);
    m_importTimeLineListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_importTimeLineListView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_importTimeLineListView->setContentsMargins(0, 0, 0, 0);
    m_importTimeLineListView->setFrameShape(DTableView::NoFrame);
    m_importTimeLineListView->setFocusPolicy(Qt::NoFocus);
    m_importTimeLineListView->m_imageType = COMMON_STR_RECENT_IMPORTED;
    m_importTimeLineListView->m_currentUID = -1;
    m_mainLayout->addWidget(m_importTimeLineListView);

    //初始化筛选无结果窗口
    m_noResultWidget = new NoResultWidget(this);
    m_noResultWidget->setVisible(false);
    m_mainLayout->addWidget(m_noResultWidget);

    //滑动列表，刷新上方悬浮标题
    connect(m_importTimeLineListView, &ThumbnailListView::sigTimeLineDataAndNum, this, &ImportTimeLineView::slotTimeLineDataAndNum);
    //筛选显示，当先列表中内容为无结果
    connect(m_importTimeLineListView, &ThumbnailListView::sigNoPicOrNoVideo, this, &ImportTimeLineView::slotNoPicOrNoVideo);

    //添加悬浮title
    m_importTitleItem = new DWidget(m_timeLineViewWidget);
    m_importTitleItem->setFocusPolicy(Qt::NoFocus);
    QVBoxLayout *titleViewLayout = new QVBoxLayout();
    titleViewLayout->setContentsMargins(18, 10, 0, 0);
    m_importTitleItem->setLayout(titleViewLayout);

    //已导入
    QHBoxLayout *hImportLayout = new QHBoxLayout();
    m_importLabel = new DLabel();
    m_importLabel->setText(tr("Import"));
    hImportLayout->addWidget(m_importLabel);
    DFontSizeManager::instance()->bind(m_importLabel, DFontSizeManager::T3, QFont::Normal);
    QFont ft3 = DFontSizeManager::instance()->get(DFontSizeManager::T3);
    ft3.setFamily("Noto Sans CJK SC");

    //bug76892藏语占用更大高度
    if (QLocale::system().language() == QLocale::Tibetan) {
        m_importLabel->setFixedHeight(TIMELINE_TITLEHEIGHT + 25);
    } else {
        m_importLabel->setFixedHeight(TIMELINE_TITLEHEIGHT);
    }
    m_importLabel->setFont(ft3);

    hImportLayout->addStretch(1);
    hImportLayout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    hImportLayout->setContentsMargins(0, 0, 19, 0);

    // 已导入下的时间和计数
    QHBoxLayout *hDateNumLayout = new QHBoxLayout();
    m_dateNumCheckBox = new DCheckBox();
    connect(m_dateNumCheckBox, &DCheckBox::clicked, this, &ImportTimeLineView::onCheckBoxClicked);
    hDateNumLayout->addWidget(m_dateNumCheckBox);
    DFontSizeManager::instance()->bind(m_dateNumCheckBox, DFontSizeManager::T6, QFont::Normal);
    QFont ft6 = DFontSizeManager::instance()->get(DFontSizeManager::T6);
    ft6.setFamily("Noto Sans CJK SC");

    m_dateNumCheckBox->setFixedHeight(TIMELINE_TITLEHEIGHT);
    m_dateNumCheckBox->setFont(ft6);

    m_dateNumLabel = new DLabel();
    m_dateNumLabel->setFixedHeight(TIMELINE_TITLEHEIGHT);
    DFontSizeManager::instance()->bind(m_dateNumLabel, DFontSizeManager::T6, QFont::Medium);
    m_dateNumLabel->setFont(ft6);
    hDateNumLayout->addWidget(m_dateNumLabel);

    connect(m_importTimeLineListView, &ThumbnailListView::sigShowCheckBox, this, &ImportTimeLineView::onShowCheckBox);
    onShowCheckBox(false);

    initDropDown();

    hDateNumLayout->addStretch(100);
    hDateNumLayout->addWidget(m_ToolButton);

    titleViewLayout->addLayout(hImportLayout);
    titleViewLayout->addLayout(hDateNumLayout);
    titleViewLayout->addStretch(100);

    DPalette ppal_light = DPaletteHelper::instance()->palette(m_importTitleItem);
    ppal_light.setBrush(DPalette::Window, ppal_light.color(DPalette::Base));
    QGraphicsOpacityEffect *opacityEffect_light = new QGraphicsOpacityEffect;
    opacityEffect_light->setOpacity(0.95);
    m_importTitleItem->setPalette(ppal_light);
    m_importTitleItem->setGraphicsEffect(opacityEffect_light);
    m_importTitleItem->setAutoFillBackground(true);
    m_importTitleItem->setContentsMargins(0, 0, 0, 0);
    m_importTitleItem->setGeometry(0, 0, this->width() - 15, SUSPENSION_WIDGET_HEIGHT);
}

void ImportTimeLineView::onShowCheckBox(bool bShow)
{
    m_dateNumCheckBox->setVisible(bShow);
    m_dateNumLabel->setVisible(!bShow);
}

void ImportTimeLineView::clearAndStartLayout()
{
    //由于绘制需要使用listview的宽度，但是加载的时候listview还没有显示出来，宽度是不对的，所以在显示出来后用信号通知加载，记载完成后断开信号，
    //后面的listview就有了正确的宽度，该信号槽就不需要再连接
    qDebug() << "------" << __FUNCTION__ << "";

    //获取所有时间线
    m_timelines = DBManager::instance()->getImportTimelines();
    qDebug() << __func__ << m_timelines.size();

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

    updateDateNumLabel();
}

void ImportTimeLineView::addTimelineLayout()
{
    m_importTimeLineListView->clearSelection();
    m_importTimeLineListView->clearAll();
    DBImgInfoList importList;

    for (int timelineIndex = 0; timelineIndex < m_timelines.size(); timelineIndex++) {
        //获取当前时间照片
        DBImgInfoList ImgInfoList = DBManager::instance()->getInfosByImportTimeline(m_timelines.at(timelineIndex));

        //加时间线标题
        QString date, num;
        QStringList dateTimeList = m_timelines.at(timelineIndex).toString("yyyy.MM.dd hh:mm").split(" ");
        QStringList datelist = dateTimeList.at(0).split(".");
        if (datelist.count() > 2) {
            if (dateTimeList.count() == 2) {
                date = QString(QObject::tr("Imported on") + QObject::tr(" %1-%2-%3 %4"))
                       .arg(datelist[0]).arg(datelist[1]).arg(datelist[2]).arg(dateTimeList[1]);
            } else {
                date = QString(QObject::tr("Imported on ") + QObject::tr("%1/%2/%3"))
                       .arg(datelist[0]).arg(datelist[1]).arg(datelist[2]);
            }
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
        if (photoCount == 1 && videoCount == 0) {
            num = tr("1 photo");
        } else if (photoCount == 0 && videoCount == 1) {
            num = tr("1 video");
        } else if (photoCount > 1 && videoCount == 0) {
            num = tr("%n photos", "", photoCount);
        } else if (photoCount == 0 && videoCount > 1) {
            num = tr("%n videos", "", videoCount);
        } else if (photoCount > 1 && videoCount > 1) {
            num = tr("%n items", "", (photoCount + videoCount));
        }

        if (timelineIndex == 0) {
            //加空白栏
            dateFullStr = date;
            numFullStr = num;

            DBImgInfo info;
            info.itemType = ItemTypeBlank;
            info.imgWidth = m_importTimeLineListView->width();
            m_importTimeLineListView->m_blankItemHeight = SUSPENSION_WIDGET_HEIGHT;
            info.imgHeight = SUSPENSION_WIDGET_HEIGHT;
            info.date = date;
            info.num = num;
            importList.append(info);
        } else {
            //加已导入时间线标题
            DBImgInfo info;
            info.itemType = ItemTypeImportTimeLineTitle;
            info.imgWidth = this->width();
            info.imgHeight = 40;
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

    m_importTimeLineListView->insertThumbnails(importList);

    // 添加底栏空白区域
    //m_importTimeLineListView->insertBlankOrTitleItem(ItemTypeBlank, "", "", STATUSBAR_HEIGHT);
}

void ImportTimeLineView::initDropDown()
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

    connect(m_ToolButton, &FilterWidget::currentItemChanged, this, &ImportTimeLineView::sltCurrentFilterChanged);
}

void ImportTimeLineView::slotTimeLineDataAndNum(QString data, QString num, QString text)
{
    m_importLabel->setText(tr("Import"));
    if (!data.isEmpty()) {
        dateFullStr = data;
    }
    if (!num.isEmpty()) {
        numFullStr = num;
    }

    updateDateNumLabel();

    m_dateNumCheckBox->setChecked(text != QObject::tr("Select"));
}

void ImportTimeLineView::resizeEvent(QResizeEvent *ev)
{
    Q_UNUSED(ev);
    updateSize();
}

void ImportTimeLineView::showEvent(QShowEvent *ev)
{
    Q_UNUSED(ev);
    updateSize();
}

void ImportTimeLineView::dragEnterEvent(QDragEnterEvent *e)
{
    if (!Libutils::base::checkMimeUrls(e->mimeData()->urls())) {
        return;
    }
    e->setDropAction(Qt::CopyAction);
    e->accept();
}

void ImportTimeLineView::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty()) {
        return;
    }
    //ImageEngineApi::instance()->ImportImagesFromUrlList(urls, nullptr, -1, this);
    event->accept();
}

void ImportTimeLineView::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
}

void ImportTimeLineView::mousePressEvent(QMouseEvent *e)
{
    qDebug() << "鼠标按下：";
    if (!m_ctrlPress && e->button() == Qt::LeftButton) {
        m_importTimeLineListView->clearSelection();
        emit sigUpdatePicNum();
    }
    DWidget::mousePressEvent(e);
    // 焦点移除，需要同步各个选择按钮状态
    m_importTimeLineListView->updatetimeLimeBtnText();
}

void ImportTimeLineView::clearAllSelection()
{
    m_importTimeLineListView->clearSelection();
}

QPaintEngine *ImportTimeLineView::paintEngine() const
{
    return nullptr;
}

void ImportTimeLineView::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    if (m_qquickContainer)
        m_qquickContainer->update(event->rect());
}
