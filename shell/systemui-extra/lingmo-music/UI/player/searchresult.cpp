#include "searchresult.h"
#define PT_11 11
#include <QScrollArea>
#include <QScrollBar>

#include <X11/keysym.h>
#include <X11/extensions/xtestconst.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XTest.h>

#include "UI/base/xatom-helper.h"
#include "lingmostylehelper/lingmostylehelper.h"
#include "windowmanager/windowmanager.h"
#include "UIControl/global/global.h"
#include "UI/globalsignal.h"

SearchResult::SearchResult(QWidget *parent) : QWidget(parent)
{
    this->setAutoFillBackground(true);
    this->setBackgroundRole(QPalette::Base);
    this->setWindowFlag(Qt::Tool);

    if (Global::isWayland) {
        kdk::LingmoUIStyleHelper::self()->removeHeader(this);
    }

    //窗体禁止拖动
    this->setProperty("useStyleWindowManager", false);
    vlayout = new QVBoxLayout(this);
    vlayout->setContentsMargins(4, 4, 4, 4);
    vlayout->setSpacing(0);
//    this->setLayout(vlayout);
    //歌曲名
    m_MusicLabel = new QLabel(tr("Music"),this);
    m_MusicLabel->setFixedHeight(36);
    m_MusicLabel->setIndent(28);
//    m_MusicLabel->setContentsMargins(32, 0, 0, 0);

    m_MusicView = new MusicSearchListview(this);
    m_MusicView->setSearchType(SearchType::TitleType);
    m_MusicView->setMinimumWidth(232);
//    m_MusicView->setContentsMargins(28, 0, 28, 0);

//    m_SingerFrame = new QFrame(this);
//    m_SingerFrame->setFrameShape(QFrame::HLine);
//    m_SingerFrame->setFrameShadow(QFrame::Raised);
//    m_SingerFrame->setLineWidth(2);

    //歌手
    m_SingerLabel = new QLabel(tr("Singer"),this);
    m_SingerLabel->setFixedHeight(36);
    m_SingerLabel->setIndent(28);
//    m_SingerLabel->setContentsMargins(32, 0, 0, 0);

    m_SingerView = new MusicSearchListview(this);
    m_SingerView->setSearchType(SearchType::SingerType);
    m_SingerView->setMinimumWidth(232);
//    m_SingerView->setContentsMargins(32, 0, 0, 0);

//    m_AlbumFrame = new QFrame(this);
//    m_AlbumFrame->setFrameShape(QFrame::HLine);
//    m_AlbumFrame->setFrameShadow(QFrame::Raised);
//    m_AlbumFrame->setLineWidth(2);

    //专辑
    m_AlbumLabel = new QLabel(tr("Album"),this);
    m_AlbumLabel->setFixedHeight(36);
    m_AlbumLabel->setIndent(28);
//    m_AlbumLabel->setContentsMargins(32, 0, 0, 0);

    m_AlbumView = new MusicSearchListview(this);
    m_AlbumView->setSearchType(SearchType::AlbumType);
    m_AlbumView->setMinimumWidth(232);
//    m_AlbumView->setContentsMargins(32, 0, 0, 0);

    m_MusicLabel->setStyleSheet("color:#8F9399;");
    m_SingerLabel->setStyleSheet("color:#8F9399;");
    m_AlbumLabel->setStyleSheet("color:#8F9399;");

    vlayout->addWidget(m_MusicLabel, 1);
    vlayout->addWidget(m_MusicView, 0);
//    vlayout->addWidget(m_SingerFrame);
    vlayout->addWidget(m_SingerLabel, 1);
    vlayout->addWidget(m_SingerView, 0);
//    vlayout->addWidget(m_AlbumFrame);
    vlayout->addWidget(m_AlbumLabel, 1);
    vlayout->addWidget(m_AlbumView, 0);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setFrameStyle(0);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget *vWid = new QWidget(this);
    vWid->setLayout(vlayout);
    scrollArea->setWidget(vWid);
    scrollArea->verticalScrollBar()->setProperty("drawScrollBarGroove",false);

    QVBoxLayout *sideLayout = new QVBoxLayout(this);
    sideLayout->addWidget(scrollArea);
    sideLayout->setMargin(0);
    sideLayout->setSpacing(0);
    this->setLayout(sideLayout);

    autoResize();

    connect(m_MusicView,&MusicSearchListview::clicked,this,&SearchResult::slotMusicItemClicked);
    connect(m_SingerView,&MusicSearchListview::clicked,this,&SearchResult::slotSingerItemClicked);
    connect(m_AlbumView,&MusicSearchListview::clicked,this,&SearchResult::slotAlbumItemClicked);
    connect(g_user_signal, &GlobalUserSignal::sigHideSearchResult, [this]{
            this->hide();
        });
}

SearchResult::~SearchResult()
{

}

void SearchResult::keyPressEvent(QKeyEvent *event)
{
//    m_searchEdit->raise();
    if (Global::isWayland) {

        kdk::LingmoUIStyleHelper::self()->removeHeader(this);
//        kdk::WindowManager::activateWindow(m_searchEdit->winId());

    } else {
//        m_searchEdit->activateWindow();
    }
//    QApplication::sendEvent(m_searchEdit,event);

    if (Global::isWayland) {
        QPoint point = m_searchEdit->mapToGlobal(QPoint(m_searchEdit->rect().topRight().x() - 10,m_searchEdit->rect().topRight().y() - 5));

    } else {
        setCursorWithXEvent();
    }


    if (event->key() == Qt::Key_Return) {
        this->hide();
        return;
    }

    if (event->key() == Qt::Key_Escape) {
        this->hide();
        return;
    }

    if (event->key() == Qt::Key_Backspace) {
        QApplication::sendEvent(m_searchEdit,event);
        Q_EMIT m_searchEdit->textChanged(m_searchEdit->text());
        return;
    }

    m_key = event->nativeVirtualKey();
    QTimer *timer = new QTimer;
    timer->setSingleShot(true);

    if (Global::isWayland) {

    } else {
        connect(timer,&QTimer::timeout,this,[=]{
            Display  *display = XOpenDisplay(NULL);
            XTestFakeKeyEvent(display, XKeysymToKeycode(display,m_key), 1, 10);
            XFlush(display);
            XTestFakeKeyEvent(display, XKeysymToKeycode(display,m_key), 0, 10);
            XFlush(display);
            XCloseDisplay(display);
        });

    }
    connect(timer,&QTimer::timeout,timer,&QTimer::deleteLater);
    timer->start(200);

//    Q_EMIT m_searchEdit->textChanged(m_searchEdit->text());
}

void SearchResult::setCursorWithXEvent()
{
    double device = QGuiApplication::primaryScreen()->devicePixelRatio();
    if (device <= 0) {
        device = 1;
    }

    QPoint point = m_searchEdit->mapToGlobal(QPoint(m_searchEdit->rect().topRight().x() - 10,m_searchEdit->rect().topRight().y() - 5));

    XEvent xEvent;
    //该值返回一个指向存储区 &xEvent 的指针。
    memset(&xEvent, 0, sizeof(XEvent));
    //默认显示
    Display *display = QX11Info::display();
    xEvent.type = ButtonPress;
    xEvent.xbutton.button = Button1;
    //this->effectiveWinId()本机父窗口系统标识符
    xEvent.xbutton.window = this->effectiveWinId();
    if (point.y() < 5) {
        xEvent.xbutton.x = point.x() * device;
        xEvent.xbutton.y = point.y() * device;
    } else {
        xEvent.xbutton.x = this->mapFromGlobal(point).x() * device;
        xEvent.xbutton.y = this->mapFromGlobal(point).y() * device;
    }
    xEvent.xbutton.x_root = point.x()* device;
    xEvent.xbutton.y_root = point.y()* device;
    xEvent.xbutton.display = display;

    XSendEvent(display,this->effectiveWinId(),False,ButtonPressMask,&xEvent);
    XFlush(display);
    XSendEvent(display,this->effectiveWinId(),False,ButtonReleaseMask,&xEvent);
    XFlush(display);
}

void SearchResult::autoResize()
{
    if (m_searchEdit != nullptr) {
        if (m_searchEdit->text() != "") {
            this->show();
//           this->raise();

            if (Global::isWayland) {
//                kdk::WindowManager::activateWindow(kdk::WindowManager::currentActiveWindow());
            } else {
//                this->activateWindow();
            }
        }
    }

    int lineHeight = 0;
    int lineShowCount = 0;
    if (m_MusicView->rowCount() == 0) {
        m_MusicLabel->hide();
        m_MusicView->hide();
    } else {
        m_MusicLabel->show();
        m_MusicView->show();
        lineShowCount++;
        m_MusicView->setFixedHeight(m_MusicView->rowCount() * 36);
    }

    if (m_SingerView->rowCount() == 0) {
        m_SingerLabel->hide();
        m_SingerView->hide();
    } else {
        m_SingerLabel->show();
        m_SingerView->show();
        lineShowCount++;
        m_SingerView->setFixedHeight(m_SingerView->rowCount() * 36);
    }

    if (m_AlbumView->rowCount() == 0) {
        m_AlbumLabel->hide();
        m_AlbumView->hide();
    } else {
        m_AlbumLabel->show();
        m_AlbumView->show();
        lineShowCount++;
        m_AlbumView->setFixedHeight(m_AlbumView->rowCount() * 36);
    }

    if (lineShowCount > 0) {
        lineHeight = 2 * (lineShowCount - 1);
    }

    int total = 0;
    total += m_MusicLabel->isVisible() ? m_MusicLabel->height() : 0;
    total += m_MusicView->isVisible() ? m_MusicView->height() : 0;
//    total += s_ArtistLine->isVisible() ? s_ArtistLine->height() : 0;
    total += m_SingerLabel->isVisible() ? m_SingerLabel->height() : 0;
    total += m_SingerView->isVisible() ? m_SingerView->height() : 0;
//    total += s_AblumLine->isVisible() ? s_AblumLine->height() : 0;
    total += m_AlbumLabel->isVisible() ? m_AlbumLabel->height() : 0;
    total += m_AlbumView->isVisible() ? m_AlbumView->height() : 0;
    total += lineHeight;

    if (m_MusicView->isHidden() && m_SingerView->isHidden() && m_AlbumLabel->isHidden()) {
        if (total > 500) {
            setFixedHeight(500);
        } else {
            setFixedHeight(total);
        }
    } else {
        if (total > 500) {
            setFixedHeight(500);
        } else {
            setFixedHeight(total + 28);
        }
    }
}

void SearchResult::setListviewSearchString(const QString &str)
{
    //搜索歌曲候选:<=5个
    m_MusicView->setSearchText(str);
    m_SingerView->setSearchText(str);
    m_AlbumView->setSearchText(str);
    m_Count = m_MusicView->rowCount()
              + m_AlbumView->rowCount()
              + m_SingerView->rowCount();
    m_CurrentIndex = -1;
}

void SearchResult::setLineEditSearchString(const QString &str)
{
//    if (m_searchEdit) {
//        m_searchEdit->blockSignals(true);
//        m_searchEdit->setText(str);
//        m_searchEdit->blockSignals(false);
//    }
}

void SearchResult::selectUp()
{
    if (m_Count == 0) {
        return;
    }
    if (m_CurrentIndex < 0) {
        return;
    }
    m_CurrentIndex --;
    if (m_CurrentIndex < m_MusicView->rowCount()) {
        m_MusicView->setCurrentIndexInt(m_CurrentIndex);
        m_SingerView->setCurrentIndexInt(-1);
        m_AlbumView->setCurrentIndexInt(-1);
    } else if (m_CurrentIndex >= m_MusicView->rowCount() - 1
               && m_CurrentIndex < (m_MusicView->rowCount() + m_SingerView->rowCount())) {
        m_SingerView->setCurrentIndexInt(m_CurrentIndex
                                         - m_MusicView->rowCount());
        m_MusicView->setCurrentIndexInt(-1);
        m_AlbumView->setCurrentIndexInt(-1);
    } else {
        m_AlbumView->setCurrentIndexInt(m_CurrentIndex
                                        - m_MusicView->rowCount()
                                        - m_SingerView->rowCount());
        m_MusicView->setCurrentIndexInt(-1);
        m_SingerView->setCurrentIndexInt(-1);
    }

    m_MusicView->update();
    m_SingerView->update();
    m_AlbumView->update();
}

void SearchResult::selectDown()
{
    if (m_Count == 0) {
        return;
    }
    if (m_CurrentIndex >= m_Count - 1) {
        return;
    }

    m_CurrentIndex ++;

    if (m_CurrentIndex < m_MusicView->rowCount()) {
        m_MusicView->setCurrentIndexInt(m_CurrentIndex);

        m_SingerView->setCurrentIndexInt(-1);
        m_AlbumView->setCurrentIndexInt(-1);

    } else if (m_CurrentIndex >= m_MusicView->rowCount() - 1
               && m_CurrentIndex < (m_MusicView->rowCount() + m_SingerView->rowCount())) {

        m_SingerView->setCurrentIndexInt(m_CurrentIndex
                                         - m_MusicView->rowCount());

        m_MusicView->setCurrentIndexInt(-1);
        m_AlbumView->setCurrentIndexInt(-1);

    } else {
        m_AlbumView->setCurrentIndexInt(m_CurrentIndex
                                        - m_MusicView->rowCount()
                                        - m_SingerView->rowCount());

        m_MusicView->setCurrentIndexInt(-1);
        m_SingerView->setCurrentIndexInt(-1);

        m_MusicView->clearSelection();
        m_SingerView->clearSelection();
    }

    m_MusicView->update();
    m_SingerView->update();
    m_AlbumView->update();
}

int SearchResult::getCurrentIndex()
{
    return m_CurrentIndex;
}

void SearchResult::setSearchEdit(SearchEdit *edit)
{
    m_searchEdit = edit;
}

bool SearchResult::nativeEvent(const QByteArray &eventType, void *message, long *result)
{

    Q_UNUSED(result);

    if (Global::isWayland) {
        return 0;
#if 0
        QRect rect(m_resultPosX, m_resultPosY, m_resultPosWidth, m_resultPosHeight);
        if(rect.contains(QCursor::pos(), false)) {
            return 0;
        } else {
            this->hide();
            return false;
        }
#endif
    }

    if(eventType != "xcb_generic_event_t")
    {
        return false;
    }

    xcb_generic_event_t *event = (xcb_generic_event_t*)message;
    switch (event->response_type & ~0x80)
    {
        case XCB_FOCUS_OUT:
            QRect rect(m_resultPosX, m_resultPosY, m_resultPosWidth, m_resultPosHeight);
            if(rect.contains(QCursor::pos(), false))
            {
                return 0;
            }
            else
            {
                this->hide();
                break;
            }
    }
    return false;
}

void SearchResult::changeSrearchResultPos(int posX, int posY, int width, int height)
{
    m_resultPosX = posX;
    m_resultPosY = posY;
    m_resultPosWidth = width;
    m_resultPosHeight = height;

    qDebug() << "m_resultPos(x, y, width, height) = "
             << m_resultPosX << m_resultPosY
             << m_resultPosWidth << m_resultPosHeight;
}

void SearchResult::onReturnPressed()
{
    if (m_MusicView->getIndexInt() >= 0) {
        m_MusicView->onReturnPressed();
    } else if (m_SingerView->getIndexInt() >= 0) {
        m_SingerView->onReturnPressed();
    } else if (m_AlbumView->getIndexInt() >= 0) {
        m_AlbumView->onReturnPressed();
    }
}

void SearchResult::slotLableSetFontSize(int size)
{
    //默认大小12px,换算成pt为9
    double lableBaseFontSize = PT_11;//魔鬼数字，自行处理
    double nowFontSize = lableBaseFontSize * double(size) / 11;//11为系统默认大小，魔鬼数字，自行处理
    QFont font;
    font.setPointSizeF(nowFontSize);
    m_MusicLabel->setFont(font);
    m_SingerLabel->setFont(font);
    m_AlbumLabel->setFont(font);
}

void SearchResult::slotMusicItemClicked(QModelIndex index)
{
    this->hide();
    musicDataStruct metaPtr = index.data(Qt::UserRole + SearchType::TitleType).value<musicDataStruct>();

    Q_EMIT signalFilePath(metaPtr.filepath);
}

void SearchResult::slotSingerItemClicked(QModelIndex index)
{
    this->hide();
    musicDataStruct metaPtr = index.data(Qt::UserRole + SearchType::SingerType).value<musicDataStruct>();

    Q_EMIT signalSongListBySinger(metaPtr.singer);
}

void SearchResult::slotAlbumItemClicked(QModelIndex index)
{
    this->hide();
    musicDataStruct metaPtr = index.data(Qt::UserRole + SearchType::AlbumType).value<musicDataStruct>();

    Q_EMIT signalSongListByAlbum(metaPtr.album);
}
