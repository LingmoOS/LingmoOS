// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QLabel>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QUndoStack>
#include <QGraphicsItem>
#include <QStackedLayout>

#include <QDBusInterface>
#include <QFileDialog>

#include "globaldefine.h"
#include "pagecontext.h"
#include "cprogressdialog.h"

#ifdef USE_DTK
#include <DFloatingMessage>
#include <DDialog>
DWIDGET_USE_NAMESPACE
#endif

class DrawBoardToolMgr;
class PageView;
class PageScene;
class CPrintManager;
class ProgressLayout;
class CExportImageDialog;
class ImageLoadTool;
class TabBarWgt;

class DrawBoard;
class DrawTool;
class PageContext;

class DRAWLIB_EXPORT Page: public QWidget
{
    Q_OBJECT
public:
    explicit Page(DrawBoard *parent = nullptr);

    Page(PageContext *context, DrawBoard *parent = nullptr);

    ~Page();

    bool isActivedPage()const;

    bool close(bool force = false);

    bool isModified() const;
    QString title() const;
    void    setTitle(const QString &title);

    SAttrisList currentAttris() const;
    void     setAttributionVar(int attri, const QVariant &var, int phase, bool autoCmdStack);
    void     showAttributions(const SAttrisList &attribution, int active = 0);

    void     setDefaultAttriVar(int attri, const QVariant &var);
    QVariant defaultAttriVar(int attri) const;

    QString name()const;
    void setName(const QString &name);

    QString file()const;
    void    setFile(const QString &file)const;

    DrawBoard *borad() const;
    void setBorad(DrawBoard *borad);

    QRectF pageRect() const;
    void   setPageRect(const QRectF &rect);

    QString key() const;

    PageView *view()const;
    PageScene *scene() const;

    PageContext *context() const;
    void setContext(PageContext *contex);
    bool save(const QString &file = "");
    bool saveAs();
    bool saveToImage(const QString &path, const QSize &desImageSize = QSize(), int qulity = 100) const;

    void setCurrentTool(int tool);
    int  currentTool()const;
    DrawTool *currentTool_p() const;

    void setDrawCursor(const QCursor &cursor);
    QCursor drawCursor()const;

    void blockSettingDrawCursor(bool b);
    bool isSettingDrawCursorBlocked() const;
    /**
     * @brief adjustViewScaleRatio　根据添加文件缩放视图
     */
    void adjustViewScaleRatio();
protected:
    void closeEvent(QCloseEvent *event) override;

private:
    PRIVATECLASS(Page)
};

class AttributionManager;
class FileHander;
class SystemFileWatcher;
class DrawBoardMimeData;
class DRAWLIB_EXPORT DrawBoard: public QWidget
{
    Q_OBJECT
public:
    static void initResource();
    static void loadTranslator();
    static void loadToolPlugins();

    DrawBoard(QWidget *parent = nullptr);
    ~DrawBoard();

    Page *currentPage() const;
    Page *firstPage()const;
    Page *nextPage(Page *page)const;
    Page *endPage()const;
    int count() const;

    bool isAnyPageModified() const;

    Q_SLOT Page *addPage(const QString &name = "");
    Q_SLOT Page *addPage(const QImage &img);
    Q_SLOT Page *addPage(PageContext *pageCxt);
    Q_SLOT void addPage(Page *page);

    Q_SLOT bool closePage(const QString &key);
    Q_SLOT bool closePage(Page *page);
    Q_SLOT void setCurrentPage(const Page *page);
    Q_SLOT void setCurrentPage(const QString &key);

    Page *page(int index)const;
    Page *page(const QString &key)const;
    Page *getPageByFile(const QString &file) const;

    void  setPageName(Page *page, const QString &name);
    QString pageName(Page *page)const;

    QString pageTitle(Page *page) const;
    void    setPageTitle(Page *page, const QString &title);

    DrawBoardToolMgr *toolManager() const;
    void setToolManager(DrawBoardToolMgr *toolManager);

    void setAttributionManager(AttributionManager *manager);
    AttributionManager *attributionManager() const;
    void showAttributions(const SAttrisList &attribution, int active = 1, const QPoint &pos = QPoint());

    SAttrisList currentAttris() const;
    SAttrisList pageAttris(const Page *page)const;

    QVariant pageAttriVariant(const Page *page, int attri) const;
    Q_SLOT void setDrawAttribution(int attris, const QVariant &var, int phase = EChanged, bool autoCmdStack = true);

    int  currentTool()const;
    DrawTool *currentTool_p()const;
    DrawTool *tool(int tool)const;
    bool setCurrentTool(int tool);
    bool setCurrentTool(DrawTool *tool);

    /**
     * @brief loadFiles 加载文件(可异步)
     * @param filePaths 文件名
     * @param asyn 是否异步加载
     * @param loadTypeForImage 图像加载的方式，0表示使当前画布大小和图像一致，1表示使图像自适应画布大小
     * @return void
     */
    void loadFiles(const QStringList &filePaths,
                   bool asyn = true,
                   int loadTypeForImage = 1,
                   bool quitIfAllFialed = false);

    bool load(const QString &file);
    bool loadDDf(const QString &file);
    bool loadImage(const QString &file,
                   bool adapt = true,
                   bool changContexSizeToImag = false);
    //导入图片个数限制
    int execPicturesLimit(int count);

    bool savePage(Page *page);
    FileHander *fileHander() const;
    SystemFileWatcher *fileWatcher() const;

    void setClipBoardShapeData(QMimeData *data);
    QMimeData *clipBoardShapeData()const;

    void setTouchFeelingEnhanceValue(int value);
    int  touchFeelingEnhanceValue()const;

    void zoomTo(qreal total);

    void setAutoClose(bool b);
    bool isAutoClose()const;

signals:
    void pageAdded(Page *page);
    void pageRemoved(Page *page);
    void currentPageChanged(Page *page);
    void currentPageChanged(const QString &pageKey);
    void clipBoardDataChanged(const QMimeData *data);

    void modified(bool modified);
    void zoomValueChanged(qreal totalScaled);
    void toClose();

    void attributionManagerChanged(AttributionManager *pOldMgr, AttributionManager *pNowMgr);
    void toolManagerChanged(DrawBoardToolMgr *pOldMgr, DrawBoardToolMgr *pNowMgr);

    void cutSizeChange(QSizeF rect, bool setattr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *e)override;
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    Q_SLOT void _onFileContextChanged(const QString &path, int tp);
    Q_SLOT void _onToolClicked(int tool);

    PRIVATESAFECLASS(DrawBoard)
};
#endif // MAINWIDGET_H
