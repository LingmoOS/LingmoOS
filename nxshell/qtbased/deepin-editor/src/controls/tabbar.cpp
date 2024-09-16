// SPDX-FileCopyrightText: 2017 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tabbar.h"
#include "../widgets/window.h"
#include "../common/utils.h"
#include "../editor/editwrapper.h"
#include "../startmanager.h"

#include <QApplication>
#include <QStyleFactory>
#include <QGuiApplication>
#include <DPlatformWindowHandle>
#include <DWindowManagerHelper>
#include <QPixmap>
#include <DFrame>
#include <QDesktopWidget>
#include <DGuiApplicationHelper>

QPixmap *Tabbar::sm_pDragPixmap = nullptr;

// 不同模式下的界面调整
const int s_TabbarHeight = 40;
const int s_TabbarHeightCompact = 26;

/**
 * @brief ‘&’在Qt中被标记为助记符，替换 \a str 中的‘&’字符为“&&”，以正确显示文件名中的‘&’符号
 *      painter 绘制 CE_ToolButtonLabel 时设置了 Qt::TextShowMnemonic
 * @note 在插入、新建标签页时，内部已使用替换，部分外部更新标签页名称，需手动处理
 * @sa Tabbar::addTabWithIndex(), Tabbar::insertFromMimeData(), Tabbar::setTabText()
 */
QString replaceMnemonic(const QString &str)
{
    QString tmp = str;
    tmp.replace(QChar('&'), QString("&&"));
    return tmp;
}

/**
 * @brief ‘&’在Qt中被标记为助记符，替换 \a str 中的"&&字符为'&'，以正确取得文件名中的‘&’符号
 */
QString restoreMnemonic(const QString &str)
{
    QString tmp = str;
    tmp.replace(QString("&&"), QChar('&'));
    return tmp;
}

Tabbar::Tabbar(QWidget *parent)
    : DTabBar(parent)
{
    m_rightClickTab = -1;

    installEventFilter(this);

    setMovable(true);
    setTabsClosable(true);
    setVisibleAddButton(true);
    setDragable(true);
    // setStartDragDistance(40);
    setElideMode(Qt::ElideMiddle);
    setTabPalette(palette().buttonText().color().name(), palette().highlightedText().color().name());
    setFocusPolicy(Qt::NoFocus);

    connect(this, &DTabBar::dragStarted, this, &Tabbar::onTabDrapStart);
    connect(this, &DTabBar::tabMoved, this, &Tabbar::handleTabMoved);
    connect(this, &DTabBar::tabDroped, this, &Tabbar::handleTabDroped);
    connect(this, &DTabBar::tabIsRemoved, this, &Tabbar::handleTabIsRemoved);
    connect(this, &DTabBar::tabReleaseRequested, this, &Tabbar::handleTabReleased);
    connect(this, &DTabBar::dragActionChanged, this, &Tabbar::handleDragActionChanged);

#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [ this ](){
        // 更新当前控件，自动调用 sizeHint() 更新界面参数
        this->update();
    });
#endif
}

Tabbar::~Tabbar()
{
    if (m_moreWaysCloseMenu != nullptr) {
        delete m_moreWaysCloseMenu;
        m_moreWaysCloseMenu = nullptr;
    }
    if (m_rightMenu != nullptr) {
        delete m_rightMenu;
        m_rightMenu = nullptr;
    }
}

void Tabbar::addTab(const QString &filePath, const QString &tabName, const QString &tipPath)
{
    addTabWithIndex(count(), filePath, tabName, tipPath);
}

void Tabbar::addTabWithIndex(int index, const QString &filePath, const QString &tabName, const QString &tipPath)
{
    // FIXME(rekols): do not insert duplicate values.
    if (m_tabPaths.contains(filePath)) {
        return;
    }
    m_tabPaths.insert(index, filePath);
    m_tabTruePaths.insert(index, tipPath);
    // }

    // 除去空白符 梁卫东 ２０２０－０８－２６　１４：４９：１５ ；适配助记符
    QString trimmedName = replaceMnemonic(tabName.simplified());
    DTabBar::insertTab(index, trimmedName);
    DTabBar::setCurrentIndex(index);
    if (filePath.contains(Utils::localDataPath())) {
        if (Utils::isBackupFile(filePath) && !tipPath.isNull() && tipPath.length() > 0) {
            setTabToolTip(index, tipPath);
        } else {
            setTabToolTip(index, tabName);
        }
    } else {
        QString path = filePath;
        QFontMetrics fontMetrics(font());
        int nFontWidth = fontMetrics.width(path) * (qApp->devicePixelRatio() == 1.25 ? 2 : 1);

        Window *pWindow = static_cast<Window *>(this->window());
        int w = pWindow->width() - 200;
        if (w < 800) w = 800;

        if (nFontWidth >= w) {
            int mod = nFontWidth % w;

            int step = nFontWidth / w + (mod > 0 ? 1 : 0);

            for (int i = 1; i < step; i++) {
                path.insert(i * (path.length() / step), '\n');
            }
        }

        setTabToolTip(index, path);
    }
}

void Tabbar::resizeEvent(QResizeEvent *event)
{
    int cnt = count();

    for (int i = 0; i < cnt ; i++) {
        QString path = tabToolTip(i);
        path = path.replace("\n", "");
        QFontMetrics fontMetrics(font());
        int nFontWidth = fontMetrics.width(path) * (qApp->devicePixelRatio() == 1.25 ? 2 : 1);

        Window *pWindow = static_cast<Window *>(this->window());
        int w = pWindow->width() - 200;
        if (w < 800) w = 800;

        if (nFontWidth >= w) {
            int mod = nFontWidth % w;

            int step = nFontWidth / w + (mod > 0 ? 1 : 0);

            for (int j = 1; j < step; j++) {
                path.insert(j * (path.length() / step), '\n');
            }
        }

        setTabToolTip(i, path);
    }

    // 临时修改方案：通过调用setIconSize()，更新内部的layoutDirty标识，强制重新刷新标签页布局, BUG链接：https://pms.uniontech.com/bug-view-137607.html
    // TODO: 需要dtk暴露接口重新布局
    setIconSize(iconSize());

    DTabBar::resizeEvent(event);
}

void Tabbar::closeTab(int index)
{
    if (index < 0) {
        return;
    }
    emit requestHistorySaved(fileAt(index));
    DTabBar::removeTab(index);
}

void Tabbar::closeCurrentTab()
{
    closeTab(currentIndex());
}

void Tabbar::closeCurrentTab(const QString &strFilePath)
{
    closeTab(this->indexOf(strFilePath));
}

void Tabbar::closeOtherTabs()
{
    closeOtherTabsExceptFile(currentPath());
}

void Tabbar::closeOtherTabsExceptFile(const QString &filePath)
{
    QStringList closePathList;

    for (const QString &path : m_tabPaths) {
        if (filePath != path) {
            closePathList << path;
        }
    }

    emit closeTabs(closePathList);
}

void Tabbar::closeLeftTabs(const QString &filePath)
{
    QStringList closePathList;

    for (const QString &path : m_tabPaths) {
        if (filePath == path) {
            break;
        }
        closePathList << path;
    }
    emit closeTabs(closePathList);

}

void Tabbar::closeRightTabs(const QString &filePath)
{

    QStringList closePathlist;


    for (int i = m_tabPaths.count() - 1; i >= 0; i--) {
        m_tabPaths.value(i);

        if (filePath == m_tabPaths.value(i)) {
            break;
        }
        closePathlist << m_tabPaths.value(i);
    }

    emit closeTabs(closePathlist);
}

void Tabbar::updateTab(int index, const QString &filePath, const QString &tabName)
{
    // 适配助记符 '&' 后设置文本
    setTabText(index, tabName);
    m_tabPaths[index] = filePath;
    m_tabTruePaths[index] = filePath;

    //show file path at tab,blank file only show it's name.
    if (filePath.contains(Utils::localDataPath())) {
        setTabToolTip(index, tabName);
    } else {
        QString path = filePath;
        QFontMetrics fontMetrics(font());
        int nFontWidth = fontMetrics.width(path) * (qApp->devicePixelRatio() == 1.25 ? 2 : 1);

        Window *pWindow = static_cast<Window *>(this->window());
        int w = pWindow->width() - 200;
        if (w < 800) w = 800;

        if (nFontWidth >= w) {
            int mod = nFontWidth % w;

            int step = nFontWidth / w + (mod > 0 ? 1 : 0);

            for (int i = 1; i < step; i++) {
                path.insert(i * (path.length() / step), '\n');
            }
        }

        setTabToolTip(index, path);
    }
}

void Tabbar::previousTab()
{
    int currentIndex = DTabBar::currentIndex();

    if (currentIndex <= 0) {
        DTabBar::setCurrentIndex(DTabBar::count() - 1);
    } else {
        DTabBar::setCurrentIndex(currentIndex - 1);
    }
}

void Tabbar::nextTab()
{
    int currentIndex = DTabBar::currentIndex();

    if (currentIndex >= DTabBar::count() - 1) {
        DTabBar::setCurrentIndex(0);
    } else {
        DTabBar::setCurrentIndex(currentIndex + 1);
    }
}

int Tabbar::indexOf(const QString &filePath)
{
    return m_tabPaths.indexOf(filePath);
}

QString Tabbar::currentName() const
{
    return textAt(currentIndex());
}

QString Tabbar::currentPath() const
{
    return m_tabPaths.value(currentIndex());
}

QString Tabbar::truePathAt(int index) const
{
    return m_tabTruePaths.value(index);
}

QString Tabbar::fileAt(int index) const
{
    return m_tabPaths.value(index);
}

QString Tabbar::textAt(int index) const
{
    // 获取显示文本时恢复设置的助记符 '&'
    return restoreMnemonic(DTabBar::tabText(index));
}

/**
 * @brief 设置索引 \a index 指向标签页显示文本为 \a text
 */
void Tabbar::setTabText(int index, const QString &text)
{
    // 替换助记符
    QString tmp = replaceMnemonic(text);
    DTabBar::setTabText(index, tmp);
}

void Tabbar::setTabPalette(const QString &activeColor, const QString &highlightColor)
{
    QPalette pa = this->palette();
    pa.setColor(QPalette::Inactive, QPalette::HighlightedText, QColor(highlightColor));
    pa.setColor(QPalette::Inactive, QPalette::WindowText, QColor(activeColor));
    pa.setColor(QPalette::Active, QPalette::WindowText, QColor(activeColor));
    setPalette(pa);
}

void Tabbar::setBackground(const QString &startColor, const QString &endColor)
{
    m_backgroundStartColor = startColor;
    m_backgroundEndColor = endColor;
}

void Tabbar::setDNDColor(const QString &startColor, const QString &endColor)
{
    m_dndStartColor = startColor;
    m_dndEndColor = endColor;
}

QPixmap Tabbar::createDragPixmapFromTab(int index, const QStyleOptionTab &option, QPoint *hotspot) const
{
    const qreal ratio = qApp->devicePixelRatio();

    Window *window = static_cast<Window *>(this->window());
    EditWrapper *wrapper = window->wrapper(fileAt(index));
    //加载大文本不允许拖拽
    //if(wrapper && wrapper->getFileLoading()) return QPixmap();

    TextEdit *textEdit = wrapper->textEditor();

    int width = textEdit->width() * ratio;
    int height = textEdit->height() * ratio;
    QImage screenshotImage(width, height, QImage::Format_ARGB32_Premultiplied);
    screenshotImage.setDevicePixelRatio(ratio);
    textEdit->render(&screenshotImage, QPoint(), QRegion(0, 0, width, height));

    // scaled image to smaller.
    int scaledWidth = width * ratio / 5;
    int scaledHeight = height * ratio / 5;
    auto scaledImage = screenshotImage.scaled(scaledWidth, scaledHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    QImage backgroundImage(scaledWidth + 10, scaledHeight + 10, QImage::Format_ARGB32_Premultiplied);
    backgroundImage.fill(QColor(palette().color(QPalette::Base)));
    // clip screenshot image with window radius.
    QPainter painter(&backgroundImage);
    painter.drawImage(5, 5, scaledImage);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (!Utils::isWayland() && count() == 1) {
        this->window()->showMinimized();
    }

    // adjust offset.
    hotspot->setX(scaledWidth / 2);
    hotspot->setY(scaledHeight / 2);

    QPainterPath rectPath;

    if (DWindowManagerHelper::instance()->hasComposite()) {
        QPainterPath roundedRectPath;

        rectPath.addRect(0, 0, scaledWidth + 10, scaledHeight + 10);
        roundedRectPath.addRoundedRect(QRect(0, 0, scaledWidth / ratio + 10, scaledHeight / ratio + 10), 6, 6);

        rectPath -= roundedRectPath;

        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.fillPath(rectPath, Qt::transparent);

        QColor shadowColor = QColor(palette().color(QPalette::BrightText));
        shadowColor.setAlpha(80);

        painter.end();
        if (sm_pDragPixmap) delete sm_pDragPixmap;
        sm_pDragPixmap = new QPixmap(Utils::dropShadow(QPixmap::fromImage(backgroundImage), 5, shadowColor, QPoint(0, 0)));
        return Utils::dropShadow(QPixmap::fromImage(backgroundImage), 5, shadowColor, QPoint(0, 0));
    } else {
        painter.end();
        if (sm_pDragPixmap) delete sm_pDragPixmap;
        sm_pDragPixmap = new QPixmap(QPixmap::fromImage(backgroundImage));
        return QPixmap::fromImage(backgroundImage);
    }

#if 0
    QPixmap backgroundImage = DTabBar::createDragPixmapFromTab(index, option, hotspot);
    if (sm_pDragPixmap) delete sm_pDragPixmap;
    sm_pDragPixmap = new QPixmap(backgroundImage);
    return backgroundImage;
#endif
}

QMimeData *Tabbar::createMimeDataFromTab(int index, const QStyleOptionTab &option) const
{
    const QString tabName = textAt(index);

    Window *window = static_cast<Window *>(this->window());
    EditWrapper *wrapper = window->wrapper(fileAt(index));

    if (wrapper && wrapper->getFileLoading()) return nullptr;

    QMimeData *mimeData = new QMimeData;
    mimeData->setParent(window);

    if (!wrapper) {
        //m_tabbar->closeCurrentTab();
        return nullptr;
    }

    mimeData->setProperty("wrapper", QVariant::fromValue(static_cast<void *>(wrapper)));
    mimeData->setProperty("isModified", wrapper->isModified());
    mimeData->setData("dedit/tabbar", tabName.toUtf8());
    mimeData->removeFormat("text/plain");

    return mimeData;
}

void Tabbar::insertFromMimeDataOnDragEnter(int index, const QMimeData *source)
{
    if (source == nullptr) {
        return;
    }

    const QString tabName = QString::fromUtf8(source->data("dedit/tabbar"));
    QVariant pVar = source->property("wrapper");
    EditWrapper *wrapper = static_cast<EditWrapper *>(pVar.value<void *>());

    //大文本加载不允许拖拽
    if (wrapper && (wrapper && wrapper->getFileLoading())) return;

    Window *window = static_cast<Window *>(this->window());

    if (!wrapper) {
        return;
    }

    window->addTabWithWrapper(wrapper, wrapper->textEditor()->getFilePath(), wrapper->textEditor()->getTruePath(), tabName, index);
    //window->currentWrapper()->textEditor()->setModified(source->property("isModified").toBool());
    wrapper->updateModifyStatus(source->property("isModified").toBool());
    wrapper->OnUpdateHighlighter();
    window->focusActiveEditor();
}

void Tabbar::insertFromMimeData(int index, const QMimeData *source)
{
    if (source == nullptr) {
        return;
    }

    const QString tabName = QString::fromUtf8(source->data("dedit/tabbar"));
    QVariant pVar = source->property("wrapper");
    EditWrapper *wrapper = static_cast<EditWrapper *>(pVar.value<void *>());
    Window *window = static_cast<Window *>(this->window());

    if (!wrapper) {
        return;
    }

    //qDebug() << "insertFromMimeData";
    window->addTabWithWrapper(wrapper, wrapper->textEditor()->getFilePath(), wrapper->textEditor()->getTruePath(), tabName, index);
    //window->currentWrapper()->textEditor()->setModified(source->property("isModified").toBool());
    wrapper->updateModifyStatus(source->property("isModified").toBool());
    wrapper->OnUpdateHighlighter();
    window->focusActiveEditor();
}

bool Tabbar::canInsertFromMimeData(int index, const QMimeData *source) const
{
    return source->hasFormat("dedit/tabbar");
}

void Tabbar::handleDragActionChanged(Qt::DropAction action)
{
    // Reset cursor to Qt::ArrowCursor if drag tab to TextEditor widget.
    if (action == Qt::IgnoreAction) {
        if (dragIconWindow()) {
            QGuiApplication::changeOverrideCursor(Qt::ArrowCursor);
            DPlatformWindowHandle::setDisableWindowOverrideCursor(dragIconWindow(), true);
        }
    } else if (dragIconWindow()) {
        DPlatformWindowHandle::setDisableWindowOverrideCursor(dragIconWindow(), false);
        if (QGuiApplication::overrideCursor())
            QGuiApplication::changeOverrideCursor(QGuiApplication::overrideCursor()->shape());
    }
}

bool Tabbar::eventFilter(QObject *, QEvent *event)
{

    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

        if (mouseEvent->button() == Qt::RightButton) {
            QPoint position = mouseEvent->pos();
            m_rightClickTab = this->tabAt(position);
            int indexCount = this->count();

//            m_rightClickTab = -1;

//            for (int i = 0; i < count(); i++) {
//                if (tabRect(i).contains(position)) {
//                    m_rightClickTab = i;
//                    break;
//                }
//            }

            // popup right menu on tab.
            if (m_rightClickTab >= 0) {
                m_rightMenu = new DMenu;
                //m_rightMenu->setStyle(QStyleFactory::create("dlight"));

                m_closeTabAction = new QAction(tr("Close tab"), this);
                m_closeOtherTabAction = new QAction(tr("Close other tabs"), this);
                m_moreWaysCloseMenu = new DMenu(tr("More options"), this);
                m_closeLeftTabAction = new QAction(tr("Close tabs to the left"), this);
                m_closeRightTabAction = new QAction(tr("Close tabs to the right"), this);
                m_closeAllunModifiedTabAction = new QAction(tr("Close unmodified tabs"), this);

                m_moreWaysCloseMenu->addAction(m_closeLeftTabAction);
                m_moreWaysCloseMenu->addAction(m_closeRightTabAction);
                m_moreWaysCloseMenu->addAction(m_closeAllunModifiedTabAction);

                if (m_tabPaths.length() < 2) {
                    m_closeOtherTabAction->setEnabled(false);
                    m_closeLeftTabAction->setEnabled(false);
                    m_closeRightTabAction->setEnabled(false);
                }

                //优化tab菜单显示　梁卫东
                if (m_rightClickTab == 0 && indexCount == 1) {
                    m_closeLeftTabAction->setEnabled(false);
                    m_closeRightTabAction->setEnabled(false);
                } else if (m_rightClickTab == 0 && indexCount > 1) {
                    m_closeLeftTabAction->setEnabled(false);
                    m_closeRightTabAction->setEnabled(true);
                } else if (m_rightClickTab == indexCount - 1 && indexCount == 1) {
                    m_closeLeftTabAction->setEnabled(false);
                    m_closeRightTabAction->setEnabled(false);
                } else if (m_rightClickTab == indexCount - 1 && indexCount > 1) {
                    m_closeLeftTabAction->setEnabled(true);
                    m_closeRightTabAction->setEnabled(false);
                } else {
                    m_closeLeftTabAction->setEnabled(true);
                    m_closeRightTabAction->setEnabled(true);
                }

                //showTabs();

                connect(m_closeTabAction, &QAction::triggered, this, [ = ] {
                    Q_EMIT tabCloseRequested(m_rightClickTab);
                });

                connect(m_closeOtherTabAction, &QAction::triggered, this, [ = ] {
                    closeOtherTabsExceptFile(fileAt(m_rightClickTab));
                });

                connect(m_closeLeftTabAction, &QAction::triggered, this, [ = ] {
                    closeLeftTabs(fileAt(m_rightClickTab));
//                    int currentIndex = DTabBar::currentIndex();
//                    if(currentIndex>=0) {
//                        for (int i=0;i<currentIndex;i++) {
//                            closeTab(0);
//                        }
//                    }
                });

                connect(m_closeRightTabAction, &QAction::triggered, this, [ = ] {
                    closeRightTabs(fileAt(m_rightClickTab));
//                    int currentIndex = DTabBar::currentIndex();
//                    int count = DTabBar::count();
//                    if(currentIndex>=0){
//                        for (int i=count;i>currentIndex;i--) {
//                            closeTab(currentIndex+1);
//                        }
//                    }
                });

                connect(m_closeAllunModifiedTabAction, &QAction::triggered, this, [ = ] {
                    Window *window = static_cast<Window *>(this->window());//确定在哪个窗口关闭
//                    for (int i=0;i<m_tabPaths.count();i++) {
//                        EditWrapper *wrapper = window->wrapper(m_tabPaths.value(i));
//                        if (!wrapper->textEditor()->document()->isModified()){
//                            closeTab(this->indexOf(m_tabPaths.value(i)));
//                        }
//                    }
                    for (auto path : m_tabPaths)
                    {
                        EditWrapper *wrapper = window->wrapper(path);//路径获取文件
                        if (!wrapper->isModified()) {
                            window->removeWrapper(path, true);
                            closeTab(this->indexOf(path));
                        }
                    }

                });

                m_rightMenu->addAction(m_closeTabAction);
                m_rightMenu->addAction(m_closeOtherTabAction);
                m_rightMenu->addMenu(m_moreWaysCloseMenu);
                //yanyuhan 只有一个标签页时不显示更多方式关闭
                if (m_tabPaths.size() > 1) {
                    m_moreWaysCloseMenu->setEnabled(true);
                } else {
                    m_moreWaysCloseMenu->setEnabled(false);
                }

                m_rightMenu->exec(mapToGlobal(position));

                return true;
            }
        }

    } else if (event->type() == QEvent::DragEnter) {

//        if ((!e->source() || e->source()->parent() != this) &&
//            mimeData->hasFormat("dedit/tabbar")) {
//            static_cast<Window*>(this->window())->changeTitlebarBackground(m_dndStartColor, m_dndEndColor);
//        }
    } else if (event->type() == QEvent::DragLeave) {
    } else if (event->type() == QEvent::Drop) {
    } else if (event->type() == QEvent::DragMove) {
        event->accept();
    }

    return false;
}

void Tabbar::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::MidButton) {
        emit tabCloseRequested(tabAt(QPoint(e->x(), e->y())));
    }

    DTabBar::mousePressEvent(e);
}

void Tabbar::dropEvent(QDropEvent *e)
{
    if (e->dropAction() == Qt::CopyAction && e->mimeData()->hasFormat("dedit/tabbar")) {
        if (sm_pDragPixmap) {
            QPoint cursorPos = QCursor::pos() - QPoint(sm_pDragPixmap->width() / 2, 20);
            DLabel *pLabel = new DLabel();
            pLabel->setWindowFlags(Qt::FramelessWindowHint);
            pLabel->move(cursorPos);
            pLabel->setPixmap(*sm_pDragPixmap);
            pLabel->setMaximumSize(sm_pDragPixmap->size());
            pLabel->show();

            QRect startRect = QRect(cursorPos, sm_pDragPixmap->size());
            QRect endRect =   QRect(QCursor::pos(), QSize(0, 0));
            QPropertyAnimation *geometry = new QPropertyAnimation(pLabel, "geometry");
            connect(geometry, &QPropertyAnimation::finished, pLabel, &DLabel::deleteLater);
            connect(geometry, &QPropertyAnimation::finished, geometry, &QPropertyAnimation::deleteLater);
            geometry->setDuration(100);
            geometry->setStartValue(startRect);
            geometry->setEndValue(endRect);
            geometry->setEasingCurve(QEasingCurve::InCubic);
            geometry->start();
        }
    }

    DTabBar::dropEvent(e);
}

QSize Tabbar::tabSizeHint(int index) const
{
    if (index >= 0) {
        int total = this->width();

        //计算每个tab平均宽度 返回　100到160
        int aveargeWidth = 160;
        aveargeWidth = total / DTabBar::count();

        if (aveargeWidth >= 160) {
            aveargeWidth = 160;
        } else if (aveargeWidth <= 110) {
            aveargeWidth = 110;
        }

#ifdef DTKWIDGET_CLASS_DSizeMode
        return QSize(aveargeWidth, DGuiApplicationHelper::isCompactMode() ? s_TabbarHeightCompact : s_TabbarHeight);
#else
        return QSize(aveargeWidth, 40);
#endif
    }

    return DTabBar::tabSizeHint(index);
}

QSize Tabbar::minimumTabSizeHint(int index) const
{
    Q_UNUSED(index)
#ifdef DTKWIDGET_CLASS_DSizeMode
    return QSize(110, DGuiApplicationHelper::isCompactMode() ? s_TabbarHeightCompact : s_TabbarHeight);
#else
    return QSize(110, 40);
#endif
}

QSize Tabbar::maximumTabSizeHint(int index) const
{
    Q_UNUSED(index)
#ifdef DTKWIDGET_CLASS_DSizeMode
    return QSize(160, DGuiApplicationHelper::isCompactMode() ? s_TabbarHeightCompact : s_TabbarHeight);
#else
    return QSize(160, 40);
#endif
}

void Tabbar::handleTabMoved(int fromIndex, int toIndex)
{
    //qDebug () << "handleTabMoved";
    if (m_tabPaths.count() > fromIndex && m_tabPaths.count() > toIndex && fromIndex >= 0 && toIndex >= 0) {
        m_tabPaths.swap(fromIndex, toIndex);
        m_tabTruePaths.swap(fromIndex, toIndex);
    }
}

void Tabbar::showTabs()
{
    int currentIndex  =  DTabBar::currentIndex();
    if (currentIndex <= 0)  {
        m_closeLeftTabAction->setEnabled(false);
    }
    if (currentIndex >= DTabBar::count() - 1) {
        m_closeRightTabAction->setEnabled(false);
    }
}

void Tabbar::handleTabReleased(int index)
{
    if (index == -1) index = 0;
    QString path = m_listOldTabPath.value(index);
    if (path.isEmpty()) {
        return;
    }

    int newIndex = m_tabPaths.indexOf(path);
    const QString tabPath = fileAt(newIndex);
    const QString tabName = textAt(newIndex);

    Window *window = static_cast<Window *>(this->window());
    EditWrapper *wrapper = window->wrapper(tabPath);
    if (!wrapper) {
        return;
    }

    StartManager::instance()->createWindowFromWrapper(tabName, tabPath, wrapper->textEditor()->getTruePath(), wrapper, wrapper->isModified());

    closeTab(newIndex);
    // remove wrapper from window, not delete.
    window->removeWrapper(tabPath, false);
}

void Tabbar::handleTabIsRemoved(int index)
{
    //qDebug() << "handleTabIsRemoved" << index;
    const QString filePath = m_tabPaths.at(index);
    Window *window = static_cast<Window *>(this->window());
    m_tabPaths.removeAt(index);
    m_tabTruePaths.removeAt(index);
    window->removeWrapper(filePath, false);
}

void Tabbar::handleTabDroped(int index, Qt::DropAction action, QObject *target)
{
    Tabbar *tabbar = qobject_cast<Tabbar *>(target);
    if (tabbar == nullptr) {
        // tab页拖动到外部应用如网页电子表格或wps电子表格时,
        // DTabBar::dragActionChanged 信号收到的DropType为MoveAction,
        // 这种情况下DTabBar内容不能发出DTabBar::tabReleaseRequested来重新构建编辑窗口
        // 因此只能再次添加判断，若目标文本编辑窗口的TabBar未创建，则重新重建文本编辑窗口
        handleTabReleased(index);
    } else {
//        QString path = m_listOldTabPath.value(index);
//        int newIndex = m_tabPaths.indexOf(path);

        closeTab(index);
//        StartManager::instance()->setDragEnter(false);
    }
}

void Tabbar::onTabDrapStart()
{
    Window *window = static_cast<Window *>(this->window());
    window->setChildrenFocus(false);
    m_listOldTabPath = m_tabPaths;
}
