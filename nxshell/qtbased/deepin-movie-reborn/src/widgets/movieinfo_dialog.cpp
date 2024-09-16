// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 *@file 这个文件是影片信息窗口相关的类
 */
#include "movieinfo_dialog.h"
#include "mpv_proxy.h"
#include "playlist_model.h"
#include "utils.h"
#include "tip.h"

#include <QScrollArea>
#include <QDebug>

#include <DPushButton>

#include <denhancedwidget.h>

DWIDGET_USE_NAMESPACE

namespace dmr {
/**
 * @brief ElideText 根据字体大小调整字体缩减
 * @param text 全部的文本
 * @param size 控件的大小
 * @param wordWrap 文本包装的类型
 * @param font 字体
 * @param mode 指定省略号的位置
 * @param lineHeight 行高
 * @param lastLineWidth 保存的宽度
 * @return 返回省略后的文本
 */
static QString ElideText(const QString &text, const QSize &size,
                         QTextOption::WrapMode wordWrap, const QFont &font,
                         Qt::TextElideMode mode, int lineHeight, int lastLineWidth)
{
    QTextLayout textLayout(text);
    QString str;
    QFontMetrics fontMetrics(font);

    textLayout.setFont(font);
    const_cast<QTextOption *>(&textLayout.textOption())->setWrapMode(wordWrap);

    textLayout.beginLayout();
    QTextLine line = textLayout.createLine();
    line.setLineWidth(size.width());

    QString tmp_str = nullptr;
    if (fontMetrics.boundingRect(text).width() <= line.width()) {
        tmp_str = text.mid(line.textStart(), line.textLength());
        str = tmp_str;
    } else {
        int height = 0;
        while (line.isValid()) {
            line.setLineWidth(size.width());
            tmp_str = text.mid(line.textStart(), line.textLength());

            if (tmp_str.indexOf('\n'))
                height += lineHeight;

            str += tmp_str;
            line = textLayout.createLine();

            if (line.isValid())
                str.append("\n");
        }
    }

    textLayout.endLayout();

    return str;
}

/**
 * @brief The ToolTipEvent class
 * 鼠标悬停时的tooltip类
 */
class ToolTipEvent: public QObject
{
public:
    /**
     * @brief ToolTipEvent 构造函数
     * @param parent 父窗口
     */
    explicit ToolTipEvent(QObject *parent): QObject(parent) {}

protected:
    /**
     * @brief eventFilter 事件过滤器
     * @param obj 过滤的对象
     * @param event 事件
     * @return 是否继续执行
     */
    bool eventFilter(QObject *obj, QEvent *event)
    {
        switch (event->type()) {
        case QEvent::ToolTip: {
            QHelpEvent *he = static_cast<QHelpEvent *>(event);
            auto tip = obj->property("HintWidget").value<Tip *>();
            auto btn = tip->property("for").value<QWidget *>();
            tip->setText(btn->toolTip());
            tip->show();
            tip->raise();
            tip->adjustSize();
            QPoint pos = he->globalPos() + QPoint{0, 0};
            int nDesktopWidth = qApp->desktop()->availableGeometry(btn).width();
            if (pos.x() + tip->width() > nDesktopWidth) {
                pos.rx() = nDesktopWidth - tip->width();
            }
            pos.ry() = pos.y() - tip->height();
            tip->move(pos);
            return true;
        }

        case QEvent::Leave: {
            auto parent = obj->property("HintWidget").value<Tip *>();
            parent->hide();
            event->ignore();
            break;
        }
        default:
            break;
        }

        return QObject::eventFilter(obj, event);
    }
};
/**
 * @brief The CloseButton class
 * 影片信息窗口关闭按钮
 */
class CloseButton : public DPushButton
{
public:
    /**
     * @brief CloseButton 构造函数
     * @param parent 父窗口
     */
    explicit CloseButton(QWidget *parent) {}
protected:
    /**
     * @brief paintEvent 重载绘制事件函数
     * @param pPaintEvent 绘制事件
     */
    void paintEvent(QPaintEvent *pPaintEvent) override
    {
        QPainter painter(this);
        QRect rect = this->rect();
        if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
            painter.drawPixmap(rect, QPixmap(INFO_CLOSE_LIGHT));
        } else if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
            painter.drawPixmap(rect, QPixmap(INFO_CLOSE_DARK));
        } else {
            painter.drawPixmap(rect, QPixmap(INFO_CLOSE_LIGHT));
        }
    }
};

/**
 * @brief MovieInfoDialog 构造函数
 */
MovieInfoDialog::MovieInfoDialog(const struct PlayItemInfo &pif ,QWidget *parent)
    : DDialog(parent)
{
    initMember();
    if(utils::check_wayland_env()){
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    }

    this->setObjectName(MOVIE_INFO_DIALOG);
    this->setAccessibleName(MOVIE_INFO_DIALOG);
    m_titleList.clear();

    DWidget *content = new DWidget();
    addContent(content);

    const MovieInfo &strMovieInfo = pif.mi;

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    pMainLayout->setContentsMargins(0, 0, 0, 0);
    pMainLayout->setSpacing(0);
    content->setLayout(pMainLayout);

    PosterFrame *pPosterFrame = new PosterFrame(this);
    pPosterFrame->setWindowOpacity(1);
    pPosterFrame->setFixedHeight(128);

    qreal pixelRatio = qApp->devicePixelRatio();
    QPixmap cover;
    if (pif.thumbnail.isNull()) {
        cover = (utils::LoadHiDPIPixmap(LOGO_BIG));
    } else {
        QSize sz(220, 128);
        sz *= pixelRatio;
        auto img = pif.thumbnail.scaledToWidth(sz.width(), Qt::SmoothTransformation);
        cover = img.copy(0, (img.height() - sz.height()) / 2, sz.width(), sz.height());
        cover.setDevicePixelRatio(pixelRatio);
    }
    cover = utils::MakeRoundedPixmap(cover, 8, 8);
    pPosterFrame->setPixmap(cover);
    pPosterFrame->ensurePolished();
    pMainLayout->addWidget(pPosterFrame);
    pMainLayout->setAlignment(pPosterFrame, Qt::AlignHCenter);
    pMainLayout->addSpacing(9);

    m_pFileNameLbl->setMinimumWidth(260);
    DFontSizeManager::instance()->bind(m_pFileNameLbl, DFontSizeManager::T8);
    m_pFileNameLbl->setText(m_pFileNameLbl->fontMetrics().elidedText(QFileInfo(strMovieInfo.filePath).fileName(), Qt::ElideMiddle, 260));
    m_pFileNameLbl->setAlignment(Qt::AlignCenter);
    pMainLayout->addWidget(m_pFileNameLbl);
    pMainLayout->setAlignment(m_pFileNameLbl, Qt::AlignHCenter);
    pMainLayout->addSpacing(50);

    QList<DLabel *> tipLst;
    tipLst.clear();

    m_pScrollArea->setObjectName(MOVIE_INFO_SCROLL_AREA);
    m_pScrollArea->setAccessibleName(MOVIE_INFO_SCROLL_AREA);
    m_pScrollArea->viewport()->setObjectName(SCROLL_AREA_VIEWPORT);
    QPalette palette = m_pScrollArea->viewport()->palette();
    palette.setBrush(QPalette::Background, Qt::NoBrush);
    m_pScrollArea->viewport()->setPalette(palette);
    m_pScrollArea->setFrameShape(QFrame::Shape::NoFrame);
    m_pScrollArea->setWidgetResizable(true);
    pMainLayout->addWidget(m_pScrollArea);

    QWidget *scrollContentWidget = new QWidget(m_pScrollArea);
    scrollContentWidget->setObjectName(MOVIE_INFO_SCROLL_CONTENT);
    QVBoxLayout *scrollWidgetLayout = new QVBoxLayout;
    scrollWidgetLayout->setContentsMargins(0, 0, 0, 0);
    scrollWidgetLayout->setSpacing(10);
    scrollContentWidget->setLayout(scrollWidgetLayout);
    m_pScrollArea->setWidget(scrollContentWidget);
    m_pScrollArea->setWidgetResizable(true);
    m_pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

    //添加基本信息
    ArrowLine *film = new ArrowLine;
    film->setObjectName(FILM_INFO_WIDGET);
    film->setTitle(tr("Film info"));
    //去掉分割线
    film->setSeparatorVisible(false);
    film->setExpandedSeparatorVisible(false);
    InfoBottom *infoRect = new InfoBottom;
    scrollWidgetLayout->addWidget(film);
    scrollWidgetLayout->setAlignment(film, Qt::AlignHCenter);
    film->setContent(infoRect);
    film->setFixedWidth(280);
    infoRect->setFixedWidth(280);
//    infoRect->setMinimumHeight(132);
    film->setExpand(true);
    m_expandGroup.append(film);
    QFormLayout *pFormLayout = new QFormLayout(infoRect);
    pFormLayout->setContentsMargins(10, 5, 20, 16);
    pFormLayout->setVerticalSpacing(6);
    pFormLayout->setHorizontalSpacing(10);
    pFormLayout->setLabelAlignment(Qt::AlignLeft);
    pFormLayout->setFormAlignment(Qt::AlignCenter);
    DEnhancedWidget *hanceedWidget = new DEnhancedWidget(film, this);
    connect(hanceedWidget, &DEnhancedWidget::heightChanged, this, &MovieInfoDialog::changedHeight);

    QString strDuration = strMovieInfo.durationStr();
    if(strDuration == "00:00:00") {
        strDuration = "-";
    }
    QString strVCodecName = strMovieInfo.videoCodec();
    if(strVCodecName == "none" || strVCodecName.isEmpty()) {
        strVCodecName = "-";
    }
    QString strVCodecRate = "-";
    if(strMovieInfo.vCodeRate > 0) {
        strVCodecRate = strMovieInfo.vCodeRate > 1000 ? QString(tr("%1 kbps")).arg(strMovieInfo.vCodeRate / 1000)
                  : QString(tr("%1 bps")).arg(strMovieInfo.vCodeRate);
    }
    QString strFps = strMovieInfo.fps > 0 ? QString(tr("%1 fps")).arg(strMovieInfo.fps) : "-";
    QString strProportion = strMovieInfo.proportion > 0.0f ? QString(tr("%1")).arg(static_cast<double>(strMovieInfo.proportion)) : "-";
    QString strResolution = strMovieInfo.width > 0 ? strMovieInfo.resolution : "-" ;

    QString strACodecName = strMovieInfo.audioCodec();
    if(strACodecName.isEmpty()) {
        strACodecName = "-";
    }
    QString strACodecRate = "-";
    if(strMovieInfo.aCodeRate > 1000) {
        strACodecRate = strMovieInfo.aCodeRate > 1000 ? QString(tr("%1 kbps")).arg(strMovieInfo.aCodeRate / 1000)
                 : QString(tr("%1 bps")).arg(strMovieInfo.aCodeRate);
    }
    QString strName(tr("Name"));
    QString strBits = strMovieInfo.aDigit > 0 ? QString(tr("%1 bits").arg(strMovieInfo.aDigit)) : "-";
    QString strChannels = strMovieInfo.channels > 0 ? QString(tr("%1 channels")).arg(strMovieInfo.channels) : "-";
    QString strSamp = strMovieInfo.sampling > 0 ? QString(tr("%1hz")).arg(strMovieInfo.sampling) : "-";

    addRow(tr("Format"), strMovieInfo.fileType, pFormLayout, tipLst);
    addRow(tr("Size"), strMovieInfo.sizeStr(), pFormLayout, tipLst);
    addRow(tr("Duration"), strDuration, pFormLayout, tipLst);
    DLabel *tmp = new DLabel;
    DFontSizeManager::instance()->bind(tmp, DFontSizeManager::T8);
    tmp->setText(strMovieInfo.filePath);
    QFontMetrics fontMetrics = tmp->fontMetrics();
    addRow(tr("Path"), strMovieInfo.filePath, pFormLayout, tipLst);

    //添加视频信息
    ArrowLine *video = new ArrowLine;
    video->setObjectName(CODEC_INFO_WIDGET);
    video->setTitle(tr("Codec info"));
    video->setSeparatorVisible(false);
    video->setExpandedSeparatorVisible(false);
    InfoBottom *videoRect = new InfoBottom;
    scrollWidgetLayout->addWidget(video);
    scrollWidgetLayout->setAlignment(video, Qt::AlignHCenter);
    video->setContent(videoRect);
    video->setFixedSize(280, 136);
    videoRect->setFixedWidth(280);
    videoRect->setMinimumHeight(136);
    video->setExpand(true);
    m_expandGroup.append(video);
    QFormLayout *pVideoFormLayout = new QFormLayout(videoRect);
    pVideoFormLayout->setContentsMargins(10, 5, 20, 19);
    pVideoFormLayout->setVerticalSpacing(6);
    pVideoFormLayout->setHorizontalSpacing(10);
    pVideoFormLayout->setLabelAlignment(Qt::AlignLeft);
    pVideoFormLayout->setFormAlignment(Qt::AlignCenter);
    DEnhancedWidget *videoWidget = new DEnhancedWidget(video, this);
    connect(videoWidget, &DEnhancedWidget::heightChanged, this, &MovieInfoDialog::changedHeight);

    addRow(tr("Video CodecID"), strVCodecName, pVideoFormLayout, tipLst);
    addRow(tr("Video CodeRate"), strVCodecRate, pVideoFormLayout, tipLst);
    addRow(tr("FPS"), strFps, pVideoFormLayout, tipLst);
    addRow(tr("Proportion"), strProportion, pVideoFormLayout, tipLst);
    addRow(tr("Resolution"), strResolution, pVideoFormLayout, tipLst);

    //添加音频信息
    ArrowLine *audio = new ArrowLine;
    audio->setObjectName(AUDIO_INFO_WIDGET);
    audio->setTitle(tr("Audio info"));
    audio->setSeparatorVisible(false);
    audio->setExpandedSeparatorVisible(false);
    InfoBottom *audioRect = new InfoBottom;
    scrollWidgetLayout->addWidget(audio);
    scrollWidgetLayout->setAlignment(audio, Qt::AlignHCenter);
    audio->setContent(audioRect);
    audio->setFixedWidth(280);
    audioRect->setFixedWidth(280);
    audioRect->setMinimumHeight(136);
    audio->setExpand(true);
    m_expandGroup.append(audio);
    QFormLayout *audioForm = new QFormLayout(audioRect);
    audioForm->setContentsMargins(10, 5, 20, 16);
    audioForm->setVerticalSpacing(6);
    audioForm->setHorizontalSpacing(10);
    audioForm->setLabelAlignment(Qt::AlignLeft);
    audioForm->setFormAlignment(Qt::AlignCenter);
    DEnhancedWidget *audioWidget = new DEnhancedWidget(audio, this);
    connect(audioWidget, &DEnhancedWidget::heightChanged, this, &MovieInfoDialog::changedHeight);

    addRow(tr("Audio CodecID"), strACodecName, audioForm, tipLst);
    addRow(tr("Audio CodeRate"), strACodecRate, audioForm, tipLst);
    addRow(tr("Audio digit"), strBits, audioForm, tipLst);
    addRow(tr("Channels"), strChannels, audioForm, tipLst);
    addRow(tr("Sampling"), strSamp, audioForm, tipLst);

    setFixedSize(300, 642);

    if (!m_titleList.isEmpty()) {
        auto f = m_titleList[10]->fontMetrics();
        auto widget = f.boundingRect(m_titleList[10]->text()).width();
        if (widget > 60) {
            foreach (QLabel *l, m_titleList) {
                l->setFixedWidth(widget + 3);
            }
        }
    }

    auto th = new ToolTipEvent(this);
    if (tipLst.size() > 1) {
        DLabel *pFilePathLbl = tipLst.at(3);
        pFilePathLbl->setObjectName("filePathLabel");
        auto codeLabel = m_titleList.at(5);
        QFontMetrics fm_cl = codeLabel->fontMetrics();
        pFilePathLbl->setMinimumWidth(qMin(160, 250 - fm_cl.boundingRect(codeLabel->text()).width()));
        auto fp = ElideText(tmp->text(), {pFilePathLbl->width(), fm_cl.height()}, QTextOption::WrapAnywhere,
                            pFilePathLbl->font(), Qt::ElideRight, fm_cl.height(), pFilePathLbl->width());
        pFilePathLbl->setText(fp);
        m_pFilePathLbl = pFilePathLbl;
        m_sFilePath = tmp->text();
        pFilePathLbl->setToolTip(tmp->text());
        auto t = new Tip(QPixmap(), tmp->text(), nullptr);
        t->resetSize(QApplication::desktop()->availableGeometry().width());
        t->setProperty("for", QVariant::fromValue<QWidget *>(pFilePathLbl));
        pFilePathLbl->setProperty("HintWidget", QVariant::fromValue<QWidget *>(t));
        pFilePathLbl->installEventFilter(th);
    }

    delete tmp;
    tmp = nullptr;

    connect(qApp, &QGuiApplication::fontChanged, this, &MovieInfoDialog::onFontChanged);
    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this, &MovieInfoDialog::slotThemeTypeChanged);

    m_expandGroup.at(0)->setExpand(true);
    m_expandGroup.at(1)->setExpand(true);
    m_expandGroup.at(2)->setExpand(true);
}
/**
 * @brief paintEvent 重载绘制事件函数
 * @param pPaintEvent 绘制事件
 */
void MovieInfoDialog::paintEvent(QPaintEvent *ev)
{
    QPainter painter(this);
    if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
        painter.fillRect(this->rect(), QColor(255, 255, 255, static_cast<int>(255 * 0.8)));
    } else {
        painter.fillRect(this->rect(), QColor(0, 0, 0, static_cast<int>(255 * 0.8)));
    }

    QDialog::paintEvent(ev);
}

void MovieInfoDialog::showEvent(QShowEvent *pEvent)
{
    moveToCenter();

    return QDialog::showEvent(pEvent);
}
/**
 * @brief onFontChanged 字体变化槽函数（跟随系统变化）
 * @param font 变化后的字体
 */
void MovieInfoDialog::onFontChanged(const QFont &font)
{
    QFontMetrics fm(font);
    QString strFileName = m_pFileNameLbl->fontMetrics().elidedText(QFileInfo(m_sFilePath).fileName(), Qt::ElideMiddle, m_pFileNameLbl->width());
    m_pFileNameLbl->setText(strFileName);

    if (m_pFilePathLbl) {
        QString sFilePath = ElideText(m_sFilePath, {m_pFilePathLbl->width(), fm.height()}, QTextOption::WrapAnywhere,
                            m_pFilePathLbl->font(), Qt::ElideRight, fm.height(), m_pFilePathLbl->width());
        m_pFilePathLbl->setText(sFilePath);
    }
}
/**
 * @brief changedHeight 高度变化槽函数
 */
void MovieInfoDialog::changedHeight(const int height)
{
    QRect rc = geometry();
    int expandsHeight = 30;
    QList<DDrawer *>::const_iterator expand = m_expandGroup.cbegin();

    while (expand != m_expandGroup.cend()) {
        expandsHeight += (*expand)->height();
        ++expand;
    }

    expandsHeight += contentsMargins().top() + contentsMargins().bottom();
    rc.setHeight(expandsHeight + 265);

    setGeometry(rc);
    this->setFixedHeight(qMin(615, expandsHeight + 265));
}
/**
 * @brief slotThemeTypeChanged
 * 主题变化槽函数
 */
void MovieInfoDialog::slotThemeTypeChanged()
{
    m_pFileNameLbl->setForegroundRole(DPalette::BrightText);
}
/**
 * @brief addRow 增加信息条目函数
 * @param title 信息名称
 * @param field 信息内容
 * @param form 布局管理
 * @retval tipList 传回当前添加信息的label控件
 */
void MovieInfoDialog::addRow(QString sTitle, QString sField, QFormLayout *pForm, QList<DLabel *> &tipList)
{
    auto f = new DLabel(sTitle, this);
    f->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    f->setMinimumSize(60, 20);
    DFontSizeManager::instance()->bind(f, DFontSizeManager::T8);
    DPalette pa1 = DApplicationHelper::instance()->palette(f);
    pa1.setBrush(DPalette::Text, pa1.color(DPalette::TextTitle));
    f->setPalette(pa1);

    auto t = new DLabel(sField, this);
    t->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    t->setMinimumHeight(20);
    t->setWordWrap(true);
    DFontSizeManager::instance()->bind(t, DFontSizeManager::T8);
    DPalette pa2 = DApplicationHelper::instance()->palette(t);
    pa2.setBrush(DPalette::Text, pa2.color(DPalette::TextTitle));
    t->setPalette(pa2);
    pForm->addRow(f, t);
    tipList.append(t);
    m_titleList.append(f);
}
/**
 * @brief initMember 初始化成员变量
 */
void MovieInfoDialog::initMember()
{
    m_pFileNameLbl = new DLabel(this);
    m_pFilePathLbl = nullptr;
    m_sFilePath = QString();
    m_pScrollArea = new QScrollArea;
    m_nLastHeight = -1;
}

}
