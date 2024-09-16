// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "utils.h"
#include "settings.h"
#include <QDebug>
#include <QDBusInterface>
#include <QDesktopWidget>
#include <QScreen>
#include <QClipboard>
#include <QFileDialog>

#define MOVENUM 1

#define WHEELNUM 10
namespace {
const int TOOLBAR_Y_SPACING = 12;
}

MainWindow::MainWindow(QWidget *parent)
    : DWidget(parent)
{
    //初始化主窗口
    initMainWindow();
    //计算屏幕大小
    CalculateScreenSize();
    this->setMinimumSize(WHEELNUM, WHEELNUM);
    initShortcut(); // 初始化快捷键
}

MainWindow::~MainWindow()
{
    if (m_toolBar != nullptr) {
        delete m_toolBar;
        m_toolBar = nullptr;
    }
}

void MainWindow::initMainWindow()
{
    m_ocrInterface = nullptr;
    m_toolBar = nullptr;
    //去菜单栏，置顶窗口
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::BypassWindowManagerHint);

    //设置可以进行鼠标操作
    setMouseTracking(true);
    isLeftPressDown = false;
    dir = UP;
    //获取屏幕的缩放比例
    m_pixelRatio = qApp->primaryScreen()->devicePixelRatio();

    m_menuController = new MenuController();
    connect(m_menuController, &MenuController::saveAction, this, &MainWindow::onSave);
    connect(m_menuController, &MenuController::closeAction, this, &MainWindow::onExit);

    // 工具栏设置
    m_toolBar = new ToolBarWidget(); //初始化
    connect(m_toolBar, SIGNAL(sendOcrButtonClicked()), this, SLOT(onOpenOCR()));
    connect(m_toolBar, SIGNAL(sendSaveButtonClicked()), this, SLOT(onSave()));
    connect(m_toolBar, SIGNAL(sendCloseButtonClicked()), this, SLOT(onExit()));

    QWidget::installEventFilter(this);
}

bool MainWindow::openFile(const QString &filePaths)
{
    qDebug() << "func: " << __func__ ;
    QImage image(filePaths);
    openImageAndName(image, "", QPoint(0, 0));
    return true;
}

bool MainWindow::openImage(const QImage &image)
{
    qDebug() << "func: " << __func__ ;
    openImageAndName(image, "", QPoint(0, 0));
    return true;
}

bool MainWindow::openImageAndName(const QImage &image, const QString &name, const QPoint &point)
{
    qDebug() << "func: " << __func__ << name;
    m_image = image;
    m_lastImagePath = name;
    m_imageName = QString(tr("Screenshot") + "_%1").arg(QDateTime::currentDateTime().
                                                        toString("yyyyMMddHHmmss"));
    int width = static_cast<int>(m_image.width() / m_pixelRatio);
    int height = static_cast<int>(m_image.height() / m_pixelRatio);
    //将窗口的大小重置为图片的大小
    resize(width, height);
    //将坐标点转换为m_pixelRatio屏幕缩放比下的点
    m_showPosition = QPoint(static_cast<int>(point.x() * m_pixelRatio), static_cast<int>(point.y() * m_pixelRatio));
    QRect currnetScreenRect;
    //查找当前截图贴图区域所在的屏幕(m_screenInfo中存储已缩放的数据）
    for (int i = 0; i < m_screenInfo.size(); ++i) {
        currnetScreenRect = {
            static_cast<int>(m_screenInfo[i].x),
            static_cast<int>(m_screenInfo[i].y),
            static_cast<int>(m_screenInfo[i].width *  m_pixelRatio),
            static_cast<int>(m_screenInfo[i].height * m_pixelRatio)
        };
        if (currnetScreenRect.contains(m_showPosition)) {
            int x = static_cast<int>((m_showPosition.x() - currnetScreenRect.x()) / m_pixelRatio + currnetScreenRect.x());
            m_showPosition.setX(x);
            int y = static_cast<int>((m_showPosition.y() - currnetScreenRect.y()) / m_pixelRatio + currnetScreenRect.y());
            m_showPosition.setY(y);
            break;
        }
    }
    //移动到指定位置
    move(m_showPosition);
    update();
    proportion = static_cast<double>(this->width())  / this->height();
    m_isfirstTime = true; //解决119157Bug采用的标识
    updateToolBarPosition();// 更新位置
    return true;
}
// 开启OCR
void MainWindow::onOpenOCR()
{
    saveImg();
    // 测试
    if (m_ocrInterface == nullptr)
        m_ocrInterface = new OcrInterface("com.deepin.Ocr", "/com/deepin/Ocr", QDBusConnection::sessionBus(), this);
    m_ocrInterface->openImageAndName(m_image, m_imageName);
}

// 贴图保存
void MainWindow::saveImg()
{
    m_saveInfo = m_toolBar->getSaveInfo(); // 获取保存信息

    if (m_saveInfo.first == SubToolWidget::DESKTOP) {
        qDebug() << "保存到桌面";
        QString savePath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
        QString formatStr;
        if (m_saveInfo.second == SubToolWidget::PNG) {
            formatStr = QString("png");
        } else if (m_saveInfo.second == SubToolWidget::JPG) {
            formatStr = QString("jpg");
        } else if (m_saveInfo.second == SubToolWidget::BMP) {
            formatStr = QString("bmp");
        }
        m_lastImagePath = QString("%1/%2.%3").arg(savePath).arg(m_imageName).arg(formatStr);
    } else if (m_saveInfo.first == SubToolWidget::PICTURES) {
        qDebug() << "保存到图片";
        QString savePath = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first() + QDir::separator() + "Screenshots";
        if ((!QDir(savePath).exists() && QDir().mkdir(savePath) == false) ||  // 文件不存在，且创建失败
                (QDir(savePath).exists() && !QFileInfo(savePath).isWritable())) {  // 文件存在，且不能写
            savePath = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first();
        }
        QString formatStr;
        if (m_saveInfo.second == SubToolWidget::PNG) {
            formatStr = QString("png");
        } else if (m_saveInfo.second == SubToolWidget::JPG) {
            formatStr = QString("jpg");
        } else if (m_saveInfo.second == SubToolWidget::BMP) {
            formatStr = QString("bmp");
        }
        m_lastImagePath = QString("%1/%2.%3").arg(savePath).arg(m_imageName).arg(formatStr);
    } else if (m_saveInfo.first == SubToolWidget::FOLDER_CHANGE) {
        m_toolBar->hide();
        this->hide();
        QString saveFileName;
        qDebug() << "保存到指定位置";
        QString imgName = Settings::instance()->getSavePath();
        if (!imgName.isEmpty()) {
            imgName += "/";
        } else {
            imgName = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first() + QDir::separator() + "Screenshots";
        }
        if (m_saveInfo.second == SubToolWidget::PNG) {
            imgName += QString("%1.png").arg(m_imageName);
            saveFileName = QFileDialog::getSaveFileName(this, tr("Save"), imgName,
                                                        tr("PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp)"));
        } else if (m_saveInfo.second == SubToolWidget::JPG) {
            imgName += QString("%1.jpg").arg(m_imageName);
            saveFileName = QFileDialog::getSaveFileName(this, tr("Save"),  imgName,
                                                        tr("JPEG (*.jpg *.jpeg);;PNG (*.png);;BMP (*.bmp)"));
        } else if (m_saveInfo.second == SubToolWidget::BMP) {
            imgName += QString("%1.bmp").arg(m_imageName);
            saveFileName = QFileDialog::getSaveFileName(this, tr("Save"),  imgName,
                                                        tr("BMP (*.bmp);;JPEG (*.jpg *.jpeg);;PNG (*.png)"));
        }
        if (saveFileName.isEmpty()) {
            qWarning() << "指定路径为空！";
            onExit();
            return;
        }
        qDebug() << "saveFileName" << saveFileName;
        m_lastImagePath = saveFileName;
        Settings::instance()->setSavePath(QFileInfo(saveFileName).path());
    } else if (m_saveInfo.first == SubToolWidget::FOLDER) {
        QString savePath = Settings::instance()->getSavePath();
        qDebug() << "保存到历史路径" << savePath;
        QString formatStr;
        if (m_saveInfo.second == SubToolWidget::PNG) {
            formatStr = QString("png");
        } else if (m_saveInfo.second == SubToolWidget::JPG) {
            formatStr = QString("jpg");
        } else if (m_saveInfo.second == SubToolWidget::BMP) {
            formatStr = QString("bmp");
        }
        m_lastImagePath = QString("%1/%2.%3").arg(savePath).arg(m_imageName).arg(formatStr);
    } else if (m_saveInfo.first == SubToolWidget::CLIPBOARD) {
        qDebug() << "save to clip";
        m_lastImagePath = "";
    }
    //每次关闭贴图时重设isChangeSavePath
    Settings::instance()->setIsChangeSavePath(false);

    bool isSaveState = true;
    if (m_saveInfo.first != SubToolWidget::CLIPBOARD) {
        isSaveState = m_image.save(m_lastImagePath);
    }

    // 保存到剪贴板
    QMimeData *t_imageData = new QMimeData;
    t_imageData->setImageData(m_image);
    QClipboard *cb = qApp->clipboard();
    cb->setMimeData(t_imageData, QClipboard::Clipboard);

    if (Utils::isWaylandMode) {
        QEventLoop eventloop;
        connect(cb, SIGNAL(dataChanged()), &eventloop, SLOT(quit()));
        eventloop.exec();
    }

    // 发送通知
    sendNotify(m_lastImagePath, isSaveState);
}
// 获取贴图窗口的显示位置
QPoint MainWindow::getShowPosition()
{
    return m_showPosition;
}

// 保存
void MainWindow::onSave()
{
    qDebug() << "func: " << __func__ ;
    saveImg();
}
// 退出
void MainWindow::onExit()
{
    //每次关闭贴图时重设isChangeSavePath
    Settings::instance()->setIsChangeSavePath(false);
    qDebug() << "func: " << __func__ ;
    m_toolBar->close();
    this->close();
}

void MainWindow::region(const QPoint &cursorGlobalPoint)
{
    // 获取窗体在屏幕上的位置区域，tl为topleft点，rb为rightbottom点
    QRect rect = this->rect();
    QPoint tl = mapToGlobal(rect.topLeft());
    QPoint rb = mapToGlobal(rect.bottomRight());

    int x = cursorGlobalPoint.x();
    int y = cursorGlobalPoint.y();

    if (tl.x() + PADDING >= x && tl.x() <= x && tl.y() + PADDING >= y && tl.y() <= y) {
        // 左上角
        dir = LEFTTOP;
        this->setCursor(QCursor(Qt::SizeFDiagCursor));  // 设置鼠标形状
    } else if (x >= rb.x() - PADDING && x <= rb.x() && y >= rb.y() - PADDING && y <= rb.y()) {
        // 右下角
        dir = RIGHTBOTTOM;
        this->setCursor(QCursor(Qt::SizeFDiagCursor));
    } else if (x <= tl.x() + PADDING && x >= tl.x() && y >= rb.y() - PADDING && y <= rb.y()) {
        //左下角
        dir = LEFTBOTTOM;
        this->setCursor(QCursor(Qt::SizeBDiagCursor));
    } else if (x <= rb.x() && x >= rb.x() - PADDING && y >= tl.y() && y <= tl.y() + PADDING) {
        // 右上角
        dir = RIGHTTOP;
        this->setCursor(QCursor(Qt::SizeBDiagCursor));
    } else {
        // 默认
        dir = NONE;
        this->setCursor(QCursor(Qt::ArrowCursor));
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    //qDebug() << this << __FUNCTION__ << __LINE__ ;
    switch (event->button()) {
    case Qt::LeftButton:
        isLeftPressDown = true;
        if (dir != NONE) {
            this->mouseGrabber();
        } else {
            dragPosition = event->globalPos() - this->frameGeometry().topLeft();
        }
        if (!m_toolBar->isHidden())
            m_toolBar->hide(); //隐藏工具栏
        //qDebug() << this << __FUNCTION__ << __LINE__ ;
        break;
    case Qt::RightButton:
        handleMouseRightBtn(event);
        break;
    default:
        QWidget::mousePressEvent(event);
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    QPoint gloPoint = event->globalPos();
    QRect rect = this->rect();
    QPoint tl = mapToGlobal(rect.topLeft());
    QPoint br = mapToGlobal(rect.bottomRight());
    QPoint bl = mapToGlobal(rect.bottomLeft());
    QPoint tr = mapToGlobal(rect.topRight());

    if (!isLeftPressDown) {
        this->region(gloPoint);
    } else {
        if (dir != NONE) {
            QRect rMove(tl, br);
            QPoint newPoint;
            switch (dir) {
            case LEFTTOP:
                newPoint = QPointF(br.x() - gloPoint.x(), static_cast<int>((br.x() - gloPoint.x()) / proportion)).toPoint();
                rMove.setX(br.x() - newPoint.x());
                rMove.setY(br.y() - newPoint.y());
                if (rMove.x() +  WHEELNUM >= br.x() || rMove.y() + WHEELNUM >= br.y()) {
                    rMove.setX(br.x() - WHEELNUM);
                    rMove.setY(br.y() - WHEELNUM);
                }
                if (br.y() - rMove.height() <= 0) {
                    int heigt = br.y();
                    int width = static_cast<int>(heigt * proportion);
                    rMove.setX(br.x() - width);
                    rMove.setY(br.y() - heigt);
                }
                break;
            case RIGHTTOP:
                newPoint = QPointF(tl.x() + static_cast<int>((bl.y() - gloPoint.y()) * proportion), gloPoint.y()).toPoint();
                if (newPoint.x() -  WHEELNUM <= bl.x() || newPoint.y() + WHEELNUM >= bl.y()) {
                    rMove.setTopRight(QPoint(bl.x() + WHEELNUM, bl.y() - WHEELNUM));
                } else {
                    rMove.setTopRight(newPoint);
                }
                break;
            case LEFTBOTTOM:
                newPoint = QPointF(gloPoint.x(), static_cast<int>(tr.y() + ((tr.x() - gloPoint.x())) / proportion)).toPoint();
                if (newPoint.x() + WHEELNUM >= tr.x() || newPoint.y() - WHEELNUM <= tr.y()) {
                    rMove.setBottomLeft(QPoint(tr.x() - WHEELNUM, tr.y() + WHEELNUM));
                } else {
                    if (newPoint.y() >= m_screenSize.height()) {
                        int height = m_screenSize.height() - tr.y();
                        int width = static_cast<int>(height * proportion);
                        newPoint.setY(m_screenSize.height());
                        newPoint.setX(tr.x() - width);
                    }
                    rMove.setBottomLeft(newPoint);

                }
                break;
            case RIGHTBOTTOM:
                newPoint = QPointF(gloPoint.x() - tl.x(), static_cast<int>((gloPoint.x() - tl.x()) / proportion)).toPoint();
                rMove.setWidth(gloPoint.x() - tl.x());
                rMove.setHeight(newPoint.y());
                if (rMove.y() + rMove.height() >= m_screenSize.height()) {
                    int heigt = m_screenSize.height() - rMove.y();
                    int width = static_cast<int>(heigt * proportion);
                    rMove.setWidth(width);
                    rMove.setHeight(heigt);
                }
                break;
            default:
                break;
            }
            this->setGeometry(rMove);
        } else {
            this->setCursor(QCursor(Qt::ClosedHandCursor));
            //qDebug() << "=============event->globalPos()" << event->globalPos() << "dragPosition" << dragPosition;
            QPoint globalPoint = event->globalPos() - dragPosition;
            move(globalPoint);
            event->accept();
        }
    }
    QWidget::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
//    qDebug() << this << __FUNCTION__ << __LINE__ ;
    if (event->button() == Qt::LeftButton) {
        isLeftPressDown = false;
        if (dir != NONE) {
            this->releaseMouse();
        }
        this->setCursor(QCursor(Qt::ArrowCursor));
        if (m_toolBar->isHidden())
            updateToolBarPosition();
    }
}

void MainWindow::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    if (!m_image.isNull()) {
        QPainter pp(this);
        QRect Temp(0, 0, this->width(), this->height());
        // 用rect获取到的图片不容易失帧
        //QImage tempImage = m_image.scaled(this->width(), this->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        pp.drawImage(Temp, m_image);
    }


}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    int x = this->pos().x();
    int y = this->pos().y();
    bool isNeedUpdateToolBar = false;
    if (event->key() == Qt::Key_Left) {
        this->move(x - MOVENUM, y);
        isNeedUpdateToolBar = true;
    } else if (event->key() == Qt::Key_Right) {
        this->move(x + MOVENUM, y);
        isNeedUpdateToolBar = true;
    } else if (event->key() == Qt::Key_Up) {
        // 适配wayland
        if (y - MOVENUM < 0) {
            this->move(x, 0);
        } else {
            this->move(x, y - MOVENUM);
        }

        isNeedUpdateToolBar = true;
    } else if (event->key() == Qt::Key_Down) {
        this->move(x, y + MOVENUM);
        isNeedUpdateToolBar = true;
    }

    if (isNeedUpdateToolBar) {
        updateToolBarPosition();
    }
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    QRect rect;
    int width = this->rect().width();
    int height = this->rect().height();
    QPointF unionPoint(event->posF().x() / width, event->posF().y() / height);
    //qDebug() << unionPoint;
    if (event->delta() < 0 && (width < this->minimumWidth() || height < this->minimumHeight()))
        return;
    if (event->delta() > 0) {
        QPoint newPoint = QPointF(width + WHEELNUM, static_cast<int>((width + WHEELNUM) / proportion)).toPoint();
        rect.setWidth(newPoint.x());
        rect.setHeight(newPoint.y());
    } else {
        QPoint newPoint = QPointF(width - WHEELNUM, static_cast<int>((width - WHEELNUM) / proportion)).toPoint();
        rect.setWidth(newPoint.x());
        rect.setHeight(newPoint.y());
    }
    QPointF globalPos = event->globalPos(); // 鼠标在屏幕中的坐标
    //qDebug()<<"globalPos"<<globalPos;
    QPointF newPoint(rect.width() * unionPoint.x(), rect.height() * unionPoint.y()); //扩大后，鼠标在贴图中的坐标
    QPointF topLeft(globalPos.x() - newPoint.x(), globalPos.y() - newPoint.y()); // 贴图左上角的点在屏幕中的坐标
    rect = QRect(topLeft.toPoint(), rect.size());

    this->setGeometry(rect.x(), rect.y(), rect.width(), rect.height());
    updateToolBarPosition(); //更新位置
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this) {
        //QEvent::WindowActivate:窗口被激活;QEvent::WindowDeactivate:窗口已停用
        if (QEvent::WindowActivate == event->type()) {
            //qDebug() <<this<< m_toolBar << __FUNCTION__ << __LINE__ << event->type();
            //规避wayland下打开贴图1，并在贴图1上重新生成贴图2时，贴图2激活状态不对(工具栏已经设置显示，但实际上未显示。)。
            if (Utils::isWaylandMode && !isLeftPressDown) {
                this->m_toolBar->hide();
            }
            updateToolBarPosition();
            //规避wayland下打开多个贴图，通过鼠标连续切换贴图时，贴图激活状态不对(工具栏已经设置显示，但实际上未显示。)。
            if (Utils::isWaylandMode && isLeftPressDown) {
                this->m_toolBar->hide();
            }
            //qDebug() <<this<< m_toolBar << __FUNCTION__ << __LINE__ << event->type();
            return false;
        } else if (QEvent::WindowDeactivate == event->type()) {
            //qDebug() << this << m_toolBar <<__FUNCTION__ << __LINE__ << event->type();
            if (m_toolBar->isActiveWindow())
                return false;
            m_toolBar->hide();
            //qDebug() << this << m_toolBar <<__FUNCTION__ << __LINE__ << event->type();
            return false;
        }
    }
    return false;
}

// 初始化快捷键
void MainWindow::initShortcut()
{
    // Esc —— 退出
    QShortcut *Keyesc = new QShortcut(QKeySequence("Escape"), this);
    connect(Keyesc, &QShortcut::activated, this, [ = ] {
        qDebug() << "shortcut : Keyesc (key: esc)";
        onExit();
    });

    // Ctrl+S —— 保存
    QShortcut *Keysave = new QShortcut(QKeySequence("Ctrl+S"), this);
    connect(Keysave, &QShortcut::activated, this, [ = ] {
        qDebug() << "shortcut : Keysave (key: ctrl+s)";
        onSave();
    });

    // Alt+O —— 文字识别
    QShortcut *Keyocr = new QShortcut(QKeySequence("Alt+O"), this);
    connect(Keyocr, &QShortcut::activated, this, [ = ] {
        qDebug() << "shortcut : Keyocr (key: alt+o)";
        onOpenOCR();
    });


    // F1 —— 显示帮助 测试
    QShortcut *Keyhelp = new QShortcut(QKeySequence("F1"), this);
    connect(Keyhelp, &QShortcut::activated, this, [ = ] {
        qDebug() << "shortcut : Keyhelp (key: F1)";
        QDBusInterface interFace("com.deepin.Manual.Open",
                                 "/com/deepin/Manual/Open",
                                 "com.deepin.Manual.Open",
                                 QDBusConnection::sessionBus());
        // 帮助手册跳转到子标题，传入子标题标签(标签文档内唯一，固定为英文，与翻译无关)
        QList<QVariant> arg;
        arg << (QString("deepin-screen-recorder"))                  // 应用名称
            << QString("pinscreenshots");                         // 帮助文案中的标题名称
        interFace.callWithArgumentList(QDBus::AutoDetect, "OpenTitle", arg);
    });

    QShortcut *KeyF3 = new QShortcut(QKeySequence("F3"), this);
    connect(KeyF3, &QShortcut::activated, this, [ = ] {
        qDebug() << "shortcut : Keyocr (key: F3)";
        m_toolBar->onOptionButtonClicked();
    });
}

// 计算屏幕大小
void MainWindow::CalculateScreenSize()
{
    //qreal pixelRatio = qApp->primaryScreen()->devicePixelRatio();
    QList<QScreen *> screenList = qApp->screens();
    for (auto it = screenList.constBegin(); it != screenList.constEnd(); ++it) {
        QRect rect = (*it)->geometry();
        qDebug() << (*it)->name() << rect;
        ScreenInfo screenInfo;
        screenInfo.x = rect.x();
        screenInfo.y = rect.y();
        screenInfo.height =  static_cast<int>(rect.height());
        screenInfo.width = static_cast<int>(rect.width());
        screenInfo.name = (*it)->name();
        m_screenInfo.append(screenInfo);
    }

    m_screenSize.setWidth(m_screenInfo[0].x + m_screenInfo[0].width);
    m_screenSize.setHeight(m_screenInfo[0].y + m_screenInfo[0].height);

    // 通过每个屏幕， 右下角的坐标来计算屏幕总大小。
    for (int i = 1; i < m_screenInfo.size(); ++i) {
        if ((m_screenInfo[i].height + m_screenInfo[i].y) > m_screenSize.height())
            m_screenSize.setHeight(m_screenInfo[i].height + m_screenInfo[i].y);

        if ((m_screenInfo[i].width + m_screenInfo[i].x) > m_screenSize.width())
            m_screenSize.setWidth(m_screenInfo[i].width + m_screenInfo[i].x);
    }

    qDebug() << m_screenSize;
    if (m_screenInfo.size() > 1) {
        // 排序
        qSort(m_screenInfo.begin(), m_screenInfo.end(), [ = ](const ScreenInfo info1, const ScreenInfo info2) {
            return info1.x < info2.x;
        });
    }
}

//处理鼠标右键
void MainWindow::handleMouseRightBtn(QMouseEvent *mouseEvent)
{
    qDebug() << "func: " << __func__ ;
    if (m_menuController != nullptr) {
        m_menuController->showMenu(QPoint(mapToGlobal(mouseEvent->pos())));
    } else {
        qDebug() << "the m_menuController is nullptr!!!";
    }
}

// 发送通知
void MainWindow::sendNotify(QString savePath, bool bSaveState)
{
    QDBusInterface notification("org.freedesktop.Notifications",
                                "/org/freedesktop/Notifications",
                                "org.freedesktop.Notifications",
                                QDBusConnection::sessionBus());

    if (!bSaveState) {
        qDebug() << __FUNCTION__;
        QString tips = QString(tr("Save failed. Please save it in your home directory."));
        Settings::instance()->setSavePath("");
        QList<QVariant> arg;
        unsigned int id = 0;
        int timeout = -1;
        arg << tr("Pin Screenshots") << id << "deepin-screen-recorder" << QString() << tips << QStringList() << QVariantMap() << timeout;
        notification.callWithArgumentList(QDBus::AutoDetect, "Notify", arg);
        onExit();
        return;
    }

    QStringList actions;
    QVariantMap hints;
    QString body;
    // 如果保存路径为剪切板，不做打开view操作
    if (!savePath.isEmpty()) {
        actions << "_open" << tr("View");
        if (!QStandardPaths::findExecutable("dde-file-manager").isEmpty()) {
            qDebug() << QFileInfo(savePath).path();
            hints["x-deepin-action-_open"] = QString("dde-file-manager,--show-item,%1").arg(savePath);
        } else {
            hints["x-deepin-action-_open"] = QString("xdg-open,%1").arg(savePath);
        }
        body = QString(tr("Saved to %1")).arg(savePath);
    }
    int timeout = -1;
    unsigned int id = 0;
    QList<QVariant> arg;
    arg << tr("Pin Screenshots")                 // appname
        << id                                                    // id
        << QString("deepin-screen-recorder")                     // icon
        << tr("Screenshot finished")                              // summary
        << body              // body
        << actions                                               // actions
        << hints                                                 // hints
        << timeout;                                              // timeout
    notification.callWithArgumentList(QDBus::AutoDetect, "Notify", arg);
    onExit();
}

// 更新工具栏显示位置
void MainWindow::updateToolBarPosition()
{
    //获取贴图界面右下角的点
    QPoint brPoint = mapToGlobal(this->rect().bottomRight());
    int x = 0, y = 0;
//    qDebug() << "updateToolBarPosition brPoint is" << brPoint;
    //判断工具栏放贴图界面下方时是否会超出屏幕下方
    if ((brPoint.y() + TOOLBAR_Y_SPACING + m_toolBar->height()) >= m_screenSize.height()) {
        //此时工具栏放贴图界面下方会超出屏幕，只能放贴图界面上方
        //获取贴图界面右上角的点
        QPoint trPoint = mapToGlobal(this->rect().topRight());
        //判断工具栏是否超出屏幕上方
        if (trPoint.y() - TOOLBAR_Y_SPACING - m_toolBar->height() <= 0) {
            //工具栏放贴图界面上下方都会超出屏幕，此时就工具栏放屏幕上方
            x = trPoint.x() - m_toolBar->width();
            y = 0;
        } else {
            x = trPoint.x() - m_toolBar->width();
            y = trPoint.y() - TOOLBAR_Y_SPACING - m_toolBar->height();
        }
    } else {
        //此时工具栏放贴图界面下方不会超出屏幕可以放界面下方
        x = brPoint.x() - m_toolBar->width();
        y = brPoint.y() + TOOLBAR_Y_SPACING;
        //qDebug() << "m_toolBar->width()" << m_toolBar->toolBarWidth();
    }
    if (x < 0) {
        x = 0;
    } else if (x + m_toolBar->width() > m_screenSize.width()) {
        x = m_screenSize.width() - m_toolBar->width();
    }
    m_toolBar->showAt(QPoint(x, y), m_isfirstTime);
    m_isfirstTime = false;
}


