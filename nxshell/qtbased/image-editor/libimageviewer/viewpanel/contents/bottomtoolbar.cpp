// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bottomtoolbar.h"

#include <QTimer>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QDebug>
#include <QPainterPath>
#include <QAbstractItemModel>
#include <QtMath>

#include <DLabel>
#include <DImageButton>
#include <DThumbnailProvider>
#include <DApplicationHelper>
#include <QMetaObject>
#include <DSpinner>

#include "imgviewlistview.h"
#include "imgviewwidget.h"
#include "accessibility/ac-desktop-define.h"
#include "unionimage/baseutils.h"
#include "unionimage/imageutils.h"
#include "unionimage/unionimage.h"
#include "service/permissionconfig.h"
#include "service/commonservice.h"
#include "imageengine.h"
#include "image-viewer_global.h"
DWIDGET_USE_NAMESPACE
namespace {
/////////

/////////////
const int LEFT_MARGIN = 10;
const QSize ICON_SIZE = QSize(50, 50);
const int ICON_SPACING = 9;
const int FILENAME_MAX_LENGTH = 600;
const int RIGHT_TITLEBAR_WIDTH = 100;
const QString LOCMAP_SELECTED_DARK = ":/dark/images/58 drak.svg";
const QString LOCMAP_NOT_SELECTED_DARK = ":/dark/images/imagewithbg-dark.svg";
const QString LOCMAP_SELECTED_LIGHT = ":/light/images/58.svg";
const QString LOCMAP_NOT_SELECTED_LIGHT = ":/light/images/imagewithbg.svg";

const int TOOLBAR_MINIMUN_WIDTH = 782;
const int TOOLBAR_JUSTONE_WIDTH_ALBUM = 532;
const int TOOLBAR_JUSTONE_WIDTH_LOCAL = 350;
const int RT_SPACING = 20;
const int TOOLBAR_HEIGHT = 60;

const int TOOLBAR_DVALUE = 114 + 8;
const QColor TOOLBAR_BUTTOM_DARK=QColor("#303030");
const int THUMBNAIL_WIDTH = 32;
const int THUMBNAIL_ADD_WIDTH = 32;
const int THUMBNAIL_LIST_ADJUST = 9;
const int THUMBNAIL_VIEW_DVALUE = 668;
const int MAINWINDOW_MIN_WIDTH = 630;
const int LOAD_LEFT_RIGHT = 25;     //前后加载图片数（动态）

}  // namespace

LibBottomToolbar::LibBottomToolbar(QWidget *parent) : DFloatingWidget(parent)
{
    m_ocrIsExists = Libutils::base::checkCommandExist("deepin-ocr");
    this->setBlurBackgroundEnabled(true);
    initUI();
    initConnection();
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                     this, &LibBottomToolbar::slotThemeChanged);
    slotThemeChanged(DGuiApplicationHelper::instance()->themeType());
}

LibBottomToolbar::~LibBottomToolbar()
{

}

int LibBottomToolbar::getAllFileCount()
{
    if (m_imgListWidget) {
        return m_imgListWidget->getImgCount();
    } else {
        return -1;
    }
}

int LibBottomToolbar::getToolbarWidth()
{
    //默认值，下面会重新计算
    int width = 300;
    if (LibCommonService::instance()->getImgViewerType() == imageViewerSpace::ImgViewerType::ImgViewerTypeLocal
            || LibCommonService::instance()->getImgViewerType() == imageViewerSpace::ImgViewerType::ImgViewerTypeNull) {
        width = 0;
        setButtonVisible(imageViewerSpace::ButtonTypeBack, false);
        setButtonVisible(imageViewerSpace::ButtonTypeCollection, false);
    } else if (LibCommonService::instance()->getImgViewerType() == imageViewerSpace::ImgViewerType::ImgViewerTypeAlbum) {
        //相册
        width = 0;
        setButtonVisible(imageViewerSpace::ButtonTypeBack, true);
        setButtonVisible(imageViewerSpace::ButtonTypeCollection, true);
    }
    //如果相册的按钮存在，增加宽度
    if (m_backButton->isVisible()) {
        width += m_backButton->width() + ICON_SPACING;
    }
    if (m_clBT->isVisible()) {
        width += m_clBT->width() + ICON_SPACING;
    }
    //看图，本地图片
    width += LEFT_RIGHT_MARGIN * 2;//左右边距
    if (m_preButton->isVisible()) {
        width += m_preButton->width() + ICON_SPACING;//上一张宽度加边距
        width += m_nextButton->width() + ICON_SPACING;//上一张宽度加边距
        width += m_spaceWidget->width();//特殊控件宽度
    }
    //增加可控制设置图片宽度
    if (m_adaptImageBtn->isVisible()) {
        width += m_adaptImageBtn->width() + ICON_SPACING;//适应图片
    }
    if (m_adaptScreenBtn->isVisible()) {
        width += m_adaptScreenBtn->width() + ICON_SPACING;//适应屏幕
    }
    if (m_rotateLBtn->isVisible()) {
        width += m_rotateLBtn->width() + ICON_SPACING;//左旋
    }
    if (m_ocrIsExists && m_ocrBtn->isVisible()) {
        width += m_ocrBtn->width() + ICON_SPACING;//OCR
    }
    if (m_rotateRBtn->isVisible()) {
        width += m_rotateRBtn->width() + ICON_SPACING;//右旋
    }
    if (m_trashBtn->isVisible()) {
        width += m_trashBtn->width();//右旋
    }
    if (m_imgListWidget->getImgCount() <= 1) {
        width += 0;
    } else {
        //ITEM_CURRENT_WH存在着数字是60,但实际大小绘制的时候减小为30所以使用ImgViewListView::ITEM_CURRENT_WH / 2
        width += LibImgViewListView::ITEM_CURRENT_WH;
        width += (m_imgListWidget->getImgCount() /*- 1*/) * (LibImgViewListView::ITEM_CURRENT_WH / 2 + LibImgViewListView::ITEM_SPACING);
        width += m_spaceWidget_thumbnailLeft->width();
        width += m_spaceWidget_thumbnailRight->width();
    }

    return width;
}

imageViewerSpace::ItemInfo LibBottomToolbar::getCurrentItemInfo()
{
    return m_imgListWidget->getCurrentImgInfo();
}

void LibBottomToolbar::setCurrentPath(const QString &path)
{
    m_imgListWidget->setCurrentPath(path);
}

QStringList LibBottomToolbar::getAllPath()
{
    return m_imgListWidget->getAllPath();
}

void LibBottomToolbar::setRotateBtnClicked(const bool &bRet)
{
    if (m_rotateLBtn) {
        m_rotateLBtn->setEnabled(bRet);
    }
    if (m_rotateRBtn) {
        m_rotateRBtn->setEnabled(bRet);
    }
}

void LibBottomToolbar::setPictureDoBtnClicked(const bool &bRet)
{
    bool enableCopy = PermissionConfig::instance()->checkAuthFlag(PermissionConfig::EnableCopy);

    if (m_ocrIsExists && m_ocrBtn) {
        m_ocrBtn->setEnabled(bRet && enableCopy);
    }
    if (m_adaptImageBtn) {
        m_adaptImageBtn->setEnabled(bRet);
    }
    if (m_adaptScreenBtn) {
        m_adaptScreenBtn->setEnabled(bRet);
    }
    if (m_rotateLBtn) {
        m_rotateLBtn->setEnabled(bRet);
    }
    if (m_rotateRBtn) {
        m_rotateRBtn->setEnabled(bRet);
    }
}

DIconButton *LibBottomToolbar::getBottomtoolbarButton(imageViewerSpace::ButtonType type)
{
    DIconButton *button = nullptr;
    switch (type) {
    default:
        break;
    case imageViewerSpace::ButtonTypeBack:
        button = m_backButton;
        break;
    case imageViewerSpace::ButtonTypeNext:
        button = m_nextButton;
        break;
    case imageViewerSpace::ButtonTypePre:
        button = m_preButton;
        break;
    case imageViewerSpace::ButtonTypeAdaptImage:
        button = m_adaptImageBtn;
        break;
    case imageViewerSpace::ButtonTypeAdaptScreen:
        button = m_adaptScreenBtn;
        break;
    case imageViewerSpace::ButtonTypeCollection:
        button = m_clBT;
        break;
    case imageViewerSpace::ButtonTypeOcr:
        if (m_ocrIsExists) {
            button = m_ocrBtn;
        }
        break;
    case imageViewerSpace::ButtonTypeRotateLeft:
        button = m_rotateLBtn;
        break;
    case imageViewerSpace::ButtonTypeRotateRight:
        button = m_rotateRBtn;
        break;
    case imageViewerSpace::ButtonTypeTrash:
        button = m_trashBtn;
        break;
    }
    return button;
}

void LibBottomToolbar::disCheckAdaptImageBtn()
{
    m_adaptImageBtn->setChecked(false);
    badaptImageBtnChecked = false;
}
void LibBottomToolbar::disCheckAdaptScreenBtn()
{
    m_adaptScreenBtn->setChecked(false);
    badaptScreenBtnChecked = false;
}

void LibBottomToolbar::checkAdaptImageBtn()
{
    m_adaptImageBtn->setChecked(true);
    badaptImageBtnChecked = true;
}
void LibBottomToolbar::checkAdaptScreenBtn()
{
    m_adaptScreenBtn->setChecked(true);
    badaptScreenBtnChecked = true;
}

void LibBottomToolbar::deleteImage()
{
    //移除正在展示照片
    if (m_imgListWidget) {
        if (m_imgListWidget->getImgCount() == 0)
            return;

        QString path = getCurrentItemInfo().path;

        QFile file(path);
        if (!file.exists()) {
            return;
        }
        //文件是否被删除的判断bool值
        bool iRetRemove = false;
        if (LibCommonService::instance()->getImgViewerType() == imageViewerSpace::ImgViewerTypeLocal) {
            //先删除文件，需要判断文件是否删除，如果删除了，再决定看图软件的显示
            //不再采用默认删除,使用utils里面的删除
            Libutils::base::trashFile(path);
            QFile fileRemove(path);
            if (!fileRemove.exists()) {
                iRetRemove = true;
            }
        } else if (LibCommonService::instance()->getImgViewerType() == imageViewerSpace::ImgViewerTypeAlbum) {
            iRetRemove = true;
        }

        if (iRetRemove) {
            m_imgListWidget->removeCurrent();
            if (m_imgListWidget->getImgCount() == 1) {
                if (m_preButton) {
                    setButtonVisible(imageViewerSpace::ButtonTypePre, false);
                }
                if (m_nextButton) {
                    setButtonVisible(imageViewerSpace::ButtonTypeNext, false);
                }
                if (m_spaceWidget) {
                    m_spaceWidget->setVisible(false);
                }
                if (m_spaceWidget_thumbnailLeft) {
                    m_spaceWidget_thumbnailLeft->setVisible(false);
                }
                if (m_spaceWidget_thumbnailRight) {
                    m_spaceWidget_thumbnailRight->setVisible(false);
                }
                //BUG#93072 图片删到最后要清理一下控件聚焦
                m_trashBtn->clearFocus();
                //当图片不存在的时候,回到初始界面
                if (!QFileInfo(m_imgListWidget->getCurrentImgInfo().path).isFile()) {
                    emit ImageEngine::instance()->sigPicCountIsNull();
                };
            } else if (m_imgListWidget->getImgCount() == 0) {
                emit ImageEngine::instance()->sigPicCountIsNull();
            }
            if (m_imgListWidget->getCurrentCount() >= m_imgListWidget->getImgCount() - 1) {
                m_nextButton->setEnabled(false);
            }
            if (m_imgListWidget->getCurrentCount() == 0) {
                m_preButton->setEnabled(false);
            }

            emit removed();     //删除数据库图片

            m_imgListWidget->moveCenterWidget();

            PermissionConfig::instance()->triggerAction(PermissionConfig::TidDelete, path);
        }
    }

}

void LibBottomToolbar::onBackButtonClicked()
{
    //2020/6/9 DJH 优化退出全屏，不再闪出退出全屏的间隙 31331
    this->setVisible(false);
    this->setVisible(true);
}

void LibBottomToolbar::onAdaptImageBtnClicked()
{
    emit resetTransform(false);
    m_adaptImageBtn->setChecked(true);
    if (!badaptImageBtnChecked) {
        badaptImageBtnChecked = true;
    }
}

void LibBottomToolbar::onAdaptScreenBtnClicked()
{
    emit resetTransform(true);
    m_adaptScreenBtn->setChecked(true);
    if (!badaptScreenBtnChecked) {
        badaptScreenBtnChecked = true;
    }
}

void LibBottomToolbar::onclBTClicked()
{
    if (true == m_bClBTChecked) {
//        DBManager::instance()->removeFromAlbum(COMMON_STR_FAVORITES, QStringList(m_currentpath), AlbumDBType::Favourite);
    } else {
//        DBManager::instance()->insertIntoAlbum(COMMON_STR_FAVORITES, QStringList(m_currentpath), AlbumDBType::Favourite);
//        emit dApp->signalM->insertedIntoAlbum(COMMON_STR_FAVORITES, QStringList(m_currentpath));
    }
}

void LibBottomToolbar::onRotateLBtnClicked()
{
    emit rotateClockwise();
}

void LibBottomToolbar::onRotateRBtnClicked()
{
    emit rotateCounterClockwise();
}

void LibBottomToolbar::onTrashBtnClicked()
{
    //更换删除顺序,相册需要现在显示删除,再删除本体
    QString path;
    if (m_imgListWidget) {
        path = getCurrentItemInfo().path;
    }
    if (path.isEmpty() && m_currentpath.isEmpty()) {
        path = m_currentpath;
    }

    if (LibCommonService::instance()->getImgViewerType() == imageViewerSpace::ImgViewerType::ImgViewerTypeAlbum) {
        // 相册浏览图片-删除按钮逻辑处理
#ifdef DELETE_CONFIRM
        // 新流程，相册会弹出删除确认提示框，点击确认，相册才给imageeditor发送sigDeleteImage信号删除图片
        emit ImageEngine::instance()->sigDel(path);
#else
        // 老流程，直接删除
        deleteImage();
        emit ImageEngine::instance()->sigDel(path);
#endif
    } else {
        //本地图片浏览-删除图片，直接删除
        deleteImage();
        emit ImageEngine::instance()->sigDel(path);
    }
}

void LibBottomToolbar::slotThemeChanged(int type)
{
    QString rStr;
    if (type == 1) {
        QColor maskColor(247, 247, 247);
        maskColor.setAlphaF(0.15);
        m_forwardWidget->setMaskColor(maskColor);

        DPalette pa;
        pa = m_preButton->palette();
        pa.setColor(DPalette::Light, QColor("#FFFFFF"));
        pa.setColor(DPalette::Dark, QColor("#FFFFFF"));
        // 单个按钮边框
        QColor btnframecolor("#000000");
        btnframecolor.setAlphaF(0.00);
        pa.setColor(DPalette::FrameBorder, btnframecolor);
        // 取消阴影
        pa.setColor(DPalette::Shadow, btnframecolor);
        DApplicationHelper::instance()->setPalette(m_backButton, pa);
        DApplicationHelper::instance()->setPalette(m_preButton, pa);
        DApplicationHelper::instance()->setPalette(m_nextButton, pa);
        DApplicationHelper::instance()->setPalette(m_adaptImageBtn, pa);
        DApplicationHelper::instance()->setPalette(m_adaptScreenBtn, pa);
        DApplicationHelper::instance()->setPalette(m_clBT, pa);

        if (m_ocrIsExists) {
            DApplicationHelper::instance()->setPalette(m_ocrBtn, pa);
        }

        DApplicationHelper::instance()->setPalette(m_rotateLBtn, pa);
        DApplicationHelper::instance()->setPalette(m_rotateRBtn, pa);
        DApplicationHelper::instance()->setPalette(m_trashBtn, pa);
    } else {
        QColor maskColor("#202020");
        maskColor.setAlphaF(0.50);
        m_forwardWidget->setMaskColor(maskColor);

        DPalette pa;
        pa = m_preButton->palette();
        pa.setColor(DPalette::Light, TOOLBAR_BUTTOM_DARK);
        pa.setColor(DPalette::Dark, TOOLBAR_BUTTOM_DARK);
        // 单个按钮边框
        QColor btnframecolor(TOOLBAR_BUTTOM_DARK);
        pa.setColor(DPalette::FrameBorder, btnframecolor);
        // 取消阴影
        pa.setColor(DPalette::Shadow, btnframecolor);
        DApplicationHelper::instance()->setPalette(m_backButton, pa);
        DApplicationHelper::instance()->setPalette(m_preButton, pa);
        DApplicationHelper::instance()->setPalette(m_nextButton, pa);
        DApplicationHelper::instance()->setPalette(m_adaptImageBtn, pa);
        DApplicationHelper::instance()->setPalette(m_adaptScreenBtn, pa);
        DApplicationHelper::instance()->setPalette(m_clBT, pa);

        if (m_ocrIsExists) {
            DApplicationHelper::instance()->setPalette(m_ocrBtn, pa);
        }

        DApplicationHelper::instance()->setPalette(m_rotateLBtn, pa);
        DApplicationHelper::instance()->setPalette(m_rotateRBtn, pa);
        DApplicationHelper::instance()->setPalette(m_trashBtn, pa);
    }
}

void LibBottomToolbar::slotOpenImage(int index, QString path)
{
    if (index == 0) {
        m_preButton->setEnabled(false);
    } else {
        m_preButton->setEnabled(true);
    }
    if (index >= m_imgListWidget->getImgCount() - 1) {
        m_nextButton->setEnabled(false);
    } else {
        m_nextButton->setEnabled(true);
    }
//    qDebug() << index << m_imgListWidget->getImgCount();

    //BUG#93143
    QFileInfo info(path);
    PermissionConfig::instance()->setCurrentImagePath(info.absoluteFilePath());
    m_trashBtn->setVisible(!PermissionConfig::instance()->isCurrentIsTargetImage());

    bool isCopyable = PermissionConfig::instance()->checkAuthFlag(PermissionConfig::EnableCopy);
    if (m_ocrIsExists) {
        if (isCopyable) {
            m_ocrBtn->setToolTip(QObject::tr("Extract text"));
        } else {
            m_ocrBtn->setToolTip(QObject::tr("Extract text") + QObject::tr("(Disabled)"));
        } 
    } 

    //左转右转的控制不在这里
    if (!info.isFile() || !info.exists()) {
        m_adaptImageBtn->setEnabled(false);
        m_adaptImageBtn->setChecked(false);
        m_adaptScreenBtn->setEnabled(false);

        m_trashBtn->setEnabled(false);
        if (m_ocrIsExists) {
            m_ocrBtn->setEnabled(false);
        }
    } else {
        m_adaptImageBtn->setEnabled(true);
        m_adaptScreenBtn->setEnabled(true);

        if (!PermissionConfig::instance()->checkAuthFlag(PermissionConfig::EnableEdit)) {
            return;
        }

        m_trashBtn->setEnabled(true);
        if (m_ocrIsExists) {
            m_ocrBtn->setEnabled(isCopyable);
        }
    }
}

void LibBottomToolbar::setIsConnectDel(bool bFlags)
{
    if (bFlags) {
        connect(m_trashBtn, &DIconButton::clicked, this, &LibBottomToolbar::onTrashBtnClicked, Qt::UniqueConnection);
    } else {
        m_trashBtn->disconnect();
    }
}

void LibBottomToolbar::thumbnailMoveCenterWidget()
{
    m_imgListWidget->moveCenterWidget();
}

void LibBottomToolbar::onNextButton()
{
    emit sigRotateSave();
    if (m_rotateLBtn) {
        m_rotateLBtn->setEnabled(false);
    }
    if (m_rotateRBtn) {
        m_rotateRBtn->setEnabled(false);
    }
    if (m_ocrIsExists && m_ocrBtn) {
        m_ocrBtn->setEnabled(false);
    }
    if (m_imgListWidget) {
        m_imgListWidget->openNext();
    }
}

void LibBottomToolbar::onPreButton()
{
    emit sigRotateSave();
    if (m_rotateLBtn) {
        m_rotateLBtn->setEnabled(false);
    }
    if (m_rotateRBtn) {
        m_rotateRBtn->setEnabled(false);
    }
    if (m_ocrIsExists && m_ocrBtn) {
        m_ocrBtn->setEnabled(false);
    }
    if (m_imgListWidget) {
        m_imgListWidget->openPre();
    }
}

void LibBottomToolbar::onThumbnailChanged(QPixmap pix, const QSize &originalSize)
{
    m_imgListWidget->flushCurrentImg(pix, originalSize);
}

void LibBottomToolbar::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    if (LibCommonService::instance()->getImgViewerType() == imageViewerSpace::ImgViewerType::ImgViewerTypeAlbum
            && m_imgListWidget->isVisible()) {
        if ((topLevelWidget()->width() - MAINWINDOW_MIN_WIDTH) < (ICON_SIZE.width() * 2)) {
            m_rotateRBtn->setVisible(false);
            m_rotateLBtn->setVisible(false);
        } else {
            m_rotateRBtn->setVisible(true);
            m_rotateLBtn->setVisible(true);
        }
    }

    emit sigResizeBottom();
    m_imgListWidget->moveCenterWidget();

    // 计算当前展示的Item数量是否变更
    estimatedDisplayCount();
}

void LibBottomToolbar::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);

    if (LibCommonService::instance()->getImgViewerType() == imageViewerSpace::ImgViewerType::ImgViewerTypeAlbum
            && m_imgListWidget->isVisible()) {
        if ((topLevelWidget()->width() - MAINWINDOW_MIN_WIDTH) < (ICON_SIZE.width() * 2)) {
            m_rotateRBtn->setVisible(false);
            m_rotateLBtn->setVisible(false);
        } else {
            m_rotateRBtn->setVisible(true);
            m_rotateLBtn->setVisible(true);
        }
    }

    m_imgListWidget->moveCenterWidget();
}

void LibBottomToolbar::leaveEvent(QEvent *e)
{
    emit sigLeaveBottom();
    return DFloatingWidget::leaveEvent(e);
}

void LibBottomToolbar::setAllFile(QString path, QStringList paths)
{
    //每次打开清空一下缩略图
    m_imgListWidget->clearListView();
    if (paths.size() <= 1) {
        setButtonVisible(imageViewerSpace::ButtonTypePre, false);
        setButtonVisible(imageViewerSpace::ButtonTypeNext, false);
        m_spaceWidget->setVisible(false);
        m_spaceWidget_thumbnailLeft->setVisible(false);
        m_spaceWidget_thumbnailRight->setVisible(false);
    } else {
        setButtonVisible(imageViewerSpace::ButtonTypePre, true);
        setButtonVisible(imageViewerSpace::ButtonTypeNext, true);
        m_spaceWidget->setVisible(true);
        m_spaceWidget_thumbnailLeft->setVisible(true);
        m_spaceWidget_thumbnailRight->setVisible(true);
    }

    QList<imageViewerSpace::ItemInfo> itemInfos;
    for (int i = 0; i < paths.size(); i++) {
        imageViewerSpace::ItemInfo info;
        info.path = paths.at(i);
        itemInfos << info;
    }

    m_imgListWidget->setAllFile(itemInfos, path);
}

void LibBottomToolbar::updateCollectButton()
{
    if (m_currentpath.isEmpty()) {
        return;
    }
//    if (DBManager::instance()->isImgExistInAlbum(COMMON_STR_FAVORITES, m_currentpath, AlbumDBType::Favourite)) {
//        m_clBT->setToolTip(tr("Unfavorite"));
//        m_clBT->setIcon(QIcon::fromTheme("dcc_ccollection"));
//        m_clBT->setIconSize(QSize(36, 36));
//        m_bClBTChecked = true;
//    } else {
//        m_clBT->setToolTip(tr("Favorite"));
//        DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
//        Q_UNUSED(themeType);
//        m_clBT->setIcon(QIcon::fromTheme("dcc_collection_normal"));
//        m_clBT->setIconSize(QSize(36, 36));
//        m_bClBTChecked = false;
//    }
}

void LibBottomToolbar::initUI()
{
    auto backLayout = new QVBoxLayout(this);
    backLayout->setSpacing(0);
    backLayout->setContentsMargins(0, 0, 0, 0);

    m_forwardWidget = new DBlurEffectWidget(this);
    m_forwardWidget->setBlurRectXRadius(18);
    m_forwardWidget->setBlurRectYRadius(18);
    m_forwardWidget->setRadius(30);
    m_forwardWidget->setBlurEnabled(true);
    m_forwardWidget->setMode(DBlurEffectWidget::GaussianBlur);
    QColor maskColor(255, 255, 255, 76);
    m_forwardWidget->setMaskColor(maskColor);
    backLayout->addWidget(m_forwardWidget);

    QHBoxLayout *hb = new QHBoxLayout(m_forwardWidget);
    this->setLayout(hb);
    hb->setContentsMargins(LEFT_RIGHT_MARGIN, 0, LEFT_RIGHT_MARGIN, 0);
    hb->setSpacing(ICON_SPACING);

    //初始化按钮显示状态
    m_btnDisplaySwitch.set();

    //返回，相册使用
    m_backButton = new DIconButton(this);
    m_backButton->setFixedSize(ICON_SIZE);
//    AC_SET_OBJECT_NAME(m_backButton, BottomToolbar_Back_Button);
//    AC_SET_ACCESSIBLE_NAME(m_backButton, BottomToolbar_Back_Button);
    m_backButton->setIcon(QIcon::fromTheme("dcc_back"));
    m_backButton->setIconSize(QSize(36, 36));
    m_backButton->setToolTip(QObject::tr("Back"));
    setButtonVisible(imageViewerSpace::ButtonTypeBack, false);
    hb->addWidget(m_backButton);

    m_spaceWidget = new QWidget(this);
    m_spaceWidget->setFixedSize(ICON_SPACING, ICON_SPACING);
    if (LibCommonService::instance()->getImgViewerType() == imageViewerSpace::ImgViewerType::ImgViewerTypeAlbum) {
        hb->addWidget(m_spaceWidget);
        setButtonVisible(imageViewerSpace::ButtonTypeBack, true);
    }

    //上一张
    m_preButton = new DIconButton(this);
//    AC_SET_OBJECT_NAME(m_preButton, BottomToolbar_Pre_Button);
//    AC_SET_ACCESSIBLE_NAME(m_preButton, BottomToolbar_Pre_Button);
    m_preButton->setFixedSize(ICON_SIZE);
    m_preButton->setIcon(QIcon::fromTheme("dcc_previous"));
    m_preButton->setIconSize(QSize(36, 36));
    m_preButton->setToolTip(QObject::tr("Previous"));
    m_preButton->hide();
    hb->addWidget(m_preButton);

    //下一张
    m_nextButton = new DIconButton(this);
//    AC_SET_OBJECT_NAME(m_nextButton, BottomToolbar_Next_Button);
//    AC_SET_ACCESSIBLE_NAME(m_nextButton, BottomToolbar_Next_Button);
    m_nextButton->setFixedSize(ICON_SIZE);
    m_nextButton->setIcon(QIcon::fromTheme("dcc_next"));
    m_nextButton->setIconSize(QSize(36, 36));
    m_nextButton->setToolTip(QObject::tr("Next"));
    m_nextButton->hide();
    hb->addWidget(m_nextButton);
    if (LibCommonService::instance()->getImgViewerType() == imageViewerSpace::ImgViewerType::ImgViewerTypeLocal) {
        hb->addWidget(m_spaceWidget);
    }

    //适应图片
    m_adaptImageBtn = new DIconButton(this);
//    AC_SET_OBJECT_NAME(m_adaptImageBtn, BottomToolbar_AdaptImg_Button);
//    AC_SET_ACCESSIBLE_NAME(m_adaptImageBtn, BottomToolbar_AdaptImg_Button);
    m_adaptImageBtn->setFixedSize(ICON_SIZE);
    m_adaptImageBtn->setIcon(QIcon::fromTheme("dcc_11"));
    m_adaptImageBtn->setIconSize(QSize(36, 36));
    m_adaptImageBtn->setToolTip(QObject::tr("1:1 Size"));
    m_adaptImageBtn->setCheckable(true);
    hb->addWidget(m_adaptImageBtn);

    //适应屏幕
    m_adaptScreenBtn = new DIconButton(this);
    m_adaptScreenBtn->setFixedSize(ICON_SIZE);
//    AC_SET_OBJECT_NAME(m_adaptScreenBtn, BottomToolbar_AdaptScreen_Button);
//    AC_SET_ACCESSIBLE_NAME(m_adaptScreenBtn, BottomToolbar_AdaptScreen_Button);
    m_adaptScreenBtn->setIcon(QIcon::fromTheme("dcc_fit"));
    m_adaptScreenBtn->setIconSize(QSize(36, 36));
    m_adaptScreenBtn->setToolTip(QObject::tr("Fit to window"));
//    m_adaptScreenBtn->setCheckable(true);
    hb->addWidget(m_adaptScreenBtn);

    //收藏，相册使用
    m_clBT = new DIconButton(this);
    m_clBT->setFixedSize(ICON_SIZE);
//    AC_SET_OBJECT_NAME(m_clBT, BottomToolbar_Collect_Button);
//    AC_SET_ACCESSIBLE_NAME(m_clBT, BottomToolbar_Collect_Button);
    hb->addWidget(m_clBT);

    //ocr
    if (m_ocrIsExists) {
        m_ocrBtn = new DIconButton(this);
        m_ocrBtn->setFixedSize(ICON_SIZE);
        m_ocrBtn->setIcon(QIcon::fromTheme("dcc_ocr"));
        m_ocrBtn->setIconSize(QSize(36, 36));
        m_ocrBtn->setToolTip(QObject::tr("Extract text"));
        hb->addWidget(m_ocrBtn);
    }

    //向左旋转
    m_rotateLBtn = new DIconButton(this);
//    AC_SET_OBJECT_NAME(m_rotateLBtn, BottomToolbar_Rotate_Left_Button);
//    AC_SET_ACCESSIBLE_NAME(m_rotateLBtn, BottomToolbar_Rotate_Left_Button);
    m_rotateLBtn->setFixedSize(ICON_SIZE);
    m_rotateLBtn->setIcon(QIcon::fromTheme("dcc_left"));
    m_rotateLBtn->setIconSize(QSize(36, 36));
    m_rotateLBtn->setToolTip(QObject::tr("Rotate counterclockwise"));
    hb->addWidget(m_rotateLBtn);

    //向右旋转
    m_rotateRBtn = new DIconButton(this);
//    AC_SET_OBJECT_NAME(m_rotateRBtn, BottomToolbar_Rotate_Right_Button);
//    AC_SET_ACCESSIBLE_NAME(m_rotateRBtn, BottomToolbar_Rotate_Right_Button);
    m_rotateRBtn->setFixedSize(ICON_SIZE);
    m_rotateRBtn->setIcon(QIcon::fromTheme("dcc_right"));
    m_rotateRBtn->setIconSize(QSize(36, 36));
    m_rotateRBtn->setToolTip(QObject::tr("Rotate clockwise"));
    hb->addWidget(m_rotateRBtn);

    //增加缩略图左侧空隙
    m_spaceWidget_thumbnailLeft = new QWidget(this);
    m_spaceWidget_thumbnailLeft->setFixedWidth(0);
    hb->addWidget(m_spaceWidget_thumbnailLeft);

    //缩略图列表
    m_imgListWidget = new MyImageListWidget(this);
    hb->addWidget(m_imgListWidget);

    //增加缩略图右侧空隙
    m_spaceWidget_thumbnailRight = new QWidget(this);
    m_spaceWidget_thumbnailRight->setFixedWidth(5);
    hb->addWidget(m_spaceWidget_thumbnailRight);

    //删除
    m_trashBtn = new DIconButton(this);
    m_trashBtn->setFixedSize(ICON_SIZE);
//    AC_SET_OBJECT_NAME(m_trashBtn, BottomToolbar_Trash_Button);
//    AC_SET_ACCESSIBLE_NAME(m_trashBtn, BottomToolbar_Trash_Button);
    m_trashBtn->setIcon(QIcon::fromTheme("dcc_delete"));
    m_trashBtn->setIconSize(QSize(36, 36));
    m_trashBtn->setToolTip(QObject::tr("Delete"));
    hb->addWidget(m_trashBtn);

    if (PermissionConfig::instance()->isValid() && m_imgListWidget) {
        auto authIns = PermissionConfig::instance();

        if (m_ocrBtn) {
            m_ocrBtn->setEnabled(authIns->checkAuthFlag(PermissionConfig::EnableEdit));
        }
        bool isDeletable = authIns->checkAuthFlag(PermissionConfig::EnableDelete);
        m_trashBtn->setVisible(isDeletable);
        m_trashBtn->setEnabled(isDeletable);
    }
}

void LibBottomToolbar::initConnection()
{
    //返回按钮，相册使用
    connect(m_backButton, &DIconButton::clicked, this, &LibBottomToolbar::onBackButtonClicked);
    //前一张
    connect(m_preButton, &DIconButton::clicked, this, &LibBottomToolbar::onPreButton);
    //下一张
    connect(m_nextButton, &DIconButton::clicked, this, &LibBottomToolbar::onNextButton);
    //适应图片
    connect(m_adaptImageBtn, &DIconButton::clicked, this, &LibBottomToolbar::onAdaptImageBtnClicked);
    //适应屏幕
    connect(m_adaptScreenBtn, &DIconButton::clicked, this, &LibBottomToolbar::onAdaptScreenBtnClicked);
    //收藏，相册使用
    connect(m_clBT, &DIconButton::clicked, this, &LibBottomToolbar::onclBTClicked);
    //向左旋转
    connect(m_rotateLBtn, &DIconButton::clicked, this, &LibBottomToolbar::onRotateLBtnClicked);
    //向右旋转
    connect(m_rotateRBtn, &DIconButton::clicked, this, &LibBottomToolbar::onRotateRBtnClicked);
    //缩略图列表，单机打开图片
    connect(m_imgListWidget, &MyImageListWidget::openImg, this, &LibBottomToolbar::openImg, Qt::QueuedConnection);
    connect(m_imgListWidget, &MyImageListWidget::openImg, this, &LibBottomToolbar::slotOpenImage);
    //删除
    connect(m_trashBtn, &DIconButton::clicked, this, &LibBottomToolbar::onTrashBtnClicked);
#ifdef DELETE_CONFIRM
    connect(ImageEngine::instance(), &ImageEngine::sigDeleteImage, this, &LibBottomToolbar::deleteImage);
#endif
    //ocr
    if (m_ocrIsExists) {
        connect(m_ocrBtn, &DIconButton::clicked, this, &LibBottomToolbar::sigOcr);
    }
}

void LibBottomToolbar::setButtonAlawysNotVisible(imageViewerSpace::ButtonType id, bool notVisible)
{
    m_btnDisplaySwitch.set(id, !notVisible);

    if (notVisible) {
        auto btn = getBottomtoolbarButton(id);
        if (btn != nullptr) {
            btn->setVisible(false);
        }
    }
}

void LibBottomToolbar::setButtonVisible(imageViewerSpace::ButtonType id, bool visible)
{
    auto btn = getBottomtoolbarButton(id);
    if (btn != nullptr) {
        if (m_btnDisplaySwitch.test(id)) {
            btn->setVisible(visible);
        } else {
            btn->setVisible(false);
        }
    }
}

/**
   @return 返回当前缩略图栏显示的item计数，计数 = (显示控件宽度 / 显示的 Item 宽度) + 1，
        计数将至少为 1
 */
int LibBottomToolbar::estimatedDisplayCount()
{
    int itemWidth = LibImgViewListView::ITEM_NORMAL_WIDTH + LibImgViewListView::ITEM_SPACING;
    int estimate = ((m_imgListWidget->width() - LibImgViewListView::ITEM_CURRENT_WH) / itemWidth) + 1;
    int curCount = qMax(1, estimate);

    bool growup = curCount > m_estimateDisplayCount;
    if (curCount != m_estimateDisplayCount) {
        m_estimateDisplayCount = curCount;
    }
    if (growup) {
        Q_EMIT displayItemGrowUp(m_estimateDisplayCount);
    }

    return curCount;
}
