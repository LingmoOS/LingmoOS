// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "drawboard.h"

#include "drawboardtoolmgr.h"
#include "pageview.h"
#include "cshapemimedata.h"
#include "datahander.h"

#include "cexportimagedialog.h"
#include "progresslayout.h"
#include "ccutdialog.h"

#include "pagescene.h"
#include "pagecontext.h"

#include "cprogressdialog.h"
#include "ddfhander.h"
#include "filewatcher.h"
#include "attributemanager.h"
#include "layeritem.h"
#include "drawboardtab.h"
#include "DataHanderInterface.h"
#include "drawdialog.h"
#include "cundoredocommand.h"
#include "setting.h"
#include "ctextedit.h"

#include <QDebug>
#include <QGraphicsItem>
#include <QtConcurrent>
#include <QDesktopWidget>
#include <QApplication>

#include <QPdfWriter>
#include <QScreen>
#include <QWindow>
#include <QTextEdit>
#include <QClipboard>
#include <QCheckBox>
#include <QStackedWidget>
#include <malloc.h>
#include <QMenu>
#include <QScrollBar>

#define NOTSHOWPROGRESS 1

#ifdef USE_DTK
#include <DGuiApplicationHelper>
#include <DMessageManager>
#include <DFloatingMessage>
#include <DToolButton>
DGUI_USE_NAMESPACE
#endif
const int CANVAS_MARGINS_WIDTH = 45;
const int CANVAS_MARGINS_HEIGHT = 105;
class Page::Page_private
{
public:
    explicit Page_private(Page *page): _page(page) {}
    inline bool &rForceClose()  {return _forceClose;}
private:
    Page *_page;
    bool _forceClose = false;
    bool _blockSettingDrawCursor = false;

    int  _currentTool = 0;
    PageView  *_view  = nullptr;
    PageContext *_context = nullptr;
    friend class Page;

    friend void _setPageTool(Page *page, int tool);
};
void _setPageTool(Page *page, int tool)
{
    if (page->borad()->toolManager() == nullptr)
        return;

    if (page->d_Page()->_currentTool != tool) {
        page->d_Page()->_currentTool = tool;
        if (page->borad() != nullptr) {
            auto pTool = page->borad()->toolManager()->tool(tool);
            if (pTool != nullptr) {
                page->setDrawCursor(pTool->cursor());
                page->view()->setFocus();
            }
        }
    }
}
class DrawBoard::DrawBoard_private
{
public:
    explicit DrawBoard_private(DrawBoard *borad): _borad(borad)
    {
        initUi();

        //init signal and slots
        connect(_stackWidget, &QStackedWidget::currentChanged, _borad, [ = ](int index) {
            auto page = qobject_cast<Page *>(_stackWidget->widget(index));
            if (page != nullptr) {

                if (_toolManager == nullptr)
                    return;

                //1.set ui show page current tool.
                QSignalBlocker bloker(_toolManager);
                qWarning() << "page->currentTool() = " << page->currentTool();
                _toolManager->setCurrentTool(page->currentTool());

                //2.set ui show page current total scaled value.
                emit _borad->zoomValueChanged(page->view()->getScale());

                //3.set ui show page context.
                page->showAttributions(page->currentAttris(), AttributionManager::CurrentPageChanged);

                //4. emit current page changed.
                //CManageViewSigleton::GetInstance()->setCurView(page->view());
                emit _borad->currentPageChanged(page);
                emit _borad->currentPageChanged(page->key());

                //5. set current undo stack
                //UndoCommand::setCurrentStack(page->view()->stack());
            }
        });
        s_boards.append(_borad);
    }
    ~DrawBoard_private()
    {
        _fileWatcher->deleteLater();
        _fileWatcher = nullptr;

        if (_pClipBordData != nullptr) {
            delete _pClipBordData;
            _pClipBordData = nullptr;
        }
        s_boards.removeOne(_borad);
    }
    void initUi()
    {
        //init layout
        _topTabs     = new TabBarWgt(_borad);
        _stackWidget = new PageStackedWidget(_borad);

        //QHBoxLayout *hLay = new QHBoxLayout;
        QVBoxLayout *hLay = new QVBoxLayout;
        hLay->setContentsMargins(0, 0, 0, 0);
        hLay->setSpacing(0);

        QVBoxLayout *subVLay = new QVBoxLayout;
        subVLay->setContentsMargins(0, 0, 0, 0);
        subVLay->setSpacing(0);
        subVLay->addWidget(_topTabs);

        QVBoxLayout *canvaslayout = new QVBoxLayout;
        canvaslayout->addWidget(_stackWidget);

        _attributionLayout = new QVBoxLayout;

        QHBoxLayout *statcklayout = new QHBoxLayout;
        statcklayout->addLayout(canvaslayout);
        statcklayout->addLayout(_attributionLayout);

        _attributionLayout->setContentsMargins(5, 0, 0, 0);
        hLay->addItem(subVLay);
        hLay->addLayout(statcklayout);
        _borad->setLayout(hLay);
    }

    ProgressLayout  *processDialog()
    {
        if (_dialog == nullptr) {
            _dialog = new ProgressLayout(_borad);
        }
        return _dialog;
    }

    QString execFileSelectDialog(const QString &defualFileName, bool toddf = true, QString file = "")
    {
        if (toddf) {
            FileSelectDialog dialog(_borad);
            dialog.setNameFilters(Setting::instance()->writableFormatNameFilters());
            dialog.selectNameFilter(Setting::instance()->defaultFileDialogNameFilter());

            if (!file.isEmpty()) {
                dialog.selectFile(file);
                dialog.setDirectory(QFileInfo(file).dir().absolutePath());
            } else {
                dialog.selectFile(defualFileName);
                dialog.setDirectory(Setting::instance()->defaultFileDialogPath());
            }
            dialog.exec();

            return dialog.resultFile();
        }
        CExportImageDialog dialog(_borad);
        dialog.exec();
        return dialog.resultFile();
    }

    QString execCheckLoadingFileToSupName(const QString &file)
    {
        QString legeFile = FileHander::toLegalFile(file);

        if (legeFile.isEmpty()) {
            //mean file not exist.
            MessageDlg::execMessage(tr("The file does not exist"), ENormalMsg);
        } else {
            QFileInfo info(legeFile);
            QString infoName = info.fileName();
            QFontMetrics font(_topTabs->font());
            infoName = font.elidedText(infoName, Qt::ElideMiddle, 200);//返回一个带有省略号的字符串

            if (info.isFile()) {
                const QString suffix = info.suffix().toLower();
                auto formatslist = Setting::instance()->readableFormats();
                if (formatslist.contains(suffix)) {
                    if (!info.isReadable()) {
                        MessageDlg::execMessage(tr("Unable to open the write-only file \"%1\"").arg(infoName), ENormalMsg);
                    } else {
                        return legeFile;
                    }
                } else {
                    MessageDlg::execMessage(tr("Unable to open \"%1\", unsupported file format").arg(infoName), ENormalMsg);
                }
            }
        }
        return "";
    }

    void addPageHelper(Page *page)
    {
        page->setParent(_stackWidget);
        _stackWidget->addWidget(page);
        _topTabs->addItem(page->name(), page->key());
        //重新计算画布大小
        int pageRect_width = page->scene()->sceneRect().width() - _toolManager->width() - _scroll->width();
        int pageRect_height = page->scene()->sceneRect().height() - _topTabs->height();
        //调整画布大小
        page->setPageRect(QRect(0, 0, pageRect_width - CANVAS_MARGINS_WIDTH, pageRect_height - CANVAS_MARGINS_HEIGHT));
        emit _borad->pageAdded(page);
    }
    void closePageHelper(Page *page)
    {
        _stackWidget->removeWidget(page);
        _topTabs->removeItem(page->key());
        emit _borad->pageRemoved(page);
        checkClose();
    }
    bool isFocusFriendWgt(QWidget *w)
    {
        bool result = false;
        if (_borad->attributionManager() != nullptr) {
            result = _borad->attributionManager()->isFriendWidget(w);
        }
        if (!result) {
            result = (qobject_cast<QMenu *>(qApp->activePopupWidget()) != nullptr);
        }
        return result;
    }

    void checkClose()
    {
        if (_borad->isAutoClose()) {
            QMetaObject::invokeMethod(_borad, [ = ]() {
                if (_borad->count() == 0
                        && qApp->activeModalWidget() == nullptr) {
                    _borad->close();
                }
            },
            Qt::QueuedConnection);
        }
    }

    void showErrorMsg(int error, const QString &describle)
    {
        qWarning() << "load result = " << error << describle;
        if (error != FileHander::NoError && (FileHander::EUserCancelLoad_OldPen != error && FileHander::EUserCancelLoad_OldBlur != error)) {
            MessageDlg::execMessage(describle, ENormalMsg);
        }
    }
    void setAttributionManager(AttributionManager *attri_Manager)
    {
        _attriManager = attri_Manager;
    }

    SAttrisList showExternAttr()
    {
        SAttrisList result;
        if (_toolManager->currentTool() != eraser && _toolManager->currentTool() != blur && _toolManager->currentTool() != cut) {
            SAttri groupAttri = SAttri(EGroupWgt, _borad->pageAttriVariant(_borad->currentPage(), EGroupWgt));
            SAttri orderAttri = SAttri(EOrderProperty, _borad->pageAttriVariant(_borad->currentPage(), EOrderProperty));
            result <<  groupAttri << orderAttri;

            if (0 == _borad->currentPage()->scene()->selectedItemCount() && _toolManager->currentTool() != line) {
                result << SAttri(ERotProperty, 0);
            }

        } else if (_toolManager->currentTool() == cut) {
            SAttri cutAttri = SAttri(ECutToolAttri, _borad->pageAttriVariant(_borad->currentPage(), ECutToolAttri));
            result <<  cutAttri;
        }

        return result;
    }

public:
    DrawBoard *_borad;

    DrawBoardToolMgr *_toolManager = nullptr;
    TabBarWgt       *_topTabs     = nullptr;
    QStackedWidget  *_stackWidget = nullptr;
    ProgressLayout *_dialog = nullptr;
    QVBoxLayout *_attributionLayout = nullptr;
    CExportImageDialog *_exportImageDialog = nullptr;
    DScrollArea *_leftScrollArea = nullptr;
    QPoint      _frontMousePos;

    int  _touchEnchValue = 7;
    bool _autoClose = false;
    bool _mouseClicked = false;

    bool enableShowPageItemAttris = false;

    AttributionManager *_attriManager = nullptr;
    QScrollArea *_scroll = nullptr;
    FileHander        *_fileHander = nullptr;
    SystemFileWatcher *_fileWatcher = nullptr;
    DrawBoardMimeData    *_pClipBordData = nullptr;

    static QList<DrawBoard *> s_boards;

    friend class DrawBoard;
};
QList<DrawBoard *> DrawBoard::DrawBoard_private::s_boards = QList<DrawBoard *>();

bool adaptImgPosAndRect(PageScene *pScene, const QString &imgName, const QImage &img, QPointF &pos, QRectF &rect, int &choice)
{
    if (nullptr == pScene)
        return false;

    QSizeF sceneSize = QSizeF(pScene->width(), pScene->height());
    bool bAddImg = true;
    if (sceneSize.width() < img.width() || sceneSize.height() < img.height()) {
        QString tmpName = imgName.isEmpty() ? QObject::tr("Unnamed") : imgName;

        auto btns = QStringList() << QObject::tr("Keep original size") << QObject::tr("Auto fit");

        static const int cancelImport = -2;//取消导入标识
        int ret = choice;
        if (-1 == choice) {
            auto parent = (pScene->page() != nullptr ? pScene->page()->borad() : qApp->activeWindow());
            MessageDlg msgDlg(parent);
#ifdef USE_DTK
            QCheckBox *pBox = new QCheckBox(QObject::tr("Apply to all"));
            msgDlg.addContent(pBox);
#endif
            msgDlg.setMessage(SMessage(QObject::tr("The dimensions of %1 exceed the canvas. How to display it?").arg(tmpName),
                                       EWarningMsg, QStringList() << QObject::tr("Keep original size") << QObject::tr("Auto fit"),
                                       QList<EButtonType>() << ENormalMsgBtn << ESuggestedMsgBtn));
            ret = msgDlg.exec();
#ifdef USE_DTK
            if (pBox->isChecked() && -1 != ret)
#else
            if (-1 != ret)
#endif
            {
                choice = ret;
            } else if (pBox->isChecked() && -1 == ret) {
                //取消后面导入比场景大的图
                choice = cancelImport;
            }
        } else if (cancelImport == choice) {
            ret = -1;
        }

        if (1 == ret) {
            double wRatio = 1.0 * sceneSize.width() / img.width();
            double hRatio = 1.0 * sceneSize.height() / img.height();
            double scaleRatio = wRatio > hRatio ? hRatio : wRatio;
            rect = QRectF(QPointF(0, 0), img.size() * scaleRatio);
            QPointF tmppos = pScene->sceneRect().center() - rect.center();
            pos.setX(tmppos.x());
            pos.setY(tmppos.y());
        } else if (-1 == ret) {
            bAddImg = false;
        }


    }
    return bAddImg;
}

static QString genericOneKey()
{
    static int s_pageCount = 0;
    return QString("%1").arg(++s_pageCount);
}

Page::Page(DrawBoard *parent): Page_d(new Page_private(this))
{
    d_Page()->_view     = new PageView(this);
    d_Page()->_view->setFrameShape(QFrame::NoFrame);

    QHBoxLayout *hLay = new QHBoxLayout(this);
    hLay->setContentsMargins(0, 0, 0, 0);
    this->setLayout(hLay);
    hLay->addWidget(d_Page()->_view);

    if (parent != nullptr)
        setBorad(parent);

    connect(d_Page()->_view, &PageView::signalSetScale, this, [ = ](const qreal scale) {
        if (this == borad()->currentPage())
            emit borad()->zoomValueChanged(scale);
    });

    setWgtAccesibleName(this, QString("Page%1").arg(genericOneKey()));
}

Page::Page(PageContext *context, DrawBoard *parent): Page(parent)
{
    setContext(context);
}

Page::~Page()
{
}

bool Page::isActivedPage() const
{
    return (this->borad()->currentPage() == this);
}

bool Page::close(bool force)
{
    d_Page()->rForceClose() = force;
    bool ret = QWidget::close();
    d_Page()->rForceClose() = false;
    return ret;
}

bool Page::isModified() const
{
    if (d_Page()->_context != nullptr)
        return d_Page()->_context->isDirty();
    return false;
}

QString Page::title() const
{
    auto name = this->name();
    if (name.isEmpty())
        return "";
    return isModified() ? ("* " + name) : name;
}

void Page::setTitle(const QString &title)
{
    borad()->setPageTitle(this, title);
}

SAttrisList Page::currentAttris() const
{
    if (d_Page()->_context != nullptr) {
        return d_Page()->_context->currentAttris();
    }
    SAttrisList result;
    return result;
}

void Page::setAttributionVar(int attri, const QVariant &var, int phase, bool autoCmdStack)
{
    if (d_Page()->_context != nullptr) {
        d_Page()->_context->scene()->setAttributionVar(attri, var, phase, autoCmdStack);
    }
    setDefaultAttriVar(attri, var);
}

void Page::showAttributions(const SAttrisList &attribution, int active)
{
    if (borad() != nullptr) {
        if (isActivedPage())
            borad()->showAttributions(attribution, active);
    }
}

void Page::setDefaultAttriVar(int attri, const QVariant &var)
{
    if (d_Page()->_context != nullptr) {
        d_Page()->_context->setDefaultAttri(attri, var);
    }
}

QVariant Page::defaultAttriVar(int attri) const
{
    QVariant var;
    if (d_Page()->_context != nullptr) {
        var = d_Page()->_context->defaultAttri(attri);
    }
    if (var.isNull()) {
        if (borad() != nullptr && borad()->attributionManager() != nullptr)
            var = borad()->attributionManager()->defaultAttriVar(attri);
    }
    return var;
}

QString Page::name() const
{
    if (d_Page()->_context != nullptr)
        return d_Page()->_context->name();

    return "";
}

void Page::setName(const QString &name)
{
    borad()->setPageName(this, name);
}

QString Page::file() const
{
    if (d_Page()->_context != nullptr)
        return d_Page()->_context->file();
    return "";
}

void Page::setFile(const QString &file) const
{
    if (d_Page()->_context != nullptr)
        return d_Page()->_context->setFile(file);
}

DrawBoard *Page::borad() const
{
    if (parentWidget() != nullptr && parentWidget()->parentWidget() != nullptr) {
        return qobject_cast<DrawBoard *>(parentWidget()->parentWidget());
    }
    return nullptr;
}

void Page::setBorad(DrawBoard *borad)
{
    if (borad != nullptr)
        borad->addPage(this);
}

QRectF Page::pageRect() const
{
    if (d_Page()->_context != nullptr)
        return d_Page()->_context->pageRect();

    return QRectF(0, 0, 0, 0);
}

void Page::setPageRect(const QRectF &rect)
{
    if (d_Page()->_context != nullptr)
        return d_Page()->_context->setPageRect(rect);
}

QString Page::key() const
{
    if (d_Page()->_context != nullptr)
        return d_Page()->_context->key();
    return "";
}

PageView *Page::view() const
{
    return d_Page()->_view;
}

PageScene *Page::scene() const
{
    if (d_Page()->_context != nullptr) {
        return d_Page()->_context->scene();
    }
    return nullptr;
}

PageContext *Page::context() const
{
    return d_Page()->_context;
}

void Page::setContext(PageContext *contex)
{
    if (d_Page()->_context != contex) {
        d_Page()->_context = contex;
        d_Page()->_context->setParent(this);
        d_Page()->_view->setScene(d_Page()->_context->scene());
    }
}

bool Page::save(const QString &file)
{
    bool refuse = currentTool_p() != nullptr ? currentTool_p()->blockPageBeforeOutput(this) : false;
    if (refuse) {
        return false;
    }

    if (!isModified())
        return true;

    if (d_Page()->_context != nullptr) {
        QString f = file.isEmpty() ? d_Page()->_context->file() : file;
        if (f.isEmpty()) {
            f = borad()->d_DrawBoard()->execFileSelectDialog(d_Page()->_context->name(), true, f);
        }

        if (f.isEmpty())
            return false;

        //如果当前格式不支持写，弹出另存为窗口
        QString writableFormats = "." + Setting::instance()->writableFormats().join(".") + ".";
        if (!writableFormats.contains("." + QFileInfo(f).suffix().toLower() + ".")) {
            return saveAs();
        }

        bool result = d_Page()->_context->save(f);

        qWarning() << "save result = " << (borad()->fileHander()->lastError()) << (borad()->fileHander()->lastErrorDescribe());
        if (!result) {
            auto error = borad()->fileHander()->lastError();
            if (error != FileHander::NoError) {
                MessageDlg::execMessage(borad()->fileHander()->lastErrorDescribe(), ENormalMsg, borad());

                if (error == FileHander::EUnWritableFile) {
                    auto anotherFile = borad()->d_DrawBoard()->execFileSelectDialog(d_Page()->_context->name());
                    if (!anotherFile.isEmpty())
                        result = save(anotherFile);
                }
            }
        }

        return result;
    }

    return false;
}

bool Page::saveAs()
{
    if (d_Page()->_context != nullptr) {
        if (d_Page()->_context->isEmpty())
            return true;

        bool refuse = currentTool_p() != nullptr ? currentTool_p()->blockPageBeforeOutput(this) : false;
        if (refuse) {
            return false;
        }

        QString file = borad()->d_DrawBoard()->execFileSelectDialog(d_Page()->_context->name());
        if (file.isEmpty()) {
            return false;
        }
        bool result = d_Page()->_context->save(file);
        qWarning() << "save result = " << (borad()->fileHander()->lastError()) << (borad()->fileHander()->lastErrorDescribe());
        if (!result) {
            auto error = borad()->fileHander()->lastError();
            if (error != FileHander::NoError) {
                MessageDlg::execMessage(borad()->fileHander()->lastErrorDescribe(), ENormalMsg, borad());

                if (error == FileHander::EUnWritableFile) {
                    auto anotherFile = borad()->d_DrawBoard()->execFileSelectDialog(d_Page()->_context->name());
                    if (!anotherFile.isEmpty())
                        result = save(anotherFile);
                }
            }
        }
        return result;
    }

    return false;
}

bool Page::saveToImage(const QString &file, const QSize &desImageSize, int qulity) const
{
    if (d_Page()->_context != nullptr) {
        if (file.isEmpty()) {
            return false;
        }
        return borad()->fileHander()->saveToImage(d_Page()->_context, file, desImageSize, qulity);
    }
    return false;
}

void Page::setCurrentTool(int tool)
{
    if (borad()->currentPage() == this)
        borad()->toolManager()->setCurrentTool(tool);
    else
        _setPageTool(this, tool);
}

int Page::currentTool() const
{
    return d_Page()->_currentTool;
}

DrawTool *Page::currentTool_p() const
{
    if (borad() != nullptr && borad()->toolManager() != nullptr) {
        return borad()->toolManager()->tool(currentTool());
    }

    return nullptr;
}

void Page::setDrawCursor(const QCursor &cursor)
{
    if (d_Page()->_blockSettingDrawCursor)
        return;
    view()->viewport()->setCursor(cursor);
}

QCursor Page::drawCursor() const
{
    return view()->viewport()->cursor();
}

void Page::blockSettingDrawCursor(bool b)
{
    d_Page()->_blockSettingDrawCursor = b;
}

bool Page::isSettingDrawCursorBlocked() const
{
    return d_Page()->_blockSettingDrawCursor;
}

void Page::closeEvent(QCloseEvent *event)
{
    if (d_Page()->rForceClose()) {
        event->accept();
        borad()->d_DrawBoard()->closePageHelper(this);
        return;
    }
    bool refuse = currentTool_p() != nullptr ? currentTool_p()->blockPageBeforeOutput(this) : false;

    if (!refuse) {
        if (this->isModified()) {
            borad()->setCurrentPage(this);
            DrawDialog quitQuestionDialog(this);
            int ret = quitQuestionDialog.exec();
            if (ret == 2) {
                QString file = this->file();
                if (file.isEmpty()) {
                    file = borad()->d_DrawBoard()->execFileSelectDialog(d_Page()->_context->name());
                }
                if (file.isEmpty()) {
                    refuse = true;
                } else {
                    bool success = this->save(file);
                    if (!success) {
                        auto error = borad()->fileHander()->lastError();
                        if (error == FileHander::EUnWritableFile) {
                            refuse = true;
                        }
                    }
                }

            } else if (ret <= 0) {
                refuse = true;
            }
        }
    }

    if (refuse) {
        event->ignore();
    } else {
        event->accept();
        borad()->d_DrawBoard()->closePageHelper(this);
    }
}

void Page::adjustViewScaleRatio()
{
    if (nullptr != view()) {
        int viewportWidth = view()->viewport()->width();
        int viewportHeight = view()->viewport()->height();
        qreal sceneWidth = scene()->width();
        qreal sceneHeight = scene()->height();
        if (viewportWidth < sceneWidth || viewportHeight < sceneHeight) {
            qreal wRatio = qFloor(100.0 * viewportWidth / sceneWidth);
            qreal hRatio = qFloor(100.0 * viewportHeight / sceneHeight);
            qreal rsRatio = wRatio > hRatio ? hRatio / 100 : wRatio / 100;
            view()->scale(rsRatio);
        }
    }
}

static int noticeFileContextChanged(Page *page, DrawBoard *borad)
{
#ifdef USE_DTK
    static QMap<QString, DDialog *> noticeDialogMap;
    static int reload = 0;
    static int cancel = 1;
    auto itf = noticeDialogMap.find(page->file());
    if (itf == noticeDialogMap.end()) {
        auto dia = new DDialog(borad);
        dia->setFixedSize(404, 163);
        dia->setAttribute(Qt::WA_DeleteOnClose);
        dia->setModal(true);
        dia->setIcon(QPixmap(":/icons/deepin/builtin/Bullet_window_warning.svg"));
        dia->setProperty("file", page->file());
        //设置显示文字与交互按钮
        QFileInfo fInfo(page->file());
        QString shortenFileName = QFontMetrics(dia->font()).elidedText(fInfo.fileName(), Qt::ElideMiddle, dia->width() / 2);
        dia->setMessage(QObject::tr("%1 has been modified in other programs. Do you want to reload it?").arg(shortenFileName));
        reload  = dia->addButton(QObject::tr("Reload"), false, DDialog::ButtonNormal);
        cancel  = dia->addButton(QObject::tr("Cancel"), false, DDialog::ButtonWarning);
        //设置message的两边间隙为10
        QWidget *pWidget = dia->findChild<QWidget *>("MessageLabel"); //如果dtk修改了object名字，记得修改
        if (pWidget != nullptr) {
            QMargins margins = pWidget->contentsMargins();
            margins.setLeft(10);
            margins.setRight(10);
            pWidget->setContentsMargins(margins);
        }
        noticeDialogMap[page->file()] = dia;
    } else {
        borad->activateWindow();
        return 2;
    }

    auto dialog = noticeDialogMap[page->file()];
    int ret = dialog->exec();
    noticeDialogMap.remove(page->file());
    if (ret == reload) {
        return 0;
    } else if (ret == cancel)
        return 1;

    return 2;
#endif
    return 2;
}

void DrawBoard::initResource()
{
    Q_INIT_RESOURCE(drawBaseRes);
    //Q_INIT_RESOURCE(frameRes);
    Q_INIT_RESOURCE(images);
    Q_INIT_RESOURCE(cursorIcons);
}

void DrawBoard::loadTranslator()
{
    static bool haveInit = false;

    if (!haveInit) {
        const QString transfileDir = TRANSFILEPATH;
        qDebug() << "translation path is " << transfileDir;
        //load translations
        QDir dir(transfileDir);
        if (dir.exists()) {
            QDirIterator qmIt(transfileDir, QStringList() << QString("*%1.qm").arg(QLocale::system().name()), QDir::Files);
            while (qmIt.hasNext()) {
                qmIt.next();
                QFileInfo finfo = qmIt.fileInfo();
                QTranslator *translator = new QTranslator;
                if (translator->load(finfo.baseName(), finfo.absolutePath())) {
                    QCoreApplication::installTranslator(translator);
                }
            }
        }
        haveInit = true;
    }
}

void DrawBoard::loadToolPlugins()
{
    DrawToolFactory::loadAllToolPlugins();
}

DrawBoard::DrawBoard(QWidget *parent): QWidget(parent), DrawBoard_d(new DrawBoard_private(this))
{
    loadTranslator();
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, [ = ]() {
        setClipBoardShapeData(const_cast<QMimeData *>(QApplication::clipboard()->mimeData()));
    });

//    //设置一个默认的Tool Manager
//    setToolManager(new DrawBoardToolMgr(this, this)); setToolManager(new DrawBoardToolMgr(this, this));
//    layout()->addWidget(d_DrawBoard()->_toolManager);
//    layout()->setAlignment(d_DrawBoard()->_toolManager, Qt::AlignCenter);
//    dynamic_cast<QVBoxLayout *>(layout())->addSpacing(10);

    //file hander init
    d_DrawBoard()->_fileHander = new FileHander(this);

    connect(d_DrawBoard()->_fileHander, &FileHander::progressBegin, this, [ = ](const QString & describe) {
        d_DrawBoard()->processDialog()->reset();
        d_DrawBoard()->processDialog()->showInCenter(this);
        d_DrawBoard()->processDialog()->setText(describe);
    });
    connect(d_DrawBoard()->_fileHander, &FileHander::progressChanged, this, [ = ](int progress, int total, const QString & describe) {
        if (!describe.isEmpty())
            d_DrawBoard()->processDialog()->setText(describe);

        d_DrawBoard()->processDialog()->setRange(0, total);
        d_DrawBoard()->processDialog()->setSubText("");
        d_DrawBoard()->processDialog()->setProgressValue(progress);
    });
    connect(d_DrawBoard()->_fileHander, &FileHander::progressEnd, this, [ = ](bool success, const QString & describe) {
        d_DrawBoard()->processDialog()->delayClose();
    });
    connect(d_DrawBoard()->_fileHander, &FileHander::message_waitAnswer, this, [ = ](const SMessage & message, int &retureRet) {
        retureRet = MessageDlg::execMessage(message);
    });
    d_DrawBoard()->_fileWatcher = new SystemFileWatcher();
    connect(d_DrawBoard()->_fileWatcher, &SystemFileWatcher::fileChanged, this, &DrawBoard::_onFileContextChanged);

    this->setAcceptDrops(true);

    static int count = 0;
    setWgtAccesibleName(this, QString("DrawBoard%1").arg(++count));

    qApp->installEventFilter(this);
}

DrawBoard::~DrawBoard()
{
    delete d_DrawBoard()->_stackWidget;
    d_DrawBoard()->_stackWidget = nullptr;
    delete  DrawBoard_d;
    DrawBoard_d = nullptr;
}

Page *DrawBoard::addPage(const QString &name)
{
    auto pageCxt = new PageContext(name);
    auto page = new Page(pageCxt);
    addPage(page);

    qWarning() << "page parent ==== " << page->parentWidget() << (page->parentWidget() == d_DrawBoard()->_stackWidget);
    return page;
}

Page *DrawBoard::addPage(const QImage &img)
{
    if (!img.isNull()) {
        PageContext *contex = new PageContext;
        contex->addImageItem(img, QPointF(), QRectF(0, 0, img.width(), img.height()));
        return addPage(contex);
    }
    return nullptr;
}

Page *DrawBoard::addPage(PageContext *pageCxt)
{
    if (pageCxt == nullptr)
        return nullptr;
    auto page = new Page(pageCxt);
    addPage(page);
    return page;
}

void DrawBoard::addPage(Page *page)
{
    d_DrawBoard()->addPageHelper(page);

    fileWatcher()->addWather(page->file());

    setCurrentPage(page);
}

bool DrawBoard::closePage(const QString &key)
{
    auto page = this->page(key);
    return closePage(page);
}

bool DrawBoard::closePage(Page *page)
{
    return page->close();
}

Page *DrawBoard::currentPage() const
{
    auto w = d_DrawBoard()->_stackWidget->currentWidget();
    if (w != nullptr)
        return qobject_cast<Page *>(w);
    return nullptr;
}

Page *DrawBoard::firstPage() const
{
    return page(0);
}

Page *DrawBoard::nextPage(Page *page) const
{
    int index = d_DrawBoard()->_topTabs->index(page->key());

    if (index == -1)
        return nullptr;

    ++index;

    auto key = d_DrawBoard()->_topTabs->key(index);

    return this->page(key);
}

Page *DrawBoard::endPage() const
{
    return page(count() - 1);
}

int DrawBoard::count() const
{
    return d_DrawBoard()->_stackWidget->count();
}

bool DrawBoard::isAnyPageModified() const
{
    for (int i = 0; i < count(); ++i) {
        auto page = this->page(i);
        if (page->isModified()) {
            return true;
        }
    }
    return false;
}

void DrawBoard::setCurrentPage(const Page *page)
{
    if (page == nullptr)
        return;

    d_DrawBoard()->_stackWidget->setCurrentWidget(const_cast<Page *>(page));
    QSignalBlocker bloker(d_DrawBoard()->_topTabs);
    d_DrawBoard()->_topTabs->setCurrentIndex(d_DrawBoard()->_topTabs->index(page->key()));
    this->activateWindow();
}

void DrawBoard::setCurrentPage(const QString &key)
{
    auto page = this->page(key);

    if (page != nullptr)
        setCurrentPage(page);
}

Page *DrawBoard::page(int index) const
{
    auto key = d_DrawBoard()->_topTabs->key(index);
    return page(key);
}

Page *DrawBoard::page(const QString &key) const
{
    for (int i = 0; i < d_DrawBoard()->_stackWidget->count(); ++i) {
        auto page = qobject_cast<Page *>(d_DrawBoard()->_stackWidget->widget(i));
        if (page->key() == key) {
            return page;
        }
    }
    return nullptr;
}

Page *DrawBoard::getPageByFile(const QString &file) const
{
    auto temp = FileHander::toLegalFile(file);
    if (temp.isEmpty())
        return nullptr;

    for (int i = 0; i < d_DrawBoard()->_stackWidget->count(); ++i) {
        auto page = qobject_cast<Page *>(d_DrawBoard()->_stackWidget->widget(i));
        if (page->file() == temp) {
            return page;
        }
    }
    return nullptr;
}

void DrawBoard::setPageName(Page *page, const QString &name)
{
    if (page->context() != nullptr)
        page->context()->setName(name);

    setPageTitle(page, page->title());
}

QString DrawBoard::pageName(Page *page) const
{
    return page->name();
}

QString DrawBoard::pageTitle(Page *page) const
{
    return page->title();
}

void DrawBoard::setPageTitle(Page *page, const QString &title)
{
    auto tabIndex = d_DrawBoard()->_topTabs->index(page->key());
    d_DrawBoard()->_topTabs->setTabText(tabIndex, title);
    d_DrawBoard()->_topTabs->setTabToolTip(tabIndex, title);
}

DrawBoardToolMgr *DrawBoard::toolManager() const
{
    return d_DrawBoard()->_toolManager;
}

void DrawBoard::setToolManager(DrawBoardToolMgr *toolManager)
{
    auto oldMgr = d_DrawBoard()->_toolManager;
    if (d_DrawBoard()->_toolManager != nullptr) {
        disconnect(d_DrawBoard()->_toolManager, &DrawBoardToolMgr::toolClicked, this, &DrawBoard::_onToolClicked);
    }

    d_DrawBoard()->_toolManager = toolManager;

    connect(d_DrawBoard()->_toolManager, &DrawBoardToolMgr::toolClicked, this, &DrawBoard::_onToolClicked);

    emit toolManagerChanged(oldMgr, d_DrawBoard()->_toolManager);

    setCurrentTool(selection);
}


void DrawBoard::setAttributionManager(AttributionManager *manager)
{
    if (d_DrawBoard()->_attriManager != nullptr) {
        disconnect(d_DrawBoard()->_attriManager->helper(), &AttributionManagerHelper::attributionChanged,
                   this, &DrawBoard::setDrawAttribution);
    }

    if (manager->displayWidget() != nullptr) {

        QScrollArea *scroll = new QScrollArea(this);
        scroll->setMinimumWidth(290);
        scroll->setMaximumWidth(310);
        scroll->setContentsMargins(0, 0, 0, 0);
        scroll->setWidgetResizable(true);
        scroll->setWidget(manager->displayWidget());
        scroll->setAlignment(Qt::AlignLeft);
        scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scroll->setFrameShape(QFrame::NoFrame);
        d_DrawBoard()->_scroll = scroll;

        d_DrawBoard()->_attributionLayout->addWidget(scroll);
        manager->displayWidget()->show();
    }

    auto old = d_DrawBoard()->_attriManager;

    d_DrawBoard()->_attriManager = manager;

    if (d_DrawBoard()->_attriManager != nullptr)
        connect(d_DrawBoard()->_attriManager->helper(), &AttributionManagerHelper::attributionChanged,
                this, &DrawBoard::setDrawAttribution);

    emit attributionManagerChanged(old, d_DrawBoard()->_attriManager);
}

AttributionManager *DrawBoard::attributionManager() const
{
    return d_DrawBoard()->_attriManager;
}

void DrawBoard::showAttributions(const SAttrisList &attribution, int active, const QPoint &pos)
{
    if (d_DrawBoard()->_attriManager != nullptr) {
        SAttrisList tmpAttri = attribution;
        tmpAttri << d_DrawBoard()->showExternAttr();
        d_DrawBoard()->_attriManager->showAttributions(tmpAttri, active, pos);
    }
}

SAttrisList DrawBoard::currentAttris() const
{
    SAttrisList attris;

    //1.first check page attris(contxt attris).
    if (currentPage() != nullptr && currentTool() == selection) {
        attris = currentPage()->currentAttris();
    }

    //2.second check tools attris(contxt attris).
    if (attris.isEmpty()) {
        if (currentTool_p() != nullptr) {
            attris = currentTool_p()->attributions();
        }
    }

    //3.third check current page title attri.
    if (attris.isEmpty()) {
        if (currentPage() != nullptr)
            attris << SAttri(ETitle, currentPage()->title());
    }

    //4.finish show application title.
    if (attris.isEmpty()) {
        attris << SAttri(ETitle, qApp->applicationName());
    }
    return attris;
}

SAttrisList DrawBoard::pageAttris(const Page *page) const
{
    if (page != nullptr) {
        return page->currentAttris();
    }
    return SAttrisList();
}

QVariant DrawBoard::pageAttriVariant(const Page *page, int attri) const
{
    if (page == nullptr) {
        if (d_DrawBoard()->_attriManager != nullptr)
            return d_DrawBoard()->_attriManager->defaultAttriVar(attri);
        return QVariant();
    }
    return page->defaultAttriVar(attri);
}

int DrawBoard::currentTool() const
{
    if (toolManager() == nullptr)
        return -1;
    return toolManager()->currentTool();
}

DrawTool *DrawBoard::currentTool_p() const
{
    if (toolManager() == nullptr)
        return nullptr;

    return toolManager()->tool(toolManager()->currentTool());
}

DrawTool *DrawBoard::tool(int tool) const
{
    return toolManager()->tool(tool);
}

bool DrawBoard::setCurrentTool(int tool)
{
    if (toolManager() == nullptr)
        return false;

    return toolManager()->setCurrentTool(tool);
}

bool DrawBoard::setCurrentTool(DrawTool *tool)
{
    if (toolManager() == nullptr)
        return false;
    return toolManager()->setCurrentTool(tool);
}


template <class Fun>
void doMyRun(Fun fc, bool syn = false)
{
    if (syn) {
        fc();
    } else {
        QtConcurrent::run([ = ]() {
            fc();
        });
    }
}

void DrawBoard::loadFiles(const QStringList &filePaths, bool asyn,  int loadTypeForImage, bool quitIfAllFialed)
{
    QStringList validFilePaths;
    for (auto filePath : filePaths) {
        // 检测需要加载的文件是否合法
        QString validPath = d_DrawBoard()->execCheckLoadingFileToSupName(filePath);
        if (!validPath.isEmpty()) {
            validFilePaths.append(validPath);
        }
    }

    if (validFilePaths.isEmpty()) {
        return;
    }

    doMyRun([ = ]() {
        QMetaObject::invokeMethod(this, [ = ]() {
            if (validFilePaths.size() > 0) {
                d_DrawBoard()->processDialog()->setRange(0, validFilePaths.count());
                d_DrawBoard()->processDialog()->showInCenter(this);
            }
        }, Qt::AutoConnection);

        Qt::ConnectionType  connectType = asyn ? Qt::BlockingQueuedConnection : Qt::DirectConnection;

        int lastChoice = -1;
        bool loaded = false;
        int i = 0;
        for (; i < validFilePaths.size(); ++i) {
            QMetaObject::invokeMethod(this, [ = ]() {
                d_DrawBoard()->processDialog()->setProgressValue(i);
            }, Qt::AutoConnection);

            QString path = validFilePaths[i];
            QFileInfo info(path);
            auto stuffix = info.suffix();
            if (FileHander::supDdfStuffix().contains(stuffix)) {
                DdfHander hander(path, new PageContext);
                connect(&hander, &DdfHander::message_waitAnswer, this, [ = ](const SMessage & message, int &retureRet) {
                    retureRet = MessageDlg::execMessage(message);
                }, connectType);


                Page *pg = nullptr;
                QMetaObject::invokeMethod(this, [ =, &pg]() {
                    pg = getPageByFile(path);
                }, connectType);

                if (pg != nullptr) {
                    if (i == validFilePaths.count()) {
                        setCurrentPage(pg);
                    }
                    continue;
                }

                //PageContext *p = hander.load(path);
                bool result = hander.load();

                if (/*nullptr == p*/!result) {
                    //没有page页，需要新增，防止背景错误
                    if (!currentPage()) {
                        PageContext *p = hander.context();
                        QMetaObject::invokeMethod(this, [ = ]() {
                            addPage(p);
                        }, Qt::AutoConnection);
                    }
                    QMetaObject::invokeMethod(this, [ =, &hander]() {
                        d_DrawBoard()->showErrorMsg(hander.error(), hander.errorString());
                    }
                    , connectType);
                    continue;
                }
                loaded = true;

                PageContext *p = hander.context();
                QMetaObject::invokeMethod(this, [ = ]() {
                    addPage(p);
                }, Qt::AutoConnection);
            } else {
                ImageHander hander;
                connect(&hander, &ImageHander::message_waitAnswer, this, [ = ](const SMessage & message, int &retureRet) {
                    retureRet = MessageDlg::execMessage(message);
                }, connectType);

                QImage img = hander.load(path);
                QSize maxSize(10000, 10000);
                if (img.isNull()) {
                    QMetaObject::invokeMethod(this, [ =, &hander]() {
                        d_DrawBoard()->showErrorMsg(hander.error(), hander.errorString());
                    }
                    , connectType);
                    continue;
                }

                if (img.size().width() > maxSize.width() || img.size().height() > maximumSize().height()) {

                    int ret = 0;
                    emit hander.message_waitAnswer(SMessage(QObject::tr("Import failed: no more than 10,000 pixels please"), EWarningMsg), ret);
                    continue;
                }

                loaded = true;
                QMetaObject::invokeMethod(this, [ =, &lastChoice]() {

                    bool bNewPage = false;
                    if (nullptr == currentPage()) {
                        addPage("");
                        currentPage()->setPageRect(QRectF(QPointF(0, 0), img.size()));
                        currentPage()->setFile(path);
                        currentPage()->context()->setDirty(false);
                        bNewPage = true;
                    }

                    if (0 == loadTypeForImage) {
                        QRectF sceneRect = currentPage()->pageRect();
                        QRectF newRect = sceneRect.adjusted(
                                             (sceneRect.width() > img.width() ? 0 : (sceneRect.width() - img.width()) / 2),
                                             (sceneRect.height() > img.height() ? 0 : (sceneRect.height() - img.height()) / 2),
                                             (sceneRect.width() > img.width() ? 0 : -(sceneRect.width() - img.width()) / 2),
                                             (sceneRect.height() > img.height() ? 0 : -(sceneRect.height() - img.height()) / 2)
                                         );
                        currentPage()->setPageRect(newRect);
                        //顶层画布需要调整成新大小
                        PageView *pView = currentPage()->view();
                        pView->pageScene()->currentTopLayer()->setRect(newRect);
                        currentPage()->context()->scene()->clearSelections();
                        currentPage()->context()->addImageItem(img, QPointF(), QRectF());
                        currentPage()->adjustViewScaleRatio();
                    } else if (1 == loadTypeForImage) {
                        QPointF pos;
                        QRectF rect;
                        if (adaptImgPosAndRect(currentPage()->scene(), info.fileName(), img, pos, rect, lastChoice)) {
                            currentPage()->context()->scene()->clearSelections();
                            currentPage()->context()->addImageItem(img, pos, rect);
                        }
                        if (-2 == lastChoice) {
                            QMetaObject::invokeMethod(this, [ = ]() {
                                d_DrawBoard()->processDialog()->setProgressValue(i);
                                d_DrawBoard()->processDialog()->delayClose();
                                if (!loaded && quitIfAllFialed) {
                                    close();
                                }

                            }, Qt::AutoConnection);
                            return;
                        }
                    }

                }, connectType);
            }
        }

        QMetaObject::invokeMethod(this, [ = ]() {
            d_DrawBoard()->processDialog()->setProgressValue(i);
            d_DrawBoard()->processDialog()->delayClose();
            if (!loaded && quitIfAllFialed) {
                close();
            }

        }, Qt::AutoConnection);
    }, !asyn);
}

bool DrawBoard::load(const QString &file)
{
    auto filePath = file;

    if (filePath.isEmpty())
        return false;

    bool ret = false;
    QFileInfo info(filePath);
    auto stuffix = info.suffix();
    if (FileHander::supDdfStuffix().contains(stuffix)) {
        ret = loadDDf(filePath);
    } else {
        ret = loadImage(filePath);
    }

    return ret;
}



bool DrawBoard::savePage(Page *page)
{
    if (page != nullptr) {
        return page->save();
    }

    return false;
}

FileHander *DrawBoard::fileHander() const
{
    return d_DrawBoard()->_fileHander;
}

SystemFileWatcher *DrawBoard::fileWatcher() const
{
    return d_DrawBoard()->_fileWatcher;
}

void DrawBoard::setClipBoardShapeData(QMimeData *data)
{
    QMimeData *result = data;
    if (d_DrawBoard()->_pClipBordData != nullptr) {
        delete d_DrawBoard()->_pClipBordData;
        d_DrawBoard()->_pClipBordData = nullptr;
    }

    if (data != nullptr) {
        auto shapeData = qobject_cast<DrawBoardMimeData *>(data);
        if (shapeData != nullptr) {
            d_DrawBoard()->_pClipBordData = shapeData;
            result = d_DrawBoard()->_pClipBordData;
        }
    }
    emit clipBoardDataChanged(result);
}

QMimeData *DrawBoard::clipBoardShapeData() const
{
    if (d_DrawBoard()->_pClipBordData != nullptr)
        return d_DrawBoard()->_pClipBordData;

    return const_cast<QMimeData *>(QApplication::clipboard()->mimeData());
}

void DrawBoard::setTouchFeelingEnhanceValue(int value)
{
    d_DrawBoard()->_touchEnchValue = value;
}

int DrawBoard::touchFeelingEnhanceValue() const
{
    return d_DrawBoard()->_touchEnchValue;
}

void DrawBoard::zoomTo(qreal total)
{
    if (currentPage() != nullptr) {
        currentPage()->view()->scale(total);
    }
}

void DrawBoard::setAutoClose(bool b)
{
    d_DrawBoard()->_autoClose = b;
}

bool DrawBoard::isAutoClose() const
{
    return d_DrawBoard()->_autoClose;
}

void DrawBoard::dragEnterEvent(QDragEnterEvent *e)
{
    QWidget::dragMoveEvent(e);
}

void DrawBoard::dragMoveEvent(QDragMoveEvent *event)
{
    QWidget::dragMoveEvent(event);
}

void DrawBoard::dropEvent(QDropEvent *e)
{
    QWidget::dropEvent(e);
}

void DrawBoard::closeEvent(QCloseEvent *event)
{
    bool refuse = false;
    Page *loopPage = this->firstPage();
    while (loopPage != nullptr) {
        if (!closePage(loopPage)) {
            refuse = true;
            break;
        }
        loopPage = this->firstPage();
    }
    if (refuse)
        event->ignore();
    else {
        event->accept();
        emit toClose();
    }
}

bool DrawBoard::eventFilter(QObject *o, QEvent *e)
{
#ifdef Q_OS_WIN
    if (e->type() == QEvent::MouseButtonPress) {
        if (qApp->activePopupWidget() != nullptr) {
            auto window = qApp->topLevelAt(static_cast<QMouseEvent *>(e)->globalPos());
            bool clickedInActivePopupWindow = (qApp->activePopupWidget()->isAncestorOf(window) || qApp->activePopupWidget() == window);
            if (window == nullptr || (!clickedInActivePopupWindow)) {
                qApp->activePopupWidget()->hide();
                return true;
            }
        }
    }
#endif
    if (d_DrawBoard() == nullptr)
        return QWidget::eventFilter(o, e);

    if (e->type() == QEvent::FocusOut) {
        //1.如果丢失焦点的是view，那么如果当前view有激活的代理widget且新的当前的焦点控件是焦点友好的(如属性设置界面或者颜色板设置控件)那么应该不丢失焦点
        auto currentFocus = qApp->focusWidget();

        //当前焦点和丢失焦点是一样的那么什么都不用处理
        if (currentFocus == o)
            return true;

        // 解决打开不支持的文件时，页面崩溃的问题
        if (currentPage() == nullptr)
            return false;

        auto currenView = currentPage()->view();
        if (currenView == o) {
            if (currenView->activeProxWidget() != nullptr /*&& d_DrawBoard()->isFocusFriendWgt(currentFocus)*/) {
                auto activeWgtFocusWgt = currenView->activeProxWidget();
                if (activeWgtFocusWgt != nullptr && qobject_cast<QTextEdit *>(activeWgtFocusWgt) != nullptr) {
                    auto textEditor = qobject_cast<QTextEdit *>(activeWgtFocusWgt);
                    textEditor->setTextInteractionFlags(textEditor->textInteractionFlags() & (~Qt::TextEditable));
                }
                return true;
            }
        }
        //2.如果丢失焦点的是属性界面的控件，如果当前的焦点控件不是焦点友好的(如属性设置界面或者颜色板设置控件)那么需要
        else if (d_DrawBoard()->isFocusFriendWgt(qobject_cast<QWidget *>(o))) {
            if (currenView->activeProxWidget() != nullptr) {
                currenView->setFocus();
                currenView->activeProxWidget()->setFocus();
            }
        }
    } else if (e->type() == QEvent::Hide) {
        if (o != nullptr && o->isWindowType()) {
            if (qApp->activePopupWidget() == nullptr) {
                if (currentPage() != nullptr) {
                    QMetaObject::invokeMethod(this, [ = ]() {
                        currentPage()->view()->setFocus();
                    }, Qt::QueuedConnection);
                }
            }
        }
    } else if (e->type() == QEvent::FocusIn) {
        if (currentPage() == nullptr) {
            return QWidget::eventFilter(o, e);
        }
        static bool sss = false;
        if (o == currentPage()->view() && !sss) {
            if (currentPage()->view()->activeProxWidget() != nullptr) {
                sss = true;
                auto activeWgtFocusWgt = currentPage()->view()->activeProxWidget();
                if (activeWgtFocusWgt != nullptr && qobject_cast<QTextEdit *>(activeWgtFocusWgt) != nullptr) {
                    auto textEditor = qobject_cast<QTextEdit *>(activeWgtFocusWgt);

                    if (!(textEditor->textInteractionFlags() & Qt::TextEditable)) {
                        textEditor->setTextInteractionFlags(textEditor->textInteractionFlags() | (Qt::TextEditable));
                    }

                }

                currentPage()->view()->setFocus();
                activeWgtFocusWgt->setFocus();
                sss = false;
                return true;
            }
        }
    } else if (e->type() == QEvent::Shortcut) {
        if (currentTool_p() != nullptr && currentTool_p()->isWorking()) {
            return true;
        }
        if (currentPage() != nullptr && currentPage()->view() != nullptr) {
            auto viewPort = currentPage()->view()->viewport();
            QMetaObject::invokeMethod(viewPort, [ = ]() {
                QMouseEvent event(QMouseEvent::MouseMove, viewPort->mapFromGlobal(QCursor::pos()), Qt::NoButton,
                                  Qt::NoButton, qApp->keyboardModifiers());
                qApp->sendEvent(viewPort, &event);
            }, Qt::QueuedConnection);
        }
    } else if (e->type() == QEvent::MouseButtonPress) {//鼠标点击画板时,如果焦点丢失的是文本里面的子窗口，把焦点转移到view
        QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(e);
        if (mouseEvent->button() == Qt::RightButton) {
            auto activeProxy = currentPage()->view()->activeProxWidget();
            auto text = dynamic_cast<TextEdit *>(activeProxy);
            if (text != nullptr) {
                text->setMenu();
                return true;
            }
        }
    } else if (e->type() == QEvent::FocusIn) {
        static bool sss = false;
        if (currentPage() != nullptr && o == currentPage()->view() && !sss) {
            if (currentPage()->view()->activeProxWidget() != nullptr) {
                sss = true;
                currentPage()->view()->setFocus();
                currentPage()->view()->activeProxWidget()->setFocus();
                sss = false;
                return true;
            }
        }
    } else if (e->type() == QEvent::MouseButtonPress) {
        QMouseEvent *event = dynamic_cast<QMouseEvent *>(e);
        QPoint currentPos = d_DrawBoard()->_toolManager->mapFromGlobal(event->globalPos());
        if (event->button() == Qt::LeftButton && nullptr !=  d_DrawBoard()->_toolManager && d_DrawBoard()->_toolManager->rect().contains(currentPos)) {
            auto child = d_DrawBoard()->_toolManager->childAt(currentPos);
            auto pTool = dynamic_cast<DToolButton *>(child);
            //点击位置不能在工具按钮上
            if (nullptr == pTool) {
                d_DrawBoard()->_mouseClicked  = true;
                d_DrawBoard()->_frontMousePos = currentPos;
            }
        }
    } else if (e->type() == QEvent::MouseMove && d_DrawBoard()->_mouseClicked) {
        QMouseEvent *event = dynamic_cast<QMouseEvent *>(e);
        int hOffset = (event->pos() - d_DrawBoard()->_frontMousePos).y();
        QScrollBar *scrollbar =  d_DrawBoard()->_leftScrollArea->verticalScrollBar();
        int verValue = scrollbar->value() + hOffset;
        scrollbar->setValue(qMin(qMax(scrollbar->minimum(), verValue), scrollbar->maximum()));
        d_DrawBoard()->_frontMousePos = event->pos();

    } else if (e->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *event = dynamic_cast<QMouseEvent *>(e);
        if (event->button() == Qt::LeftButton) {
            d_DrawBoard()->_mouseClicked  = false;
        }
    }

    return QWidget::eventFilter(o, e);
}

void DrawBoard::setDrawAttribution(int attris, const QVariant &var, int phase, bool autoCmdStack)
{
    if (currentPage() != nullptr) {
        auto tool = currentPage()->currentTool_p();
        if (tool != nullptr)
            tool->setAttributionVar(attris, var, phase, autoCmdStack);

        currentPage()->setDefaultAttriVar(attris, var);
    }
}

void DrawBoard::_onFileContextChanged(const QString &path, int tp)
{
    auto page = getPageByFile(path);
    if (page != nullptr) {
        QFileInfo info(path);
        if (info.exists()) {
            enum {EReload, ECLOSE, EJustActiveWindow};
            int ret = noticeFileContextChanged(page, this);
            switch (ret) {
            case EReload: {
                bool autoClose = this->isAutoClose();
                this->setAutoClose(false);
                page->close(true);
                this->setAutoClose(autoClose);
                load(path);
                d_DrawBoard()->checkClose();
                break;
            }
            case ECLOSE: {
                page->close(true);
                break;
            }
            default: {
                this->activateWindow();
            }
            }
        } else {
            fileWatcher()->removePath(path);
            page->setFile("");
            page->context()->setDirty(true);
        }
    }
}

void DrawBoard::_onToolClicked(int tool)
{
    auto tl =  toolManager()->tool(tool);
    auto button = tl->toolButton();
    if (tl != nullptr && tl->status() == DrawTool::EReady) {
        showAttributions(tl->attributions(),
                         AttributionManager::ToolActivedAndClicked,
                         button->mapToGlobal(QPoint(button->rect().center())));
    }
}

bool DrawBoard::loadDDf(const QString &file)
{
    auto filePath = file;
    bool ret = false;
    auto page = getPageByFile(filePath);
    if (page == nullptr) {
        PageContext *context = d_DrawBoard()->_fileHander->loadDdf(filePath);
        setCurrentPage(addPage(context));
        ret = (context != nullptr);
    } else {
        setCurrentPage(page);
    }

    qWarning() << "load result = " << fileHander()->lastError() << fileHander()->lastErrorDescribe();
    auto error = fileHander()->lastError();
    if (error != FileHander::NoError && (FileHander::EUserCancelLoad_OldPen != error && FileHander::EUserCancelLoad_OldBlur != error)) {
        MessageDlg::execMessage(fileHander()->lastErrorDescribe(), ENormalMsg);

    }
    this->activateWindow();

    return ret;
}

bool DrawBoard::loadImage(const QString &file, bool adapt, bool changContexSizeToImag)
{
    auto filePath = file;
    bool ret = false;

    QImage img = d_DrawBoard()->_fileHander->loadImage(filePath);
    ret = !img.isNull();
    if (!img.isNull()) {
        QSize maxSize(10000, 10000);
        if (img.size().width() > maxSize.width() || img.size().height() > maxSize.height()) {
            MessageDlg::execMessage(QObject::tr("Import failed: no more than 10,000 pixels please"), EWarningMsg);
            return false;
        }
    }

    if (currentPage() == nullptr) {
        setCurrentPage(addPage(""));
        currentPage()->setPageRect(QRectF(QPoint(0, 0), img.size()));
    }

    if (changContexSizeToImag) {
        QRectF sceneRect = currentPage()->pageRect();
        QRectF newRect = sceneRect.adjusted(
                             (sceneRect.width() > img.width() ? 0 : (sceneRect.width() - img.width()) / 2),
                             (sceneRect.height() > img.height() ? 0 : (sceneRect.height() - img.height()) / 2),
                             (sceneRect.width() > img.width() ? 0 : -(sceneRect.width() - img.width()) / 2),
                             (sceneRect.height() > img.height() ? 0 : -(sceneRect.height() - img.height()) / 2)
                         );
        currentPage()->setPageRect(newRect);
    }

    auto currentContext = currentPage()->context();
    auto pos = currentContext->pageRect().center() - img.rect().center();

    QRectF rect = QRectF(QPointF(0, 0), img.size());
    bool bAddImg = true;
    if (adapt) {
        QFileInfo info(filePath);
        int r = -1;
        bAddImg = adaptImgPosAndRect(currentPage()->scene(), info.fileName(), img, pos, rect, r);
    }

    if (bAddImg) {
        currentContext->scene()->clearSelections();
        currentContext->addImageItem(img, pos, rect);
        currentPage()->setCurrentTool(selection);
    }



    qWarning() << "load result = " << fileHander()->lastError() << fileHander()->lastErrorDescribe();
    auto error = fileHander()->lastError();
    if (error != FileHander::NoError && (FileHander::EUserCancelLoad_OldPen != error && FileHander::EUserCancelLoad_OldBlur != error)) {
        MessageDlg::execMessage(fileHander()->lastErrorDescribe(), ENormalMsg);
    }
    this->activateWindow();

    return ret;
}

int DrawBoard::execPicturesLimit(int count)
{
    int ret = 0;
    int exitPicNum = 0;
    if (!currentPage())
        return ret;
    //获取已导入图片数量

    auto scence = currentPage()->view()->pageScene();

    if (scence != nullptr) {
        QList<QGraphicsItem *> items = scence->items();
        if (items.count() != 0) {
            for (int i = 0; i < items.size(); i++) {
                if (items[i]->type() == RasterItemType) {
                    RasterItem *layerItem = static_cast<RasterItem *>(items[i]);
                    if (layerItem->isBlurEnable()) {
                        exitPicNum = exitPicNum + 1;
                    }
                }
            }
        }
    }

    //大于30张报错，主要是适应各种系统环境，不给内存太大压力
    if (exitPicNum + count > 30) {
        Dtk::Widget::DDialog warnDlg(this);
        warnDlg.setIcon(QIcon::fromTheme("dialog-warning"));
        warnDlg.setTitle(tr("You can import up to 30 pictures, please try again!"));
        warnDlg.addButtons(QStringList() << tr("OK"));
        warnDlg.setDefaultButton(0);
        warnDlg.exec();
        ret = -1;
    }

    return  ret;
}
