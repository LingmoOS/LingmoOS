// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file 此文件中实现播放窗口工具栏相关内容
 */
#ifndef _DMR_TOOLBOX_PROXY_H
#define _DMR_TOOLBOX_PROXY_H

#include <DPlatformWindowHandle>
//#include <QtWidgets>
#include <DWidget>
#include <QFrame>
#include <QHBoxLayout>
#include <QTimer>
#include <QMutex>
#include <DIconButton>
#include <DButtonBox>
#include <DBlurEffectWidget>
#include <DFrame>
#include <DLabel>
#include <DFloatingWidget>
#include <QPainterPath>

#include "dguiapplicationhelper.h"
#include "videoboxbutton.h"
#include "toolbutton.h"
#include "playlist_widget.h"
#include "thumbnail_worker.h"
#include "slider.h"
#include "volumeslider.h"
#include "mircastwidget.h"

namespace Dtk {
namespace Widget {
class DImageButton;
}
}

DWIDGET_USE_NAMESPACE

namespace dmr {
class PlayerEngine;
class VolumeButton;
class ToolButton;
class MainWindow;
class DMRSlider;
class ThumbnailPreview;
class SliderTime;
//class SubtitlesView;
class VolumeSlider;
class ViewProgBar;
class viewProgBarLoad;
class PlaylistWidget;
class ImageItem : public DLabel
{
    Q_OBJECT
public:
    /**
     * @brief ImageItem 实现胶片整体的窗口布局
     * @param image 胶片
     * @param bIsblack 是否为灰色胶片
     * @param parent 父窗口
     */
    ImageItem(QPixmap image, bool bIsblack = false, QWidget *parent = nullptr): DLabel(parent), m_pixmap(image)
    {
    }
protected:
    /**
     * @brief paintEvent 绘制事件函数
     */
    void paintEvent(QPaintEvent *)
    {
        QPainter painter(this);
        painter.setRenderHints(QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform |
                               QPainter::Antialiasing);

        QSize size(m_pixmap.size());
        QBitmap mask(size);

        QPainter painter1(&mask);
        painter1.setRenderHints(QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform |
                                QPainter::Antialiasing);
        painter1.fillRect(mask.rect(), Qt::white);
        painter1.setBrush(QColor(0, 0, 0));
        painter1.drawRoundedRect(mask.rect(), 5, 5);

        QPixmap image = m_pixmap;
        image.setMask(mask);

        painter.setClipping(true);
        QPainterPath bg0;
        bg0.addRoundedRect(rect(), 5, 5);
        painter.setClipPath(bg0);
        painter.drawPixmap(rect(), image);

        QPen pen;
        pen.setWidth(2);
        if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
            pen.setColor(QColor(0, 0, 0, int(0.1 * 255)));
            painter.setPen(pen);
        } else if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
            pen.setColor(QColor(255, 255, 255, int(0.1 * 255)));
            painter.setPen(pen);
        }
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(rect(), 5, 5);
    }
private:
    QPixmap m_pixmap;    ///胶片的图像
};

/**
 * @brief The IndicatorItem class
 * 实现胶片模式时，播放进度光标显示
 */
class IndicatorItem : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief IndicatorItem 构造函数
     * @param parent 父窗口
     */
    explicit IndicatorItem(QWidget *parent = nullptr): QWidget(parent)
    {
        initMember();
    }
    /**
     * @brief setPressed 设置是否按下
     * @param bPressed 按下标志位
     */
    void setPressed(bool bPressed)
    {
        m_bIsPressed = bPressed;
    }

protected:
    /**
     * @brief paintEvent 绘制事件函数
     */
    void paintEvent(QPaintEvent *)
    {
        QPainter painter(this);
        QRect backgroundRect = rect();

        painter.setRenderHints(QPainter::HighQualityAntialiasing |
                               QPainter::SmoothPixmapTransform |
                               QPainter::Antialiasing);

        QPainterPath bpath;

        if (!m_bIsPressed) {
            QPen pen;
            pen.setWidth(1);
            pen.setColor(QColor(0, 0, 0));
            bpath.addRoundedRect(backgroundRect, 3, 3);
            painter.setPen(pen);
            painter.setBrush(Qt::NoBrush);
            painter.setOpacity(0.4);
            painter.fillPath(bpath, QColor(0, 0, 0));

            //改变一下paint的顺序锯齿效果没有那么明显
            QPainterPath bpath1;
            painter.setOpacity(1);
            bpath1.addRoundedRect(backgroundRect.marginsRemoved(QMargins(1, 1, 1, 1)), 3, 3);
            painter.fillPath(bpath1, QColor(255, 255, 255));
        } else {
            painter.fillRect(backgroundRect, QBrush(QColor(255, 138, 0)));
        }
    }

    void initMember()
    {
        m_bIsPressed = false;
    }

private:
    bool m_bIsPressed;    ///鼠标是否按下
};
/**
 * @brief The ToolboxProxy class
 * 实现影院工具栏
 */
class ToolboxProxy: public DFloatingWidget
{
    Q_OBJECT
public:
    /**
     * @brief ToolboxProxy 构造函数
     * @param mainWindow 主窗口
     * @param pPlayerEngine 播放引擎对象指针
     */
    ToolboxProxy(QWidget *mainWindow, PlayerEngine *pPlayerEngine);
    /**
     * @brief ~ToolboxProxy 析构函数
     */
    virtual ~ToolboxProxy() override;
    /**
     * @brief updateTimeInfo 更新工具栏中播放时间显示
     * @param duration 视频总时长
     * @param pos 当前播放的时间点
     * @param pTimeLabel 当前播放时间
     * @param pTimeLabelend 视频总时长
     * @param flag 是否为全屏的控件
     */
    void updateTimeInfo(qint64 duration, qint64 pos, QLabel *pTimeLabel,
                        QLabel *pTimeLabelend, bool flag);
    /**
     * @brief anyPopupShown 是否存在一些弹出显示窗口
     * @return true时为有，false为无
     */
    bool anyPopupShown() const;
    /**
     * @brief closeAnyPopup 关闭所有弹窗效果
     */
    void closeAnyPopup();
    /**
     * @brief setPlaylist 传递播放列表指针
     * @param playlist 播放列表对象指针
     */
    void setPlaylist(PlaylistWidget *pPlaylist);
    /**
     * @brief addpmList 将读取到的缩略图加载到列表中
     * @param pm 缩略图图像
     */
    void addpmList(QList<QPixmap> &pm)
    {
        m_pmList.clear();
        m_pmList.append(pm);
    }
    /**
     * @brief addpmBlackList 将读取到的缩略图加载到灰色列表中
     * @param pm_black 缩略图图像
     */
    void addpmBlackList(QList<QPixmap> &pmBlack)
    {
        m_pmBlackList.clear();
        m_pmBlackList.append(pmBlack);
    }
    /**
     * @brief setVolSliderHide 将音量条控件隐藏
     */
    void setVolSliderHide();
    /**
     * @brief getVolSliderIsHided 获取音量条状态
     * @return 返回音量条的隐藏状态
     */
    bool getVolSliderIsHided();
    /**
     * @brief setButtonTooltipHide 将按键上的悬停显示内容隐藏
     */
    void setButtonTooltipHide();
    /**
     * @brief initToolTip 初始化按键上访提示
     */
    void initToolTip();
    /**
     * @brief getSlider 获取进度条
     * @return 进度条对象
     */
    DMRSlider *getSlider()
    {
        return m_pProgBar;
    }
    /**
     * @brief getViewProBar 获取胶片模式进度
     * @return 返回胶片窗口
     */
    ViewProgBar *getViewProBar()
    {
        return m_pViewProgBar;
    }
    /**
     * @brief getListBtnFocus
     * 获取播放列表按钮焦点状态
     * 用于判断是否为tab键升起
     * @return 焦点状态
     */
    bool getListBtnFocus();
    /**
     * @brief updateProgress 更新播放进度条显示
     * @param nValue 进度条的值
     */
    void updateProgress(int nValue);
    /**
     * @brief updateSlider 根据进度条显示更新影片实际进度
     */
    void updateSlider();
    /**
     * @brief initThumb 初始化加载胶片线程
     */
    void initThumbThread();
    /**
     * @brief updateSliderPoint 非x86平台下更新音量条控件位置
     * @param point 传入主窗口左上角顶点在屏幕的位置
     */
    void updateSliderPoint(QPoint &point);
    /**
     * @brief volumeUp 鼠标滚轮增加音量
     */
    void volumeUp();
    /**
     * @brief volumeDown 鼠标滚轮减少音量
     */
    void volumeDown();
    /**
     * @brief calculationStep 计算鼠标滚轮滚动的步进
     * @param angleDelta 鼠标滚动的距离
     */
    void calculationStep(int iAngleDelta);
    /**
     * @brief changeMuteState 切换静音模式
     */
    void changeMuteState();
    /**
     * @brief playlistClosedByEsc Esc关闭播放列表
     */
    void playlistClosedByEsc();
    /**
     * @brief getMouseTime 获取之前鼠标点击的时间
     * @return 时间
     */
    qint64 getMouseTime();
    /**
     * @brief clearPlayListFocus
     * 清空播放列表中的焦点并将标志位重置
     * esc降下设回焦点
     */
    void clearPlayListFocus();
    /**
     * @brief setBtnFocusSign 设置标志位
     */
    void setBtnFocusSign(bool);

    bool isInMircastWidget(const QPoint &);

    /**
     * @brief updateMircastWidget 更新投屏窗口位置
     * @param p 移动位置点
     */
    void updateMircastWidget(QPoint p);

    void hideMircastWidget();

    MircastWidget *getMircast()
    {
        return m_mircastWidget;
    }

    VolumeSlider *volumeSlider()
    {
        return m_pVolSlider;
    }

    /////add for unit test/////
    DButtonBoxButton *playBtn() {return m_pPlayBtn;}
    DButtonBoxButton *prevBtn() {return m_pPrevBtn;}
    DButtonBoxButton *nextBtn() {return m_pNextBtn;}
    ToolButton *listBtn() {return m_pListBtn;}
    ToolButton *fsBtn() {return m_pFullScreenBtn;}
    VolumeButton *volBtn() {return m_pVolBtn;}
    void setThumbnailmode(bool is_thumbnailmode) {m_bThumbnailmode = is_thumbnailmode;}
    MircastWidget *getMircastWidget() {return m_mircastWidget;}

public slots:
    /**
     * @brief finishLoadSlot 缩略图线程加载完成槽函数
     * @param size 主窗口大小，
     * TODO(xxxpengfei)：此处窗口大小没用，请在1050前去除并梳理逻辑
     */
    void finishLoadSlot(QSize size);
    /**
     * @brief updateplaylisticon 切换主题时更新播放列表图标
     */
    void updateplaylisticon();
    /**
     * @brief setthumbnailmode 设置胶片进度条的模式
     */
    void setthumbnailmode();
    /**
     * @brief updateFullState 更新全屏状态下工具栏状态
     */
    void updateFullState();

    void slotUpdateMircast(int, QString);

signals:
    /**
     * @brief sigVolumeChanged 音量变化返回主窗口信号
     * @param nVolume 变化后的音量值
     */
    void sigVolumeChanged(int &nVolume);
    /**
     * @brief sigMuteStateChanged 静音状态变化后返回主窗口的信号
     * @param bMute 静音状态
     */
    void sigMuteStateChanged(bool &bMute);
    /**
      * @brief 功能不支持信号
      */
    void sigUnsupported();

    void sigMircastState(int, QString);

protected slots:
    /**
     * @brief buttonClicked 处理信号转发器发送的信号
     * @param id 发出信号的对象id
     */
    void buttonClicked(QString id);
    /**
     * @brief buttonEnter 工具栏按钮进入事件槽函数
     */
    void buttonEnter();
    /**
     * @brief buttonLeave 工具栏按钮离开事件槽函数
     */
    void buttonLeave();
    /**
     * @brief updatePlayState 更新不同播放状态下工具栏状态
     */
    void updatePlayState();

    /**
     * @brief updateMovieProgress 更新影片进度条
     */
    void updateMovieProgress();
    /**
     * @brief updateButtonStates
     */
    void updateButtonStates();
    void updateTimeVisible(bool visible);
    /**
     * @brief progressHoverChanged 更新预览图的位置
     * @param v 鼠标悬停的位置
     */
    void progressHoverChanged(int v);
    /**
     * @brief updateHoverPreview 更新悬停时预览缩略图
     * @param url 文件url
     * @param secs 当前时间
     */
    void updateHoverPreview(const QUrl &url, int secs);
    /**
     * @brief waitPlay 等待延时播放
     */
    void waitPlay();
    /**
     * @brief slotThemeTypeChanged 主题变化槽函数
     */
    void slotThemeTypeChanged();
    /**
     * @brief slotLeavePreview 鼠标离开胶片进度条槽函数
     */
    void slotLeavePreview();
    /**
     * @brief slotHidePreviewTime 鼠标离开后隐藏事件控件显示
     */
    void slotHidePreviewTime();
    /**
     * @brief slotSliderPressed 进度条鼠标按下槽函数
     */
    void slotSliderPressed();
    /**
     * @brief slotSliderReleased 进度条鼠标释放槽函数
     */
    void slotSliderReleased();
    /**
     * @brief slotBaseMuteChanged 静音
     * @param sk
     * @param val
     */
    void slotBaseMuteChanged(QString sk, const QVariant &val);
    /**
     * @brief slotVolumeButtonClicked 音量按键单击事件槽函数
     */
    void slotVolumeButtonClicked();
    /**
     * @brief slotFileLoaded 文件加载槽函数
     */
    void slotFileLoaded();
    /**
     * @brief slotElapsedChanged 当前播放时长变化槽函数
     */
    void slotElapsedChanged();
    /**
     * @brief slotApplicationStateChanged 应用状态变化才敢三个月
     * @param e 状态
     */
    void slotApplicationStateChanged(Qt::ApplicationState e);
    /**
     * @brief slotPlayListStateChange 播放列表状态变化槽函数
     */
    void slotPlayListStateChange(bool isShortcut);
    /**
     * @brief slotUpdateThumbnailTimeOut 超时更新胶片
     */
    void slotUpdateThumbnailTimeOut();
    /**
     * @brief slotVolumeChanged 音量变化槽函数
     * @param nVolume 音量值
     */
    void slotVolumeChanged(int nVolume);
    /**
     * @brief slotMuteStateChanged 静音状态变化槽函数
     * @param bMute 静音状态
     */
    void slotMuteStateChanged(bool bMute);

protected:
    /**
     * @brief showEvent 显示事件函数
     * @param event 显示事件
     */
    void showEvent(QShowEvent *event) override;
    /**
     * @brief paintEvent 重绘事件函数
     * @param event 重绘事件
     */
    void paintEvent(QPaintEvent *event) override;
    /**
     * @brief resizeEvent 窗口大小变化事件函数
     * @param event 大小变化事件
     */
    void resizeEvent(QResizeEvent *event) override;
    /**
     * @brief mouseMoveEvent 鼠标移动事件函数
     * @param ev 鼠标移动事件
     */
    void mouseMoveEvent(QMouseEvent *ev) override;
    /**
     * @brief eventFilter 事件过滤器
     * @param obj 事件发出对象
     * @param ev 过滤到的事件
     * @return 返回是否继续执行
     */
    bool eventFilter(QObject *obj, QEvent *ev) override;

private slots:
    void updateMircastTime(int);

private:
    /**
     * @brief setup 初始化工具栏布局
     */
    void setup();
    /**
     * @brief updateToolTipTheme 更新按钮悬浮框主题
     * @param btn 对应的按钮
     */
    void updateToolTipTheme(ToolButton *btn);
    /**
     * @brief updateThumbnail 更新播放列表中的缩略图显示
     */
    void updateThumbnail();
    /**
     * @brief updatePreviewTime 更新胶片模式下鼠标点击时时间框的显示
     * @param secs 当前时间
     * @param pos 当前位置点
     */
    void updatePreviewTime(qint64 secs, const QPoint &pos);
    /**
     * @brief initMember 初始化成员变量
     */
    void initMember();

    MainWindow *m_pMainWindow;          ///主窗口
    PlayerEngine *m_pEngine;            ///播放引擎
    PlaylistWidget *m_pPlaylist;        ///播放列表窗口

    DWidget *m_pProgBarspec;             ///空白进度条窗口
    QWidget *m_pBotSpec;                 ///
    QWidget *m_pBotToolWgt;              ///
    QStackedWidget *m_pProgBar_Widget;   ///
    DBlurEffectWidget *bot_widget;       ///

    QHBoxLayout *_mid;                   ///
    QHBoxLayout *_right;                 ///

    QLabel *m_pTimeLabel;                ///视频当前播放时长控件
    QLabel *m_pTimeLabelend;             ///视频总时长的控件
    VolumeSlider *m_pVolSlider;          ///音量条控件窗口
    ViewProgBar *m_pViewProgBar;         ///胶片模式进度条窗口
    DMRSlider *m_pProgBar;               ///滑动条模式进度条窗口
    ThumbnailPreview *m_pPreviewer;      ///鼠标悬停时进度条预览胶片控件
    SliderTime *m_pPreviewTime;          ///鼠标悬停时进度条预览时间控件
    MircastWidget *m_mircastWidget;      ///投屏选项窗口

    ButtonBoxButton *m_pPlayBtn;        ///播放按钮
    ButtonBoxButton *m_pPrevBtn;        ///上一个按钮
    ButtonBoxButton *m_pNextBtn;        ///下一个按钮
    DButtonBox *m_pPalyBox;              ///按钮组
    VolumeButton *m_pVolBtn;             ///音量按钮
    ToolButton *m_pListBtn;              ///播放列表按钮
    ToolButton *m_pFullScreenBtn;        ///全屏按钮
    ToolButton *m_pMircastBtn;           ///投屏按钮

    //lmh0910DButtonBoxButton替换到ButtonBoxButton
    ButtonToolTip *m_pPlayBtnTip;        ///播放按钮的悬浮提示
    ButtonToolTip *m_pPrevBtnTip;        ///上一个按钮的悬浮提示
    ButtonToolTip *m_pNextBtnTip;        ///下一个按钮的悬浮提示
    ButtonToolTip *m_pFullScreenBtnTip;  ///全屏按钮的悬浮提示
    ButtonToolTip *m_pListBtnTip;        ///播放列表按钮的悬浮提示
    ButtonToolTip *m_pmiracastBtnTip;    ///投屏按钮的悬浮提示

    viewProgBarLoad *m_pWorker;          ///获取胶片的线程
    QPropertyAnimation *m_pPaOpen;       ///工具栏升起动画
    QPropertyAnimation *m_pPaClose;      ///工具栏降下动画

    QList<QPixmap > m_pmList;
    QList<QPixmap > m_pmBlackList;

    QMutex m_listPixmapMutex;       ///缩略图list的锁

    qint64 m_nClickTime;            ///鼠标点击时间

    bool m_bMouseFlag;
    bool m_bMousePree;              ///
    bool m_bThumbnailmode;          ///进度条是否为胶片模式
    bool m_bCanPlay;                ///判断是否能进行曲目切换的标志位
    bool m_bSetListBtnFocus;        ///设置播放列表按钮焦点标志位

    float m_processAdd;
};
/**
 * @brief The viewProgBarLoad class
 * 加载胶片线程
 */
class viewProgBarLoad: public QThread
{
    Q_OBJECT
public:
    /**
     * @brief viewProgBarLoad 构造函数
     * @param engine 播放引擎
     * @param progBar 进度条
     * @param parent 父窗口
     */
    explicit viewProgBarLoad(PlayerEngine *engine = nullptr, DMRSlider *progBar = nullptr, ToolboxProxy *parent = nullptr);

    /**
     * @brief setListPixmapMutex 设置图像表线程锁
     * @param pMutex 锁
     *
     * 必须调用这个函数加锁
     */
    void setListPixmapMutex(QMutex *pMutex);
    /**
     * @brief setListPixmapMutex 设置图像表线程锁
     * 必须调用这个函数加锁
     */
    ~viewProgBarLoad();
public slots:
    /**
     * @brief loadViewProgBar 加载胶片
     * @param size 窗口大小
     */
    void loadViewProgBar(QSize size);
signals:
    /**
     * @brief leaveViewProgBar 离开胶片进度条信号
     */
    void leaveViewProgBar();
    /**
     * @brief hoverChanged 悬停位置改变
     */
    void hoverChanged(int);
    /**
     * @brief sliderMoved 进度条移动信号
     */
    void sliderMoved(int);
    /**
     * @brief sigFinishiLoad 胶片模式加载完成信号
     * @param size 窗口尺寸
     */
    void sigFinishiLoad(QSize size);
    /**
     * @brief finished 线程结束信号
     */
//    void finished();

protected:
    void run();
private:
    /**
     * @brief initThumb 动态初始化缩略图获取
     */
    void initThumb();
    /**
     * @brief initMember 初始化成员变量
     */
    void initMember();

    PlayerEngine *m_pEngine;      ///播放引擎
    ToolboxProxy *m_pParent;      ///主窗口
    DMRSlider *m_pProgBar;        ///胶片模式窗口
    QMutex *m_pListPixmapMutex;   ///线程锁
    char *m_seekTime;             ///图像时间

    video_thumbnailer *m_video_thumbnailer = nullptr;
    image_data *m_image_data = nullptr;
    mvideo_thumbnailer m_mvideo_thumbnailer = nullptr;
    mvideo_thumbnailer_destroy m_mvideo_thumbnailer_destroy = nullptr;
    mvideo_thumbnailer_create_image_data m_mvideo_thumbnailer_create_image_data = nullptr;
    mvideo_thumbnailer_destroy_image_data m_mvideo_thumbnailer_destroy_image_data = nullptr;
    mvideo_thumbnailer_generate_thumbnail_to_buffer m_mvideo_thumbnailer_generate_thumbnail_to_buffer = nullptr;

};
}

//HACK: extent area for progress slider
#define TOOLBOX_TOP_EXTENT  10
#define TOOLBOX_SPACE_HEIGHT 314
#define TOOLBOX_HEIGHT  56
#define TOOLBOX_HEIGHT_EXT (TOOLBOX_HEIGHT + TOOLBOX_TOP_EXTENT)
#define TOOLBOX_BUTTON_WIDTH 36
#define TOOLBOX_BUTTON_HEIGHT 50
#define VOLSLIDER_WIDTH 62
#define VOLSLIDER_HEIGHT 205

#endif /* ifndef _DMR_TOOLBOX_PROXY_H */
