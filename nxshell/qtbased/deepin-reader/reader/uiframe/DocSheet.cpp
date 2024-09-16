// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DocSheet.h"
#include "SheetSidebar.h"
#include "SheetBrowser.h"
#include "Database.h"
#include "CentralDocPage.h"
#include "MsgHeader.h"
#include "EncryptionPage.h"
#include "PDFModel.h"
#include "FileAttrWidget.h"
#include "Application.h"
#include "Utils.h"
#include "SheetRenderer.h"
#include "PageRenderThread.h"
#include "PageSearchThread.h"
#include "BrowserPage.h"
#include "MainWindow.h"

#include <DSpinner>
#include <DPrintPreviewDialog>
#include <DWidgetUtil>

#include <QJsonDocument>
#include <QJsonObject>
#include <QStackedWidget>
#include <QMimeData>
#include <QUuid>
#include <QClipboard>
#include <QFileInfo>
#include <QPropertyAnimation>
#include <QDesktopWidget>
#include <QDebug>
#include <QTemporaryDir>
#include <QProcess>
#include <QGraphicsDropShadowEffect>

#include <signal.h>
#include <sys/types.h>
extern "C" {
#include "load_libs.h"
}
DWIDGET_USE_NAMESPACE

QReadWriteLock DocSheet::g_lock;
QStringList DocSheet::g_uuidList;
QList<DocSheet *> DocSheet::g_sheetList;
QString DocSheet::g_lastOperationFile;
DocSheet::DocSheet(const Dr::FileType &fileType, const QString &filePath,  QWidget *parent)
    : DSplitter(parent), m_filePath(filePath), m_fileType(fileType)
{
    setAlive(true);
    setHandleWidth(5);
    setChildrenCollapsible(false);  //  子部件不可拉伸到 0

    m_searchTask = new PageSearchThread(this);
    qRegisterMetaType<deepin_reader::SearchResult>("deepin_reader::SearchResult");
    connect(m_searchTask, &PageSearchThread::sigSearchReady, this, &DocSheet::onSearchResultComming, Qt::QueuedConnection);
    connect(m_searchTask, &PageSearchThread::finished, this, &DocSheet::onSearchFinished, Qt::QueuedConnection);
    connect(m_searchTask, &PageSearchThread::sigSearchResultNotEmpty, this, &DocSheet::onSearchResultNotEmpty, Qt::QueuedConnection);

    m_renderer = new SheetRenderer(this);
    connect(m_renderer, &SheetRenderer::sigOpened, this, &DocSheet::onOpened);

    m_browser = new SheetBrowser(this);
    m_browser->setMinimumWidth(481);

    if (Dr::PDF == fileType)
        m_sidebar = new SheetSidebar(this, PREVIEW_THUMBNAIL | PREVIEW_CATALOG | PREVIEW_BOOKMARK | PREVIEW_NOTE);
    else if (Dr::DJVU == fileType)
        m_sidebar = new SheetSidebar(this, PREVIEW_THUMBNAIL | PREVIEW_BOOKMARK);
    else if (Dr::DOCX == fileType)
        m_sidebar = new SheetSidebar(this, PREVIEW_THUMBNAIL | PREVIEW_CATALOG | PREVIEW_BOOKMARK | PREVIEW_NOTE);
    else
        m_sidebar = new SheetSidebar(this, nullptr);

    m_sidebar->setMinimumWidth(266);

    connect(m_browser, SIGNAL(sigPageChanged(int)), this, SLOT(onBrowserPageChanged(int)));
    connect(m_browser, SIGNAL(sigNeedPagePrev()), this, SLOT(onBrowserPagePrev()));
    connect(m_browser, SIGNAL(sigNeedPageNext()), this, SLOT(onBrowserPageNext()));
    connect(m_browser, SIGNAL(sigNeedPageFirst()), this, SLOT(onBrowserPageFirst()));
    connect(m_browser, SIGNAL(sigNeedPageLast()), this, SLOT(onBrowserPageLast()));
    connect(m_browser, SIGNAL(sigNeedBookMark(int, bool)), this, SLOT(onBrowserBookmark(int, bool)));
    connect(m_browser, SIGNAL(sigOperaAnnotation(int, int, deepin_reader::Annotation *)), this, SLOT(onBrowserOperaAnnotation(int, int, deepin_reader::Annotation *)));

    resetChildParent();
    this->insertWidget(0, m_browser);
    this->insertWidget(0, m_sidebar);
}

DocSheet::~DocSheet()
{
    qDebug() << "正在释放当前 sheet ...";
    // 结束正在进行的命令process
    if (nullptr != m_process) {
        __pid_t processid = static_cast<__pid_t>(m_process->processId());
        if (0 < processid) {
            kill(processid, SIGKILL);
        }
    }

    setAlive(false);

    delete m_browser;

    delete m_sidebar;

    delete m_renderer;

    delete m_searchTask;

    delete m_encryPage;

    delete m_sideAnimation;

    if (nullptr != m_tempDir)
        delete m_tempDir;
    qDebug() << "当前 sheet 已释放";
}

QImage DocSheet::firstThumbnail(const QString &filePath)
{
    //获取首页缩略图
    foreach (DocSheet *sheet, g_sheetList) {
        if (sheet->filePath() == filePath) {
            QImage image = sheet->getImage(0, 256, 256);
            return image;
        }
    }

    return SheetBrowser::firstThumbnail(filePath);
}

bool DocSheet::existFileChanged()
{
    bool changed = false;

    g_lock.lockForRead();

    foreach (DocSheet *sheet, g_sheetList) {
        if (sheet->fileChanged()) {
            changed = true;
            break;
        }
    }

    g_lock.unlock();

    return changed;
}

QUuid DocSheet::getUuid(DocSheet *sheet)
{
    g_lock.lockForRead();

    QUuid uuid = g_uuidList.value(g_sheetList.indexOf(sheet));

    g_lock.unlock();

    return uuid;
}

bool DocSheet::existSheet(DocSheet *sheet)
{
    g_lock.lockForRead();

    bool result = g_sheetList.contains(sheet);

    g_lock.unlock();

    return result;
}

DocSheet *DocSheet::getSheet(QString uuid)
{
    g_lock.lockForRead();

    DocSheet *sheet = g_sheetList.value(g_uuidList.indexOf(uuid));

    g_lock.unlock();

    return sheet;
}

DocSheet *DocSheet::getSheetByFilePath(QString filePath)
{
    g_lock.lockForRead();

    DocSheet *result = nullptr;

    foreach (DocSheet *sheet, g_sheetList) {
        if (sheet->filePath() == filePath) {
            result = sheet;
            break;
        }
    }

    g_lock.unlock();

    return result;
}

QList<DocSheet *> DocSheet::getSheets()
{
    return DocSheet::g_sheetList;
}

bool DocSheet::openFileExec(const QString &password)
{
    m_password = password;

    return m_renderer->openFileExec(password);
}

void DocSheet::openFileAsync(const QString &password)
{
    m_password = password;

    qInfo() << "添加异步打开任务...";
    m_renderer->openFileAsync(m_password);
}

void DocSheet::jumpToPage(int page)
{
    m_browser->setCurrentPage(page);
}

void DocSheet::jumpToIndex(int index)
{
    m_browser->setCurrentPage(index + 1);
}

void DocSheet::jumpToFirstPage()
{
    m_browser->setCurrentPage(1);
}

void DocSheet::jumpToLastPage()
{
    jumpToPage(m_browser->allPages());
}

void DocSheet::jumpToNextPage()
{
    int page = m_browser->currentPage() + (m_operation.layoutMode == Dr::TwoPagesMode ? 2 : 1);

    page = page >= m_browser->allPages() ? m_browser->allPages() : page;

    jumpToPage(page);
}

void DocSheet::jumpToPrevPage()
{
    int page = m_browser->currentPage() - (m_operation.layoutMode == Dr::TwoPagesMode ? 2 : 1);

    page = qMax(1, page);

    jumpToPage(page);
}

deepin_reader::Outline DocSheet::outline()
{
    return m_renderer->outline();
}

void DocSheet::jumpToOutline(const qreal  &left, const qreal &top, int index)
{
    m_browser->jumpToOutline(left, top, index);
}

void DocSheet::jumpToHighLight(deepin_reader::Annotation *annotation, const int index)
{
    m_browser->jumpToHighLight(annotation, index);
}

void DocSheet::rotateLeft()
{
    if (Dr::RotateBy0 == m_operation.rotation)
        m_operation.rotation = Dr::RotateBy270;
    else if (Dr::RotateBy270 == m_operation.rotation)
        m_operation.rotation = Dr::RotateBy180;
    else if (Dr::RotateBy180 == m_operation.rotation)
        m_operation.rotation = Dr::RotateBy90;
    else if (Dr::RotateBy90 == m_operation.rotation)
        m_operation.rotation = Dr::RotateBy0;

    m_browser->deform(m_operation);
    m_sidebar->handleRotate();
    setOperationChanged();
}

void DocSheet::rotateRight()
{
    if (Dr::RotateBy0 == m_operation.rotation)
        m_operation.rotation = Dr::RotateBy90;
    else if (Dr::RotateBy90 == m_operation.rotation)
        m_operation.rotation = Dr::RotateBy180;
    else if (Dr::RotateBy180 == m_operation.rotation)
        m_operation.rotation = Dr::RotateBy270;
    else if (Dr::RotateBy270 == m_operation.rotation)
        m_operation.rotation = Dr::RotateBy0;

    m_browser->deform(m_operation);
    m_sidebar->handleRotate();
    setOperationChanged();
}

void DocSheet::setBookMark(int index, int state)
{
    if (index < 0 || index >= pageCount())
        return;

    if (state)
        m_bookmarks.insert(index);
    else {
        showTips(tr("The bookmark has been removed"));
        m_bookmarks.remove(index);
    }

    if (m_sidebar)
        m_sidebar->setBookMark(index, state);

    m_browser->setBookMark(index, state);

    setBookmarkChanged(true);
}

void DocSheet::setBookMarks(const QList<int> &indexlst, int state)
{
    for (int index : indexlst) {
        if (state)
            m_bookmarks.insert(index);
        else {
            m_bookmarks.remove(index);
        }

        if (m_sidebar)
            m_sidebar->setBookMark(index, state);

        m_browser->setBookMark(index, state);
    }

    if (!state)
        showTips(tr("The bookmark has been removed"));

    setBookmarkChanged(true);
}

int DocSheet::pageCount()
{
    return m_renderer->getPageCount();
}

int DocSheet::currentPage()
{
    if (m_operation.currentPage < 1 || m_operation.currentPage > pageCount())
        return 1;

    return m_operation.currentPage;
}

int DocSheet::currentIndex()
{
    if (m_operation.currentPage < 1 || m_operation.currentPage > pageCount())
        return 0;

    return m_operation.currentPage - 1;
}

void DocSheet::setLayoutMode(Dr::LayoutMode mode)
{
    if (mode == m_operation.layoutMode)
        return;

    if (mode >= Dr::SinglePageMode && mode < Dr::NumberOfLayoutModes) {
        m_operation.layoutMode = mode;
        m_browser->deform(m_operation);
        setOperationChanged();
    }
}

void DocSheet::setMouseShape(Dr::MouseShape shape)
{
    if (shape >= Dr::MouseShapeNormal && shape < Dr::NumberOfMouseShapes) {
        closeMagnifier();
        m_operation.mouseShape = shape;
        m_browser->setMouseShape(m_operation.mouseShape);
        setOperationChanged();
    }
}

void DocSheet::setAnnotationInserting(bool inserting)
{
    if (nullptr == m_browser)
        return;

    m_browser->setAnnotationInserting(inserting);
}

QPixmap DocSheet::thumbnail(int index)
{
    return m_thumbnailMap.value(index);
}

void DocSheet::setThumbnail(int index, QPixmap pixmap)
{
    m_thumbnailMap[index] = pixmap;
}

void DocSheet::setScaleMode(Dr::ScaleMode mode)
{
    if (mode >= Dr::ScaleFactorMode && mode <= Dr::FitToPageWorHMode) {
        m_operation.scaleMode = mode;

        m_browser->deform(m_operation);

        setOperationChanged();

    }
}

void DocSheet::setScaleFactor(qreal scaleFactor)
{
    if (Dr::ScaleFactorMode == m_operation.scaleMode && qFuzzyCompare(scaleFactor, m_operation.scaleFactor))
        return;

    scaleFactor = qBound(0.1, scaleFactor, maxScaleFactor());

    m_operation.scaleMode = Dr::ScaleFactorMode;

    m_operation.scaleFactor = scaleFactor;

    m_browser->deform(m_operation);

    setOperationChanged();
}

QImage DocSheet::getImage(int index, int width, int height, const QRect &slice)
{
    return m_renderer->getImage(index, width, height, slice);;
}

bool DocSheet::fileChanged()
{
    return (m_documentChanged || m_bookmarkChanged);
}

bool DocSheet::saveData()
{
    PERF_PRINT_BEGIN("POINT-04", QString("filename=%1,filesize=%2").arg(QFileInfo(this->filePath()).fileName()).arg(QFileInfo(this->filePath()).size()));

    //文档改变或者原文档被删除 则进行数据保存
    if ((m_documentChanged || !QFile(m_filePath).exists()) && !m_renderer->save())
        return false;

    m_documentChanged = false;

    if (m_bookmarkChanged && !Database::instance()->saveBookmarks(filePath(), m_bookmarks))
        return false;

    m_bookmarkChanged = false;

    m_sidebar->changeResetModelData();

    return true;
}

bool DocSheet::saveAsData(QString targetFilePath)
{
    stopSearch();

    if (m_documentChanged && Dr::DOCX != fileType()) {
        if (!m_renderer->saveAs(targetFilePath))
            return false;
    } else {
        //如果是需要转换的格式，则先转换再拷贝
        QString saveAsSourceFilePath = openedFilePath();

        if (m_documentChanged) {
            saveAsSourceFilePath = convertedFileDir() + "/saveAsTemp.pdf";
            if (!m_renderer->saveAs(saveAsSourceFilePath))
                return false;
        }

        if (!Utils::copyFile(saveAsSourceFilePath, targetFilePath))
            return false;
    }

    Database::instance()->saveBookmarks(targetFilePath, m_bookmarks);

    m_sidebar->changeResetModelData();

    return true;
}

void DocSheet::handlePageModified(int index)
{
    emit sigPageModified(index);
}

void DocSheet::copySelectedText()
{
    QString selectedWordsText = m_browser->selectedWordsText();
    if (selectedWordsText.isEmpty())
        return;
#if _ZPD_
    int intercept = -1;
    if (getLoadLibsInstance()->m_document_clip_copy) {
        qInfo() << "当前文档: ***"/* << filePath()*/;
        getLoadLibsInstance()->m_document_clip_copy(filePath().toLocal8Bit().data(), &intercept);
        qInfo() << "是否拦截不允许复制(1:拦截 0:不拦截): " << intercept;
    }
    if (intercept) return;
#endif
    QClipboard *clipboard = DApplication::clipboard();  //获取系统剪贴板指针
    clipboard->setText(selectedWordsText);
}

void DocSheet::highlightSelectedText()
{
    if (!m_browser || (fileType() != Dr::FileType::PDF && fileType() != Dr::FileType::DOCX))
        return;

    if (m_browser->selectedWordsText().isEmpty()) {
        showTips(tr("Please select the text"), 1);
        return;
    }

    QPoint ponintend;

    m_browser->addHighLightAnnotation("", Utils::getCurHiglightColor(), ponintend);
}

void DocSheet::addSelectedTextHightlightAnnotation()
{
    if (!m_browser || (fileType() != Dr::FileType::PDF && fileType() != Dr::FileType::DOCX))
        return;

    if (m_browser->selectedWordsText().isEmpty()) {
        showTips(tr("Please select the text"), 1);
        return;
    }

    //进行高亮编辑
    QPoint ponintend;

    Annotation *annot = m_browser->addHighLightAnnotation("", Utils::getCurHiglightColor(), ponintend);

    if (annot)
        m_browser->showNoteEditWidget(annot, ponintend);
}

void DocSheet::openMagnifier()
{
    if (m_browser)
        m_browser->openMagnifier();
}

void DocSheet::closeMagnifier()
{
    if (m_browser)
        m_browser->closeMagnifier();
}

void DocSheet::defaultFocus()
{
    if (m_browser)
        m_browser->setFocus();
}

bool DocSheet::magnifierOpened()
{
    if (m_browser)
        return m_browser->magnifierOpened();
    return false;
}

QList<deepin_reader::Annotation *> DocSheet::annotations()
{
    if (nullptr == m_browser)
        return QList< deepin_reader::Annotation * > ();

    return m_browser->annotations();
}

bool DocSheet::removeAnnotation(deepin_reader::Annotation *annotation, bool tips)
{
    int ret = m_browser->removeAnnotation(annotation);
    if (ret && tips) {
        this->showTips(tr("The annotation has been removed"));
    }
    return ret;
}

bool DocSheet::removeAllAnnotation()
{
    bool ret = m_browser->removeAllAnnotation();
    if (ret) {
        this->showTips(tr("The annotation has been removed"));
    }
    return ret;
}

QList<qreal> DocSheet::scaleFactorList()
{
    QList<qreal> dataList = {0.1, 0.25, 0.5, 0.75, 1, 1.25, 1.5, 1.75, 2, 3, 4, 5};
    QList<qreal> factorList;

    qreal maxFactor = maxScaleFactor();

    foreach (qreal factor, dataList) {
        if (maxFactor - factor > -0.0001)
            factorList.append(factor);
    }

    return  factorList;
}

qreal DocSheet::maxScaleFactor()
{
    qreal maxScaleFactor = 20000 / (m_browser->maxHeight() * qApp->devicePixelRatio());

    maxScaleFactor = qBound(0.1, maxScaleFactor, 5.0);

    return maxScaleFactor;
}

QString DocSheet::filter()
{
    if (Dr::PDF == m_fileType)
        return  "Pdf File (*.pdf)";
    else if (Dr::DOCX == m_fileType)
        return  "Pdf File (*.pdf)";
    else if (Dr::DJVU == m_fileType)
        return "Djvu files (*.djvu)";

    return "";
}

QString DocSheet::format()
{
    if (Dr::PDF == m_fileType) {
        const Properties &propertys = m_renderer->properties();
        return QString("PDF %1").arg(propertys.value("Version").toString());
    } else if (Dr::DOCX == m_fileType) {//暂时作为pdf处理
        const Properties &propertys = m_renderer->properties();
        return QString("PDF %1").arg(propertys.value("Version").toString());
    } else if (Dr::DJVU == m_fileType) {
        return QString("DJVU");
    }
    return "";
}

QSet<int> DocSheet::getBookMarkList() const
{
    return m_bookmarks;
}

SheetOperation DocSheet::operation() const
{
    return m_operation;
}

SheetOperation &DocSheet::operationRef()
{
    return m_operation;
}

Dr::FileType DocSheet::fileType()
{
    return m_fileType;
}

QString DocSheet::filePath()
{
    return m_filePath;
}

QString DocSheet::openedFilePath()
{
    if (Dr::DOCX == fileType())
        return convertedFileDir() + "/temp.pdf";

    return filePath();
}

QString DocSheet::convertedFileDir()
{
    if (m_tempDir == nullptr)
        m_tempDir = new QTemporaryDir;

    qDebug() << "临时目录: " << m_tempDir->path();
    return m_tempDir->path();
}

bool DocSheet::hasBookMark(int index)
{
    return m_bookmarks.contains(index);
}

void DocSheet::zoomin()
{
    QList<qreal> dataList = scaleFactorList();

    for (int i = 0; i < dataList.count(); ++i) {
        if (dataList[i] > (m_operation.scaleFactor)) {
            setScaleFactor(dataList[i]);
            return;
        }
    }
}

void DocSheet::zoomout()
{
    QList<qreal> dataList = scaleFactorList();

    for (int i = dataList.count() - 1; i >= 0; --i) {
        if (dataList[i] < (m_operation.scaleFactor)) {
            setScaleFactor(dataList[i]);
            return;
        }
    }
}

void DocSheet::showTips(const QString &tips, int iconIndex)
{
    CentralDocPage *doc = static_cast<CentralDocPage *>(parent());
    if (nullptr == doc)
        return;

    doc->showTips(this, tips, iconIndex);
}

void DocSheet::onPrintRequested(DPrinter *printer, const QVector<int> &pageRange)
{
    if (pageRange.isEmpty())
        return;

    //后台加载动画
    QWidget *pCurWgt = nullptr;
    for (QWidget *pwgt : qApp->topLevelWidgets()) {
        if (QMainWindow *mwd = dynamic_cast<QMainWindow *>(pwgt)) {
            pCurWgt = mwd;
        }
    }
    LoadingWidget loading(pCurWgt);
    loading.show();

    printer->setDocName(QFileInfo(filePath()).fileName());

    QPainter painter(printer);

    const QRectF pageRect = printer->pageRect(QPrinter::DevicePixel); //打印纸张类型页面大小

    painter.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);

    for (int i = 0; i < pageRange.count(); ++i) {
        if (pageRange[i] > pageCount() || pageRange[i] > m_browser->pages().count())
            continue;

        const QRectF boundingrect = m_browser->pages().at(pageRange[i] - 1)->boundingRect(); //文档页缩放后的原区域不受旋转影响
        qreal printWidth = pageRect.width(); //适合打印的图片宽度
        qreal printHeight = printWidth * boundingrect.height() / boundingrect.width(); //适合打印的图片高度
        if (printHeight > pageRect.height()) {
            printHeight = pageRect.height();
            printWidth = printHeight * boundingrect.width() / boundingrect.height();
        }

        QImage image = loading.getImage(this, pageRange[i] - 1, static_cast<int>(printWidth), static_cast<int>(printHeight));
        painter.drawImage(QRect((static_cast<int>(pageRect.width()) - image.width()) / 2,
                                (static_cast<int>(pageRect.height()) - image.height()) / 2,
                                image.width(), image.height()), image);

        if (i != pageRange.count() - 1)
            printer->newPage();
    }
}

void DocSheet::onPrintRequested(DPrinter *printer)
{
    printer->setDocName(QFileInfo(filePath()).fileName());

    QPainter painter(printer);

    const QRectF pageRect = printer->pageRect(QPrinter::DevicePixel);

    painter.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);

    int pagesCount = pageCount();

    int fromIndex = qMax(0, printer->fromPage() - 1);

    int toIndex = printer->toPage() <= 0 ? pagesCount - 1 : printer->toPage() - 1;

#if 0
    for (int index = fromIndex; index <= toIndex; index++) {
        if (index >= pagesCount)
            break;

        QImage image;
        // 有缓存图片时使用缓存图片，没有的话需要获取对应页的图片
        if (!m_browser->getExistImage(index, image, static_cast<int>(pageRect.width()), static_cast<int>(pageRect.height()))) {
            QSizeF rect = pageSizeByIndex(index); //根据index获取对应PAGE真实大小
            qreal aspectRatio = rect.height() / rect.width(); //原始图片的高宽比

            int tmpWidth = static_cast<int>(pageRect.width());
            int tmpHeight = static_cast<int>(tmpWidth * aspectRatio);
            if (tmpHeight > static_cast<int>(pageRect.height())) { // 使打印的图片保持原始图片的宽高比
                tmpHeight = static_cast<int>(pageRect.height());
                tmpWidth = static_cast<int>(tmpHeight / aspectRatio);
            }

            image = getImage(index, tmpWidth, tmpHeight);
        }

        if (!image.isNull()) {
            painter.drawImage(QRect((static_cast<int>(pageRect.width()) - image.width()) / 2,
                                    (static_cast<int>(pageRect.height()) - image.height()) / 2,
                                    image.width(), image.height()), image);
        }

        if (index != toIndex)
            printer->newPage();
    }

#else
    //后台加载动画
    LoadingWidget loading(qApp->activeWindow());
    loading.show();

    for (int index = fromIndex; index <= toIndex; index++) {
        if (index >= pagesCount)
            break;

        QImage imageX3 = loading.getImage(this, index, int(pageRect.width() * 3), int(pageRect.height() * 3));

        painter.drawImage(pageRect, imageX3, QRect(0, 0, imageX3.width(), imageX3.height()));
        if (index != toIndex)
            printer->newPage();
    }
#endif
}

void DocSheet::openSlide()
{
    CentralDocPage *doc = static_cast<CentralDocPage *>(parent());
    if (nullptr == doc)
        return;

    doc->openSlide();
}

void DocSheet::closeSlide()
{
    CentralDocPage *doc = static_cast<CentralDocPage *>(parent());
    if (nullptr == doc)
        return;

    doc->quitSlide();
}

void DocSheet::setSidebarVisible(bool isVisible, bool notify)
{
    if (notify) {
        //左侧栏是否需要隐藏
        m_sidebar->setVisible(isVisible);
        m_operation.sidebarVisible = isVisible;

        if (isVisible) {
            this->insertWidget(0, m_sidebar);
        } else if (isFullScreen()) {
            resetChildParent();
            this->insertWidget(0, m_browser);

            m_sidebar->resize(m_sidebar->width(), dApp->primaryScreen()->size().height());
            m_sidebar->move(-m_sidebar->width(), 0);
            m_sidebar->setVisible(false);
        }

        setOperationChanged();
    } else {
        if (m_sideAnimation == nullptr) {
            m_sideAnimation = new QPropertyAnimation(m_sidebar, "movepos");
            connect(m_sideAnimation, &QPropertyAnimation::finished, this, &DocSheet::onSideAniFinished);
        }

        if (m_sideAnimation->state() == QPropertyAnimation::Running && m_fullSiderBarVisible == isVisible)
            return;

        if (isFullScreen() && this->indexOf(m_sidebar) >= 0) {
            resetChildParent();
            this->insertWidget(0, m_browser);

            m_sidebar->resize(m_sidebar->width(), dApp->primaryScreen()->size().height());
            m_sidebar->move(-m_sidebar->width(), 0);
            m_sidebar->setVisible(false);
        }

        m_sideAnimation->setEasingCurve(QEasingCurve::OutCubic);
        m_fullSiderBarVisible = isVisible;
        m_sideAnimation->stop();
        int duration = 300 * (m_sidebar->width() + m_sidebar->pos().x()) / m_sidebar->width();
        duration = duration <= 0 ? 300 : duration;
        m_sideAnimation->setDuration(duration);
        m_sideAnimation->setStartValue(QPoint(m_sidebar->pos().x(), 0));
        if (isVisible) {
            m_sidebar->setVisible(true);
            m_sideAnimation->setEndValue(QPoint(0, 0));
        } else {
            m_sideAnimation->setEndValue(QPoint(-m_sidebar->width(), 0));
        }
        m_sideAnimation->start();
    }
}

void DocSheet::onSideAniFinished()
{
    if (m_sidebar->pos().x() < 0) {
        m_sidebar->setVisible(false);
    }
}

void DocSheet::onOpened(deepin_reader::Document::Error error)
{
    if (deepin_reader::Document::NeedPassword == error) {
        showEncryPage();
    } else if (deepin_reader::Document::WrongPassword == error) {
        showEncryPage();

        m_encryPage->wrongPassWordSlot();
    } else if (deepin_reader::Document::NoError == error) {
        if (!m_password.isEmpty()) {
            m_browser->setFocusPolicy(Qt::StrongFocus);

            if (m_encryPage) {
                m_encryPage->hide();
                m_encryPage->deleteLater();
            }
            m_encryPage = nullptr;

            this->defaultFocus();
        }

        m_browser->init(m_operation, m_bookmarks);

        m_sidebar->handleOpenSuccess();

        emit sigOperationChanged(this);

        emit sigFileChanged(this);
    }

    //交给父窗口控制自己是否删除
    emit sigFileOpened(this, error);
}

bool DocSheet::isFullScreen()
{
    CentralDocPage *doc = static_cast<CentralDocPage *>(parent());

    if (nullptr == doc)
        return false;

    return doc->isFullScreen();
}

void DocSheet::openFullScreen()
{
    CentralDocPage *doc = static_cast<CentralDocPage *>(parent());
    if (nullptr == doc)
        return;

    setSidebarVisible(false);

    resetChildParent();
    this->insertWidget(0, m_browser);

    m_sidebar->resize(m_sidebar->width(), dApp->primaryScreen()->size().height());
    m_sidebar->move(-m_sidebar->width(), 0);
    m_sidebar->setVisible(false);

    if (m_browser)
        m_browser->hideSubTipsWidget();

    doc->openFullScreen();
}

bool DocSheet::closeFullScreen(bool force)
{
    CentralDocPage *doc = static_cast<CentralDocPage *>(parent());
    if (nullptr == doc)
        return false;

    if (m_browser)
        m_browser->hideSubTipsWidget();

    if (force) {
        this->insertWidget(0, m_sidebar);
        m_sidebar->move(0, 0);
        m_sidebar->setVisible(this->operation().sidebarVisible);

        return doc->quitFullScreen(force);
    } else if (doc->topLevelWidget()->windowState().testFlag(Qt::WindowFullScreen)) {
        MainWindow *mainWindow = qobject_cast<MainWindow *>(doc->topLevelWidget());
        if (nullptr != mainWindow) {
            mainWindow->setWindowState(doc->topLevelWidget()->windowState() & ~Qt::WindowFullScreen);
            mainWindow->handleMainWindowExitFull();
        }

        return true;
    }

    return false;
}

void DocSheet::setDocumentChanged(bool changed)
{
    m_documentChanged = changed;

    emit sigFileChanged(this);
}

void DocSheet::setBookmarkChanged(bool changed)
{
    m_bookmarkChanged = changed;

    emit sigFileChanged(this);
}

void DocSheet::setOperationChanged()
{
    emit sigOperationChanged(this);
}

bool DocSheet::haslabel()
{
    return  m_renderer->pageHasLable();
}

void DocSheet::docBasicInfo(deepin_reader::FileInfo &tFileInfo)
{
    QFileInfo fileInfo(filePath());
    tFileInfo.size = fileInfo.size();
    tFileInfo.createTime = fileInfo.birthTime();
    tFileInfo.changeTime = fileInfo.lastModified();
    tFileInfo.auther = fileInfo.owner();
    tFileInfo.filePath = fileInfo.filePath();

    const Properties &propertys = m_renderer->properties();
    tFileInfo.format = format();
    tFileInfo.optimization = propertys.value("Linearized").toBool();
    tFileInfo.keyword = propertys.value("KeyWords").toString();
    tFileInfo.theme = propertys.value("Title").toString();
    tFileInfo.producter = propertys.value("Producer").toString();
    tFileInfo.creater = propertys.value("Creator").toString();
    tFileInfo.safe = propertys.value("Encrypted").toBool();
    tFileInfo.width = static_cast<unsigned int>(m_browser->maxWidth());
    tFileInfo.height = static_cast<unsigned int>(m_browser->maxHeight());
    tFileInfo.numpages = static_cast<unsigned int>(m_browser->allPages());
}

void DocSheet::onBrowserPageChanged(int page)
{
    if (m_operation.currentPage != page) {
        m_operation.currentPage = page;
        if (m_sidebar)
            m_sidebar->setCurrentPage(page);
    }
}

void DocSheet::onBrowserPageFirst()
{
    jumpToFirstPage();
}

void DocSheet::onBrowserPagePrev()
{
    jumpToPrevPage();
}

void DocSheet::onBrowserPageNext()
{
    jumpToNextPage();
}

void DocSheet::onBrowserPageLast()
{
    jumpToLastPage();
}

void DocSheet::onBrowserBookmark(int index, bool state)
{
    setBookMark(index, state);
}

void DocSheet::onBrowserOperaAnnotation(int type, int index, deepin_reader::Annotation *anno)
{
    m_sidebar->handleAnntationMsg(type, index, anno);
    setDocumentChanged(true);
}

void DocSheet::prepareSearch()
{
    m_browser->handlePrepareSearch();
}

void DocSheet::startSearch(const QString &strFind)
{
    m_browser->handleSearchStart();
    m_sidebar->handleSearchStart(strFind);
    m_searchTask->startSearch(this, strFind);
    emit sigFindOperation(E_FIND_CONTENT);
}

void DocSheet::jumpToNextSearchResult()
{
    //m_sidebar->jumpToNextSearchResult();  //左侧应该同时跳转，目前无此需求
    m_browser->jumpToNextSearchResult();
}

void DocSheet::jumpToPrevSearchResult()
{
    //m_sidebar->jumpToPrevSearchResult();  //左侧应该同时跳转，目前无此需求
    m_browser->jumpToPrevSearchResult();
}

void DocSheet::stopSearch()
{
    m_searchTask->stopSearch();
    m_browser->handleSearchStop();
    m_sidebar->handleSearchStop();
    emit sigFindOperation(E_FIND_EXIT);
}

void DocSheet::onSearchResultComming(const deepin_reader::SearchResult &res)
{
    m_browser->handleSearchResultComming(res);
    m_sidebar->handleSearchResultComming(res);
}

void DocSheet::onSearchFinished()
{
    int count = m_sidebar->handleFindFinished();
    m_browser->handleFindFinished(count);
}

void DocSheet::onSearchResultNotEmpty()
{
    if (nullptr != m_browser) {
        m_browser->setIsSearchResultNotEmpty(true);
    }
}

void DocSheet::resizeEvent(QResizeEvent *event)
{
    DSplitter::resizeEvent(event);
    if (m_encryPage) {
        m_encryPage->setGeometry(0, 0, this->width(), this->height());
    }

    if (isFullScreen()) {
        m_sidebar->resize(m_sidebar->width(), this->height());
    }
}

void DocSheet::childEvent(QChildEvent *event)
{
    if (event->removed()) {
        return DSplitter::childEvent(event);
    }
}

SheetBrowser *DocSheet::getSheetBrowser() const
{
    return m_browser;
}

void DocSheet::setAlive(bool alive)
{
    if (alive) {
        if (!m_uuid.isEmpty())
            setAlive(false);

        m_uuid = QUuid::createUuid().toString();

        g_lock.lockForWrite();

        g_uuidList.append(m_uuid);

        g_sheetList.append(this);

        g_lock.unlock();

        if (Database::instance()->readOperation(this)) {
            qInfo() << "read from database config";
        } else if (readLastFileOperation()) {
            qInfo() << "read from last operation file config";
        } else {
            qInfo() << "read from default config";
        }

        Database::instance()->readBookmarks(m_filePath, m_bookmarks);

    } else {
        if (m_uuid.isEmpty())
            return;

        stopSearch();

        Database::instance()->saveOperation(this);

        g_lock.lockForWrite();

        int index = g_uuidList.indexOf(m_uuid);

        g_sheetList.removeAt(index);

        g_uuidList.removeAt(index);

        m_uuid.clear();

        g_lock.unlock();
    }
}

bool DocSheet::readLastFileOperation()
{
    QString filePath = DocSheet::g_lastOperationFile;
    if (filePath.isEmpty())
        return false;

    DocSheet *sheet = nullptr;
    foreach (sheet, g_sheetList) {
        if (sheet != this && sheet->filePath() == filePath) {
            break;
        }
    }
    if (!sheet) {
        return false;
    }

    qInfo() << __LINE__ << "read config from last operation file: " << filePath;
    m_operation = sheet->operationRef();
    return true;
}

void DocSheet::showEncryPage()
{
    if (m_encryPage == nullptr) {
        m_encryPage = new EncryptionPage(this);
        connect(m_encryPage, &EncryptionPage::sigExtractPassword, this, &DocSheet::onExtractPassword);
        this->stackUnder(m_encryPage);
    }

    //bugid:46645 密码输入框在的时候,先暂时屏蔽掉browser的焦点
    m_browser->setFocusPolicy(Qt::NoFocus);
    m_encryPage->setGeometry(0, 0, this->width(), this->height());
    m_encryPage->raise();
    m_encryPage->show();
}

bool DocSheet::opened()
{
    return m_renderer->opened();
}

int DocSheet::getIndexByPageLable(const QString &pageLable)
{
    return m_renderer->pageLableIndex(pageLable);
}

QString DocSheet::getPageLabelByIndex(const int &index)
{
    return m_renderer->pageNum2Lable(index);
}

void DocSheet::onExtractPassword(const QString &password)
{
    m_password = password;

    m_renderer->openFileAsync(m_password);
}

SheetRenderer *DocSheet::renderer()
{
    return m_renderer;
}

void DocSheet::onPopPrintDialog()
{
    if (!this->opened())
        return;

    DPrintPreviewDialog *preview = new DPrintPreviewDialog(this);
    preview->setAttribute(Qt::WA_DeleteOnClose);

#if (DTK_VERSION >= DTK_VERSION_CHECK(5,4,10,0))
    //文件打印
    preview->setAsynPreview(pageCount());
    preview->setDocName(QFileInfo(filePath()).fileName());
    if (Dr::DOCX == fileType()) {//旧版本和最新版本使用新接口，PDF文件直接传，解决打印模糊问题
        preview->setPrintFromPath(openedFilePath());
    }

    //pdf若是Linearized类型的，需要另存为Normal类型，然后打印
    if (Dr::PDF == fileType()) {
        deepin_reader::Document *document = nullptr;
        deepin_reader::Document::Error error = deepin_reader::Document ::NoError;
        document = DocumentFactory::getDocument(m_fileType,
                                                m_filePath,
                                                convertedFileDir(),
                                                m_password, nullptr, error);

        QString pdfPath = filePath();
        qInfo()  << pdfPath << "isLinearized:" << document->properties().value("Linearized").toBool();
        if (document->properties().value("Linearized").toBool()) {
            pdfPath = QTemporaryDir("LinearizedConverted.pdf").path();
            if (!m_renderer->saveAs(pdfPath)) {
                qInfo() << "saveAs failed when print Linearized pdf";
                return;
            }
        }
        preview->setPrintFromPath(pdfPath);
    }
    connect(preview, QOverload<DPrinter *, const QVector<int> &>::of(&DPrintPreviewDialog::paintRequested), this, QOverload<DPrinter *, const QVector<int> &>::of(&DocSheet::onPrintRequested));
#else
    //图片打印
    connect(preview, QOverload<DPrinter *>::of(&DPrintPreviewDialog::paintRequested), this, QOverload<DPrinter *>::of(&DocSheet::onPrintRequested));
#endif
    preview->open();
}

void DocSheet::onPopInfoDialog()
{
    FileAttrWidget pFileAttrWidget(this);
    pFileAttrWidget.setFileAttr(this);
    pFileAttrWidget.exec();
}

QSizeF DocSheet::pageSizeByIndex(int index)
{
    return m_renderer->getPageSize(index);
}

void DocSheet::resetChildParent()
{
    m_sidebar->setParent(nullptr);
    m_sidebar->setParent(this);

    m_browser->setParent(nullptr);
    m_browser->setParent(this);
}

DocSheet::LoadingWidget::LoadingWidget(QWidget *parent)
    : QWidget(parent)
    , m_parentWidget(parent)
{
    Q_ASSERT(m_parentWidget);
    setGeometry(0, 0, m_parentWidget->width(), m_parentWidget->height());

    DSpinner *spinner = new DSpinner(this);
    spinner->setFixedSize(32, 32);
    moveToCenter(spinner);
    spinner->start();

    m_parentWidget->setEnabled(false);

    raise();
}

DocSheet::LoadingWidget::~LoadingWidget()
{
    m_parentWidget->setEnabled(true);
}

QImage DocSheet::LoadingWidget::getImage(DocSheet *doc, int index, int width, int height)
{
    QImage image;
    QEventLoop loop;
    QThread *thread = QThread::create([ =, &image]() {
        image = doc->getImage(index, width, height);
    });
    QObject::connect(thread, &QThread::finished, &loop, &QEventLoop::quit);
    QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    thread->start();

    loop.exec(QEventLoop::ExcludeSocketNotifiers);

    return image;
}

void DocSheet::LoadingWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    //整体的背景
    int type = DGuiApplicationHelper::instance()->themeType();
    QColor tColor = type == 1 ? "#FFFFFF" : "#000000";
    tColor.setAlphaF(0.3);
    painter.fillRect(this->rect(), tColor);

    //进度条的背景
    QPainterPath path;
    QRect shadowRect((this->width() - 80) / 2, (this->height() - 80) / 2, 80, 80);
    path.addRoundedRect(shadowRect, 18, 18);
    QColor bg = this->palette().color(QPalette::Active, QPalette::Button);
    painter.setBrush(bg);
    painter.setPen(QPen(QColor(0, 0, 0, int(255 * 0.05)), 1));
    painter.drawPath(path);
}
