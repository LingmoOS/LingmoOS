// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "platform_playlist_widget.h"
#include "playlist_model.h"
#include "compositing_manager.h"
#include "player_engine.h"
#include "platform_toolbox_proxy.h"
#include "actions.h"
#include "platform/platform_mainwindow.h"
#include "utils.h"
#include "movieinfo_dialog.h"
#include "tip.h"

#include <DApplication>
#include <dimagebutton.h>
#include <dthememanager.h>
#include <DScrollBar>
#include "../accessibility/ac-deepin-movie-define.h"

#define PLAYLIST_FIXED_WIDTH 800
#define POPUP_DURATION 350

namespace dmr {
class Platform_PlayItemTooltipHandler: public QObject
{
public:
    explicit Platform_PlayItemTooltipHandler(QObject *parent): QObject(parent) {}

protected:
    bool eventFilter(QObject *obj, QEvent *event)
    {
        switch (event->type()) {
        case QEvent::ToolTip: {
            QHelpEvent *he = static_cast<QHelpEvent *>(event);
            Tip *tip = obj->property("HintWidget").value<Tip *>();
            QWidget *item = tip->property("for").value<QWidget *>();
            DLabel *lb = tip->findChild<DLabel *>("TipText");
            if (tip->isVisible())
                return true; //tip弹出后不再更新位置
            lb->setAlignment(Qt::AlignLeft);
            tip->update();
            tip->show();
            tip->adjustSize();
            tip->raise();
            QPoint pos = he->globalPos() + QPoint{0, 10};
            int dw = qApp->desktop()->availableGeometry(item).width();
            if (pos.x() + tip->width() > dw) {
                pos.rx() = dw - tip->width();
            }
            tip->move(pos);
            return true;
        }

        case QEvent::Leave: {
            auto tip = obj->property("HintWidget").value<Tip *>();
            tip->hide();
            event->ignore();
            break;
        }
        default:
            break;
        }
        return QObject::eventFilter(obj, event); // standard event processing
    }
};

enum ItemState {
    Normal,
    Playing,
    Invalid, // gets deleted or similar
};

class Platform_PlayItemWidget: public QFrame
{
    Q_OBJECT
public:
    friend class Platform_PlaylistWidget;

    Platform_PlayItemWidget(const PlayItemInfo &pif, QListWidget *list = nullptr, int index = 0, Platform_PlaylistWidget *parent = nullptr)
        : QFrame(), _pif {pif}, _listWidget {list}, _playlist{parent}
    {
        _thumb = nullptr;
        m_pSvgWidget = nullptr;
        setProperty("PlayItemThumb", "true");
        setFrameShape(QFrame::NoFrame);
        this->setObjectName(PLAYITEM_WIDGET);
        auto kd = "local";
        if (!_pif.url.isLocalFile()) {
            if (_pif.url.scheme().startsWith("dvd")) {
                kd = "dvd";
            } else {
                kd = "network";
            }
        }
        setProperty("ItemKind", kd);

        // it's the same for all themes
        _play = QPixmap(":/resources/icons/dark/normal/film-top.svg");
        _play.setDevicePixelRatio(qApp->devicePixelRatio());

        setFixedSize(324, 40);
        QHBoxLayout *l = new QHBoxLayout(this);
        l->setContentsMargins(10, 0, 0, 0);
        l->setSpacing(0);
        setLayout(l);

        _index = new DLabel(this);
        DFontSizeManager::instance()->bind(_index, DFontSizeManager::T9);
        _index->setText(QString::number(index + 1));
        _index->setFixedWidth(21);
        l->addWidget(_index);
        QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &Platform_PlayItemWidget::slotThemeTypeChanged);

        _name = new DLabel(this);
        _name->setProperty("Name", true);
        _name->setFrameShape(QFrame::NoFrame);
        _name->setTextInteractionFlags(Qt::NoTextInteraction);
        _name->installEventFilter(this);
        _name->setAutoFillBackground(false);

        l->addWidget(_name);

        _time = new DLabel(this);
        DFontSizeManager::instance()->bind(_time, DFontSizeManager::T9);
        _time->setProperty("Time", true);
        _time->setText(_pif.mi.durationStr());
        if (!_pif.valid) {
            setState(ItemState::Invalid);
            _time->setText(tr("The file does not exist"));
        }
        l->addStretch();
        l->addWidget(_time);

        _closeBtn = new DFloatingButton(DStyle::SP_CloseButton, this);
        _closeBtn->setFocusPolicy(Qt::NoFocus);
        _closeBtn->setObjectName(PLAYITEN_CLOSE_BUTTON);
        _closeBtn->setAccessibleName(PLAYITEN_CLOSE_BUTTON);
        _closeBtn->setIconSize(QSize(28, 28));
        _closeBtn->setFixedSize(25, 25);
        _closeBtn->hide();
        connect(_closeBtn, &DFloatingButton::clicked, this, &Platform_PlayItemWidget::closeButtonClicked);

        setToolTip(_pif.mi.title);
        auto th = new Platform_PlayItemTooltipHandler(this);
        auto t = new Tip(QPixmap(), _pif.mi.title, nullptr);
        t->setWindowFlags(Qt::ToolTip | Qt::CustomizeWindowHint);
        t->setText(_pif.mi.title);
        t->resetSize(QApplication::desktop()->availableGeometry().width());
        t->setAttribute(Qt::WA_TranslucentBackground);
        t->setProperty("for", QVariant::fromValue<QWidget *>(this));
        t->layout()->setContentsMargins(5, 10, 5, 10);
        t->hide();
        setProperty("HintWidget", QVariant::fromValue<QWidget *>(t));
        if (!utils::check_wayland_env())
            installEventFilter(th);

        m_opacityEffect = new QGraphicsOpacityEffect;
        _time->setGraphicsEffect(m_opacityEffect);
        m_opacityEffect_1 = new QGraphicsOpacityEffect;
        _index->setGraphicsEffect(m_opacityEffect_1);
        setState(ItemState::Normal);
    }
    
    ~Platform_PlayItemWidget() override
    {
        if (m_pSvgWidget) {
            delete  m_pSvgWidget;
            m_pSvgWidget = nullptr;
        }

        if (_thumb) {
            delete _thumb;
            _thumb = nullptr;
        }
    }

    void updateInfo(const PlayItemInfo &pif)
    {
        _pif = pif;
        _time->setText(_pif.mi.durationStr());
        setToolTip(_pif.mi.title);
        updateNameText();

        if (!_pif.valid) {
            setState(ItemState::Invalid);
            _time->setText(tr("The file does not exist"));
        }
        update();
    }

    void setState(ItemState is)
    {
        setProperty("ItemState", is);
        updateForeground();
    }

    ItemState state() const
    {
        return static_cast<ItemState>(property("ItemState").toInt());
    }
    void setIndex(int index)
    {
        _index->setText(QString::number(index + 1));
    }

    void setHovered(bool v)
    {
        if (_hovered != v) {
            _hovered = v;
            setProperty("hovered", v);
        }
    }

    void setCurItemHovered(bool v)
    {
        if (_hovered != v) {
            _hovered = v;
            setProperty("hovered", v);
        }

        if (v) {
            _closeBtn->show();
            _closeBtn->raise();
        } else {
            _closeBtn->hide();
        }

        updateClosePosition();
        update();
    }

    bool getBIsSelect() const
    {
        return m_bIsSelect;
    }

    void setBIsSelect(bool bIsSelect)
    {
        m_bIsSelect = bIsSelect;

        updateForeground();
    }

    void doDoubleClick()
    {
        //FIXME: there is an potential inconsistency with model if pif did changed
        //(i.e gets deleted).
        _pif.refresh();
        _time->setText(_pif.mi.durationStr());
        if (!_pif.valid) {
            setState(ItemState::Invalid);
            _time->setText(tr("The file does not exist"));
        }
        if (!_pif.url.isLocalFile() || _pif.info.exists()) {
            emit doubleClicked();
        }
    }

signals:
    void closeButtonClicked();
    void doubleClicked();

private slots:
    void slotThemeTypeChanged()
    {
        updateForeground();
    }
    void slotSizeChange()
    {
        setFixedWidth(_playlist->width());
    }

protected:
    void updateClosePosition()
    {
        auto margin = 10;
        _closeBtn->move(width() - _closeBtn->width() - margin,
                        (height() - _closeBtn->height()) / 2);
    }
    void updateForeground()
    {
        m_highlightColor = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color();

        if(m_bIsSelect) {
            _name->setForegroundRole(DPalette::Text);
            _index->setForegroundRole(DPalette::Text);

            QPalette pa;
            pa.setColor(QPalette::Text, Qt::white);
            _name->setPalette(pa);
            _index->setPalette(pa);
            update();
        } else {
            if (state() == ItemState::Playing) {
                _name->setForegroundRole(DPalette::Highlight);
                _index->setForegroundRole(DPalette::Highlight);
                _time->setForegroundRole(DPalette::Highlight);
            } else {
                _name->setForegroundRole(DPalette::BrightText);
                _index->setForegroundRole(DPalette::BrightText);
                _time->setForegroundRole(DPalette::BrightText);

                QPalette pa;
                if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
                    pa.setColor(QPalette::BrightText, Qt::black);
                    _name->setPalette(pa);
                    _index->setPalette(pa);
                } else if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
                    pa.setColor(QPalette::BrightText, Qt::white);
                    _name->setPalette(pa);
                    _index->setPalette(pa);
                }
            }
        }
    }
    void leaveEvent(QEvent *e) override
    {
        _closeBtn->hide();
        setHovered(false);

        QFrame::leaveEvent(e);
    }
    void enterEvent(QEvent *e) override
    {
        _closeBtn->show();
        _closeBtn->raise();

        updateClosePosition();
        setHovered(true);

        QFrame::enterEvent(e);
    }
    bool eventFilter(QObject *obj, QEvent *e) override
    {
        if (e->type() == QEvent::MouseButtonDblClick) {
            doDoubleClick();
            return true;
        }
        return QWidget::eventFilter(obj, e);
    }
    void resizeEvent(QResizeEvent *re) override
    {
        updateClosePosition();
        updateNameText();

        QFrame::resizeEvent(re);
    }
    bool event(QEvent *ee) override
    {
        if (ee->type() == QEvent::Resize) {
            _name->setFixedHeight(36);
        }

        if (ee->type() == QEvent::Move) {
        }

        return QFrame::event(ee);
    }
    void updateNameText()
    {
        _name->setText(utils::ElideText(_pif.mi.title, {width() - 104, 36}, QTextOption::NoWrap,
                                        _name->font(), Qt::ElideRight, 18, width() - 104));
        _name->setCursor(Qt::ArrowCursor);
        _name->setFixedHeight(36);
    }
    void showEvent(QShowEvent *se) override
    {
        updateNameText();
        QFrame::showEvent(se);
    }
    void mouseDoubleClickEvent(QMouseEvent *me) override
    {
        doDoubleClick();

        QFrame::mouseDoubleClickEvent(me);
    }
    void paintEvent(QPaintEvent *pe) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        QRectF bgRect;
        bgRect.setSize(size());
        const DPalette pal = QGuiApplication::palette();

        if (!(_index->text().toInt() % 2)) {
            QColor bgColor;
            if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
                bgColor = QColor(255, 255, 255);
                bgColor.setAlphaF(0.05);
            } else {
                bgColor = pal.color(DPalette::AlternateBase);
            }
            QPainterPath pp;
            pp.addRoundedRect(bgRect, 8, 8);
            painter.fillPath(pp, bgColor);
        }

        if (_hovered) {
            DPalette pa = DApplicationHelper::instance()->palette(this);
            pa.setBrush(DPalette::Text, pa.color(DPalette::Highlight));
            QColor bgColor(255, 255, 255, static_cast<int>(255 * 0.05));
            if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
                bgColor = QColor(0, 0, 0, static_cast<int>(255 * 0.05));
            }

            QPainterPath pp;
            pp.addRoundedRect(bgRect, 8, 8);
            painter.fillPath(pp, bgColor);

        }

        if (!_pif.valid) {
            setState(ItemState::Invalid);
            _name->setForegroundRole(DPalette::TextTips);
            _time->setText(tr("The file does not exist"));
        }

        if (m_bIsSelect) {
            _time->hide();
            _closeBtn->show();
            _closeBtn->raise();
            QPainterPath pp;
            pp.addRoundedRect(bgRect, 8, 8);
            painter.fillPath(pp, m_highlightColor);
        } else {
        _time->show();
        _closeBtn->hide();
    }
        QFrame::paintEvent(pe);
    }

private:
    QString _bg;
    DLabel *_index;
    Platform_ListPic *_thumb;
    QSvgWidget *m_pSvgWidget;
    DLabel *_name;
    DLabel *_time;
    QPixmap _play;
    PlayItemInfo _pif;
    DFloatingButton *_closeBtn;
    QListWidget *_listWidget {nullptr};
    bool _hovered {false};
    Platform_PlaylistWidget *_playlist{nullptr};
    bool m_bIsSelect = false;
    QGraphicsOpacityEffect *m_opacityEffect {nullptr};
    QGraphicsOpacityEffect *m_opacityEffect_1 {nullptr};
    QColor m_highlightColor;
};

class Platform_MainWindowListener: public QObject
{
public:
    explicit Platform_MainWindowListener(QObject *parent): QObject(parent) {}

protected:
    bool eventFilter(QObject *obj, QEvent *event)
    {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);

            if (me->buttons() == Qt::LeftButton) {
                if (!m_bClicked) {
                    m_bClicked = true;
                    m_lastPoint = me->globalPos();

                    QTimer::singleShot(200, this, [ = ] {
                        if (!m_bClicked) {
                            return;
                        }
                        m_bClicked = false;
                        auto *plw = dynamic_cast<Platform_PlaylistWidget *>(parent());
                        auto *mw = dynamic_cast<Platform_MainWindow *>(plw->parent());

                        if (mw->insideResizeArea(m_lastPoint))
                            return;
                        if (plw->state() == Platform_PlaylistWidget::Opened && !plw->underMouse()) {
                            mw->requestAction(ActionFactory::ActionKind::TogglePlaylist);
                        }
                    });
                }
            }
            return false;
        } else if (event->type() == QEvent::MouseButtonDblClick) {
            m_bClicked = false;
            return QObject::eventFilter(obj, event);
        }/* else if (event->type() == QEvent::KeyRelease) { //这段代码会导致Enter播放列表后，播放列表关闭
            QKeyEvent *key = static_cast<QKeyEvent *>(event);
            if (key->key() == Qt::Key_Return || key->key() == Qt::Key_Enter) {
                auto *plw = dynamic_cast<PlaylistWidget *>(parent());

                if (plw->state() == PlaylistWidget::State::Opened) {
                    DListWidget *playlist = plw->get_playlist();
                    for (int loop = 0; loop < playlist->count(); loop++) {
                        auto piw = dynamic_cast<PlayItemWidget *>(playlist->itemWidget(playlist->item(loop)));
                        if (piw->getBIsSelect()) {
                            piw->doubleClicked();
                            return false;
                        }
                    }

                }
            }
            return false;
        }*/ else {
            // standard event processing
            return QObject::eventFilter(obj, event);
        }
    }

private:
    bool m_bClicked {false};
    QPoint m_lastPoint;
};

/**
 * @brief 获取播放列表内的所有鼠标事件
 *
*/
class Platform_MouseEventListener: public QObject
{
public:
    explicit Platform_MouseEventListener(QObject *parent): QObject(parent) {}
    void setListWidget(DListWidget* listWidget)
    {
        m_pListWidget = listWidget;
    }

protected:
    bool eventFilter(QObject *obj, QEvent *event)
    {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *pMouseEvent = static_cast<QMouseEvent *>(event);
            if (pMouseEvent->buttons() == Qt::LeftButton && !m_pListWidget->itemAt(pMouseEvent->pos())) {
                Platform_PlayItemWidget *pItem = reinterpret_cast<Platform_PlayItemWidget *>(m_pListWidget->itemWidget(m_pListWidget->currentItem()));
                if(pItem)
                {
                    pItem->setBIsSelect(false); // 点击播放列表空白处，取消item选中效果
                    m_pListWidget->update();
                }
            }
        }

        return QObject::eventFilter(obj, event);
    }
private:
    DListWidget* m_pListWidget;
};

Platform_PlaylistWidget::Platform_PlaylistWidget(QWidget *mw, PlayerEngine *mpv)
    : QWidget(mw), _engine(mpv), _mw(static_cast<Platform_MainWindow *>(mw))
{
    bool composited = CompositingManager::get().composited();
    setAttribute(Qt::WA_TranslucentBackground, false);
    this->setObjectName(PLAYLIST_WIDGET);

    paOpen = nullptr;
    paClose = nullptr;
    pSelectItemWgt = nullptr;

    setFixedWidth(344);
    QVBoxLayout *mainVLayout = new QVBoxLayout(this);
    mainVLayout->setContentsMargins(0, 20, 0, 0);
    mainVLayout->setSpacing(0);
    setLayout(mainVLayout);

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->setContentsMargins(0, 0, 20, 0);
    topLayout->setSpacing(0);

    _title = new DLabel();
    DFontSizeManager::instance()->bind(_title, DFontSizeManager::T4);
    _title->setForegroundRole(DPalette::ToolTipText);
    _title->setText(tr("Playlist"));
    _title->setContentsMargins(20, 0, 0, 0);

    _num = new DLabel();
    _num->setForegroundRole(DPalette::BrightText);
    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect;
    _num->setGraphicsEffect(opacityEffect);
    opacityEffect->setOpacity(0.5);
    _num->setText("");
    DFontSizeManager::instance()->bind(_num, DFontSizeManager::T6);
    _num->setContentsMargins(20, 0, 0, 0);

    m_pClearButton = new DToolButton();
    m_pClearButton->setIcon(QIcon::fromTheme("dcc_clearlist"));
    m_pClearButton->setObjectName(CLEAR_PLAYLIST_BUTTON);
    m_pClearButton->setAccessibleName(CLEAR_PLAYLIST_BUTTON);
    m_pClearButton->setFocusPolicy(Qt::TabFocus);
    m_pClearButton->installEventFilter(this);
    DFontSizeManager::instance()->bind(m_pClearButton, DFontSizeManager::T6);
    connect(m_pClearButton, &DToolButton::clicked, _engine, &PlayerEngine::clearPlaylist);

    leftLayout->addWidget(_title);
    leftLayout->addStretch(5);
    leftLayout->addWidget(_num);
    leftLayout->addStretch(10);
    topLayout->addLayout(leftLayout);
    topLayout->addWidget(m_pClearButton);
    mainVLayout->addLayout(topLayout);

    _playlist = new DListWidget();
    _playlist->setAttribute(Qt::WA_DeleteOnClose);
    _playlist->setFixedSize(334, 440);
    _playlist->setContentsMargins(0, 0, 0, 0);
    _playlist->installEventFilter(this);
    _playlist->viewport()->setAutoFillBackground(false);
    _playlist->setAutoFillBackground(false);
    _playlist->setObjectName(PLAYLIST);
    _playlist->viewport()->setObjectName(FILE_LIST);
    _playlist->setParent(this);

    mainVLayout->addWidget(_playlist);
    _playlist->setFrameShape(QFrame::NoFrame);
    _playlist->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
    _playlist->setSelectionMode(QListView::NoSelection);
    _playlist->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _playlist->setResizeMode(QListView::Adjust);
    _playlist->setDragDropMode(QListView::InternalMove);
    _playlist->setSpacing(0);
    _playlist->viewport()->setAcceptDrops(true);
    _playlist->setDragEnabled(true);

    Platform_MouseEventListener* pListener = new Platform_MouseEventListener(this);
    pListener->setListWidget(_playlist);
    _playlist->viewport()->installEventFilter(pListener);
    this->installEventFilter(pListener);

    connect(_playlist, &DListWidget::itemClicked, this, &Platform_PlaylistWidget::slotShowSelectItem);
    connect(_playlist, &DListWidget::currentItemChanged, this, &Platform_PlaylistWidget::OnItemChanged);

    if (!composited) {
        _playlist->setWindowFlags(Qt::FramelessWindowHint | Qt::BypassWindowManagerHint);
        _playlist->setAttribute(Qt::WA_NativeWindow);
    }

    setTabOrder(m_pClearButton, _playlist);

#ifndef USE_DXCB
    auto *mwl = new Platform_MainWindowListener(this);
    mw->installEventFilter(mwl);
#endif

    if (!_closeMapper) {
        _closeMapper = new QSignalMapper(this);
        connect(_closeMapper,
                static_cast<void(QSignalMapper::*)(QWidget *)>(&QSignalMapper::mapped), this, &Platform_PlaylistWidget::slotCloseItem);

    }

    if (!_activateMapper) {
        _activateMapper = new QSignalMapper(this);
        connect(_activateMapper,
                static_cast<void(QSignalMapper::*)(QWidget *)>(&QSignalMapper::mapped), this, &Platform_PlaylistWidget::slotDoubleClickedItem);

    }

    connect(&_engine->playlist(), &PlaylistModel::emptied, this, &Platform_PlaylistWidget::clear);
    connect(&_engine->playlist(), &PlaylistModel::itemsAppended, this, &Platform_PlaylistWidget::appendItems);
    connect(&_engine->playlist(), &PlaylistModel::itemRemoved, this, &Platform_PlaylistWidget::removeItem);
    connect(&_engine->playlist(), &PlaylistModel::currentChanged, this, &Platform_PlaylistWidget::updateItemStates);
    connect(&_engine->playlist(), &PlaylistModel::itemInfoUpdated, this, &Platform_PlaylistWidget::updateItemInfo);

    QTimer::singleShot(10, this, &Platform_PlaylistWidget::loadPlaylist);

    connect(ActionFactory::get().playlistContextMenu(), &DMenu::aboutToShow, [ = ]() {
        QTimer::singleShot(20, [ = ]() {
            if (_mouseItem) {
                _clickedItem = _mouseItem;
                (static_cast<Platform_PlayItemWidget *>(_mouseItem))->setHovered(true);
            }
        });
    });
    connect(ActionFactory::get().playlistContextMenu(), &DMenu::aboutToHide, [ = ]() {
        if (_mouseItem) {
            (static_cast<Platform_PlayItemWidget *>(_mouseItem))->setHovered(false);
        }
    });

    connect(_playlist->model(), &QAbstractItemModel::rowsMoved, this, &Platform_PlaylistWidget::slotRowsMoved);
}

Platform_PlaylistWidget::~Platform_PlaylistWidget()
{
}

void Platform_PlaylistWidget::updateSelectItem(const int key)
{
    auto curItem = _playlist->currentItem();
    auto curRow = _playlist->row(curItem);
    qInfo() << "prevRow..." << curRow;
    Platform_PlayItemWidget *prevItemWgt = nullptr;
    if (curItem) {
        prevItemWgt = reinterpret_cast<Platform_PlayItemWidget *>(_playlist->itemWidget(curItem));
    }

    if (key == Qt::Key_Up) {
        if (curRow == -1) {
            _index = curRow + 1;
        } else {
            _index = curRow - 1;
        }
        if (_index < 0) {
            return;
        }

        _playlist->setCurrentRow(_index);
        qInfo() << "Enter Key_Up..." << _index;
        auto curItemWgt = reinterpret_cast<Platform_PlayItemWidget *>(_playlist->itemWidget(_playlist->item(_index)));
        if (prevItemWgt) {
            prevItemWgt->setBIsSelect(false);
        }
        if (curItemWgt) {
            curItemWgt->setBIsSelect(true);
        }

    } else if (key == Qt::Key_Down) {
        if (_index >= _playlist->count() - 1) {
            return;
        }
        _index = curRow + 1;
        _playlist->setCurrentRow(_index);
        qInfo() << "Enter Key_Down..." << _index;
        auto curItemWgt = reinterpret_cast<Platform_PlayItemWidget *>(_playlist->itemWidget(_playlist->item(_index)));
        if (prevItemWgt) {
            prevItemWgt->setBIsSelect(false);
        }
        if (curItemWgt) {
            curItemWgt->setBIsSelect(true);
        }
    } else if (key == Qt::Key_Enter || key == Qt::Key_Return) {
        if (m_pClearButton == focusWidget()) {   //focus在清空按钮上则清空列表
            _engine->clearPlaylist();
        } else {
            slotDoubleClickedItem(prevItemWgt);  //Enter键播放
        }
    }
}

void Platform_PlaylistWidget::clear()
{
    _playlist->clear();
    QString s = QString(tr("%1 videos")).arg(_playlist->count());
    _num->setText(s);
    _engine->getplaylist()->clearLoad();
}

void Platform_PlaylistWidget::updateItemInfo(int id)
{
    auto piw = dynamic_cast<Platform_PlayItemWidget *>(_playlist->itemWidget(_playlist->item(id)));
    if (piw == nullptr)     //update info thx
        return ;
    piw->updateInfo(_engine->playlist().items()[id]);
}

void Platform_PlaylistWidget::updateItemStates()
{
    qInfo() << __func__ << _playlist->count() << "current = " << _engine->playlist().current();

    for (int i = 0; i < _playlist->count(); i++) {
        auto piw = dynamic_cast<Platform_PlayItemWidget *>(_playlist->itemWidget(_playlist->item(i)));

        auto old = piw->state();
        piw->setState(ItemState::Normal);
        if (!piw->_pif.valid) {
            piw->setState(ItemState::Invalid);
        }

        if (i == _engine->playlist().current()) {
            if (piw->state() != ItemState::Playing) {
                //scrollToItem只能更新scroll位置，不能同步列表项
                //_playlist->scrollToItem(_playlist->item(i));
                _playlist->setCurrentRow(i);
                piw->setState(ItemState::Playing);

            }
        }
    }
}

void Platform_PlaylistWidget::showItemInfo()
{
    if (!_mouseItem) return;
    auto item = dynamic_cast<Platform_PlayItemWidget *>(_mouseItem);
    if (item) {
        MovieInfoDialog mid(item->_pif, _mw);
        mid.exec();
    }
}

void Platform_PlaylistWidget::openItemInFM()
{
    if (!_mouseItem) return;
    auto item = dynamic_cast<Platform_PlayItemWidget *>(_mouseItem);
    if (item) {
        utils::ShowInFileManager(item->_pif.mi.filePath);
    }
}

void Platform_PlaylistWidget::removeClickedItem(bool isShortcut)
{
    if (isShortcut && isVisible()) {
        for (int i = 0; i < _playlist->count(); i++) {
            Platform_PlayItemWidget * piw = dynamic_cast<Platform_PlayItemWidget *>(_playlist->itemWidget(_playlist->item(i)));
            qInfo() << piw->getBIsSelect();
            if (piw->getBIsSelect()) {
                _engine->playlist().remove(i);
                return;
            }
        }
    }

    if (!_clickedItem) return;
    Platform_PlayItemWidget * piw = dynamic_cast<Platform_PlayItemWidget *>(_clickedItem);
    if (piw) {
        qInfo() << __func__;
        for (int i = 0; i < _playlist->count(); i++) {
            if (_clickedItem == _playlist->itemWidget(_playlist->item(i))) {
                _engine->playlist().remove(i);
                break;
            }
        }
    }
}

void Platform_PlaylistWidget::slotCloseTimeTimeOut()
{
    QTimer *pCloselistTimer = dynamic_cast<QTimer *>(sender());
    pCloselistTimer->deleteLater();
    togglePopup(false);
    _mw->reflectActionToUI(ActionFactory::TogglePlaylist);
}

void Platform_PlaylistWidget::slotCloseItem(QWidget *w)
{
    qInfo() << "item close clicked";
    _clickedItem = w;
    _mw->requestAction(ActionFactory::ActionKind::PlaylistRemoveItem);
}

void Platform_PlaylistWidget::slotDoubleClickedItem(QWidget *w)
{
    qInfo() << "item double clicked";
    QList<QVariant> args;
    for (int i = 0; i < _playlist->count(); i++) {
        if (w == _playlist->itemWidget(_playlist->item(i))) {
            args << i;
            _mw->requestAction(ActionFactory::ActionKind::GotoPlaylistSelected,
                               false, args);

            QTimer *closelistTImer = new QTimer;
            closelistTImer->start(500);
            connect(closelistTImer, &QTimer::timeout, this, &Platform_PlaylistWidget::slotCloseTimeTimeOut);
            break;
        }
    }
}

void Platform_PlaylistWidget::slotRowsMoved()
{
    if (_lastDragged.first >= 0) {
        int target = -1;
        for (int i = 0; i < _playlist->count(); i++) {
            auto piw = dynamic_cast<Platform_PlayItemWidget *>(_playlist->itemWidget(_playlist->item(i)));
            if (piw == _lastDragged.second) {
                target = i;
                break;
            }
        }
        qInfo() << "swap " << _lastDragged.first << target;
        if (target >= 0 && _lastDragged.first != target) {
            _engine->playlist().switchPosition(_lastDragged.first, target);
            _lastDragged = {-1, nullptr};
        }
    }
}

/*void PlaylistWidget::dragEnterEvent(QDragEnterEvent *ev)
{
    auto md = ev->mimeData();
    qInfo() << md->formats();
    if (md->formats().contains("application/x-qabstractitemmodeldatalist")) {
        if (!_playlist->selectedItems().contains(_playlist->itemAt(ev->pos()))) {
            _playlist->setDropIndicatorShown(true);
        }
        QWidget::dragEnterEvent(ev);
        return;
    }

    if (ev->mimeData()->hasUrls()) {
        ev->acceptProposedAction();
    }
}*/

/*void PlaylistWidget::dragMoveEvent(QDragMoveEvent *ev)
{
    auto md = ev->mimeData();
    if (md->formats().contains("application/x-qabstractitemmodeldatalist")) {
        if (!_playlist->selectedItems().contains(_playlist->itemAt(ev->pos()))) {
            _playlist->setDropIndicatorShown(true);
        }
        QWidget::dragMoveEvent(ev);
        return;
    }

    if (ev->mimeData()->hasUrls()) {
        ev->acceptProposedAction();
    }
}*/

/*void PlaylistWidget::dropEvent(QDropEvent *ev)
{
    auto md = ev->mimeData();
    if (md->formats().contains("application/x-qabstractitemmodeldatalist")) {
        _playlist->setDropIndicatorShown(false);
        auto encoded = md->data("application/x-qabstractitemmodeldatalist");
        QDataStream stream(&encoded, QIODevice::ReadOnly);

        QList<int> l;
        while (!stream.atEnd()) {
            int row, col;
            QMap<int,  QVariant> roleDataMap;
            stream >> row >> col >> roleDataMap;
            auto piw = dynamic_cast<PlayItemWidget *>(_playlist->itemWidget(_playlist->item(row)));
            _lastDragged = qMakePair(row, piw);
            qInfo() << "drag to move " << row << piw->_pif.url;
        }

        QWidget::dropEvent(ev);
        return;
    }

    if (!ev->mimeData()->hasUrls()) {
        return;
    }
    auto urls = ev->mimeData()->urls();
    _engine->addPlayFiles(urls);

    ev->acceptProposedAction();
}*/

void Platform_PlaylistWidget::contextMenuEvent(QContextMenuEvent *cme)
{
    bool on_item = false;
    _mouseItem = nullptr;
    QPoint globaPoint = _playlist->mapFromParent(cme->pos());

    if (_playlist->itemAt(globaPoint)) {
        _mouseItem = _playlist->itemWidget(_playlist->itemAt(globaPoint));
        on_item = true;
    }

    if (CompositingManager::get().isPadSystem()) {
        if (pSelectItemWgt) {
            pSelectItemWgt->setBIsSelect(false);
        }
        auto piw = dynamic_cast<Platform_PlayItemWidget *>(_mouseItem);
        piw->setBIsSelect(true);
        pSelectItemWgt = piw;
    } else {
        auto piw = dynamic_cast<Platform_PlayItemWidget *>(_mouseItem);
        auto menu = ActionFactory::get().playlistContextMenu();
        for (auto act : menu->actions()) {
            auto prop = static_cast<ActionFactory::ActionKind>(act->property("kind").toInt());
            bool on = true;
            if (prop == ActionFactory::ActionKind::PlaylistOpenItemInFM) {
                on = on_item && piw->_pif.valid && piw->_pif.url.isLocalFile();
            } else if (prop == ActionFactory::ActionKind::PlaylistRemoveItem) {
                on = on_item;
            } else if (prop == ActionFactory::ActionKind::PlaylistItemInfo) {
                on = on_item && piw->_pif.valid;
            } else {
                on = _playlist->count() > 0 ? true : false;
            }
            act->setEnabled(on);
        }

        ActionFactory::get().playlistContextMenu()->popup(cme->globalPos());
    }
#ifdef USE_TEST
    ActionFactory::get().playlistContextMenu()->hide();
    ActionFactory::get().playlistContextMenu()->clear();
#endif
}

void Platform_PlaylistWidget::showEvent(QShowEvent *se)
{
    batchUpdateSizeHints();
    adjustSize();

    QWidget::showEvent(se);
}

void Platform_PlaylistWidget::removeItem(int idx)
{
    qInfo() << "idx = " << idx;
    auto item_remove = this->_playlist->item(idx);
    if (item_remove) {
        QWidget *pItem = _playlist->itemWidget(item_remove);
        Platform_PlayItemWidget *pCurItem = dynamic_cast<Platform_PlayItemWidget *>(pItem);
        if (pCurItem == pSelectItemWgt) {
            pSelectItemWgt = nullptr;            //如果删除的是原来选中的则置空
        }
        item_remove = this->_playlist->takeItem(idx);
        delete item_remove;
    }

    this->_playlist->update();
    for (int i = 0; i < _playlist->count(); i++) {
        QWidget *item = _playlist->itemWidget(_playlist->item(i));
        if (item) {
            (dynamic_cast<Platform_PlayItemWidget *>(item))->setIndex(i);
        }
    }

    if (_playlist->count() != 0 && _playlist->count() != idx) {
        QWidget *item = _playlist->itemWidget(_playlist->item(idx));
        Platform_PlayItemWidget *curItem = dynamic_cast<Platform_PlayItemWidget *>(item);
        curItem->setCurItemHovered(true);
    } else if (_playlist->count() != 0 && _playlist->count() == idx) {
        QWidget *item = _playlist->itemWidget(_playlist->item(--idx));
        Platform_PlayItemWidget *curItem = dynamic_cast<Platform_PlayItemWidget *>(item);
        curItem->setCurItemHovered(true);
    }

    QString s = QString(tr("%1 videos")).arg(_playlist->count());
    _num->setText(s);
}

void Platform_PlaylistWidget::appendItems()
{
    qInfo() << __func__;

    auto items = _engine->playlist().items();
    auto p = items.begin() + this->_playlist->count();
    while (p != items.end()) {
        auto w = new Platform_PlayItemWidget(*p, this->_playlist, p - items.begin(), this);

        auto item = new QListWidgetItem;
        item->setFlags(Qt::NoItemFlags);
        _playlist->addItem(item);
        _playlist->setItemWidget(item, w);

        connect(w, SIGNAL(closeButtonClicked()), _closeMapper, SLOT(map()));
        connect(w, SIGNAL(doubleClicked()), _activateMapper, SLOT(map()));
        _closeMapper->setMapping(w, w);
        _activateMapper->setMapping(w, w);
        ++p;

    }
    QString s = QString(tr("%1 videos")).arg(_playlist->count());
    _num->setText(s);
    batchUpdateSizeHints();
    updateItemStates();
//    _playlist->setStyleSheet(styleSheet());
//    setStyleSheet(styleSheet());
}

void Platform_PlaylistWidget::slotShowSelectItem(QListWidgetItem *item)
{
    Platform_PlayItemWidget *pWidget = nullptr;

    if (item) {
        _playlist->setCurrentItem(item);
        pWidget = reinterpret_cast<Platform_PlayItemWidget *>(_playlist->itemWidget(item));
        if (!pWidget) {
            return;
        }
    }

    if (CompositingManager::get().isPadSystem()) {
        pWidget->doDoubleClick();
        if (pSelectItemWgt) {
            pSelectItemWgt->setBIsSelect(false);
            pSelectItemWgt = nullptr;
        }
    } else {
        pWidget->setBIsSelect(true);
    }
}

void Platform_PlaylistWidget::OnItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Platform_PlayItemWidget *prevItemWgt = nullptr;
    Platform_PlayItemWidget *curItemWgt = nullptr;
    bool bIsPad = CompositingManager::get().isPadSystem();

    if (previous) {
        prevItemWgt = reinterpret_cast<Platform_PlayItemWidget *>(_playlist->itemWidget(previous));
        if (!bIsPad && prevItemWgt) {
            prevItemWgt->setBIsSelect(false);
        }
    }

    if (current) {
        curItemWgt = reinterpret_cast<Platform_PlayItemWidget *>(_playlist->itemWidget(current));
        if (!bIsPad && curItemWgt) {
            curItemWgt->setBIsSelect(true);
        }
    }
}

void Platform_PlaylistWidget::resetFocusAttribute(bool &atr)
{
    m_bButtonFocusOut = atr;
}

void Platform_PlaylistWidget::loadPlaylist()
{
    qInfo() << __func__;
    _playlist->clear();


    auto items = _engine->playlist().items();
    auto p = items.begin();
    while (p != items.end()) {
        auto w = new Platform_PlayItemWidget(*p, this->_playlist, p - items.begin(), this);
        auto item = new QListWidgetItem;
        item->setFlags(Qt::NoItemFlags);
        _playlist->addItem(item);
        _playlist->setItemWidget(item, w);

        connect(w, SIGNAL(closeButtonClicked()), _closeMapper, SLOT(map()));
        connect(w, SIGNAL(doubleClicked()), _activateMapper, SLOT(map()));
        _closeMapper->setMapping(w, w);
        _activateMapper->setMapping(w, w);
        ++p;
    }

    batchUpdateSizeHints();
    updateItemStates();
//    _playlist->setStyleSheet(styleSheet());
    QString s = QString(tr("%1 videos")).arg(_playlist->count());
    _num->setText(s);
//    setStyleSheet(styleSheet());
}

void Platform_PlaylistWidget::batchUpdateSizeHints()
{
    if (isVisible()) {
        for (int i = 0; i < this->_playlist->count(); i++) {
            auto item = this->_playlist->item(i);
            auto w = this->_playlist->itemWidget(item);
            //auto t = w->size();
            item->setSizeHint(w->size());
        }
    }
}

void Platform_PlaylistWidget::endAnimation()
{
    if (paOpen != nullptr && paClose != nullptr) {
        paOpen -> setDuration(0);
        paClose->setDuration(0);
    }
}

bool Platform_PlaylistWidget::isFocusInPlaylist()
{
    if (m_pClearButton == focusWidget() || _playlist == focusWidget()) {
        return true;
    } else {
        return false;
    }
}

void Platform_PlaylistWidget::togglePopup(bool isShortcut)
{
    if (paOpen != nullptr || paClose != nullptr) {
        return ;
    }

    QRect main_rect = _mw->rect();
#ifdef USE_DXCB
    QRect view_rect = main_rect;
#else
    QRect view_rect = main_rect.marginsRemoved(QMargins(1, 1, 1, 1));
#endif

    QRect fixed(view_rect.width() - width() - 10, 60, 344, view_rect.height() - 138);

    QRect shrunk = fixed;
    shrunk.setWidth(0);
    shrunk.moveRight(fixed.right());

    if (_toggling) return;

    if (_state == State::Opened) {
        Q_ASSERT(isVisible());

        //Set this judgment to false when the playlist is collapsed
        m_bButtonFocusOut = false;
        if (isFocusInPlaylist()) {
            //以除Esc以外的其它方式收起播放列表，焦点切换到主窗口，防止随机出现在其它控件上
            _mw->setFocus();
        }
        _toggling = false;
        _state = State::Closed;
        stateChange(isShortcut);
        setVisible(!isVisible());
    } else {
        _playlist->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        setVisible(!isVisible());
        _toggling = false;  //条件编译误报(cppcheck)
        _state = State::Opened;
        stateChange(isShortcut);
        setGeometry(fixed);
        _playlist->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    }
}

void Platform_PlaylistWidget::paintEvent(QPaintEvent *pe)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QRectF bgRect;
    bgRect.setSize(size());
    QPainterPath pp;
    pp.addRoundedRect(bgRect, 18, 18);
    painter.setBrush(QBrush(QColor(238, 238, 238, 0.8 * 255)));
    painter.setPen(Qt::NoPen);
    painter.drawPath(pp);
}

void Platform_PlaylistWidget::resizeEvent(QResizeEvent *ev)
{
    auto main_rect = _mw->rect();
#ifdef USE_DXCB
    auto view_rect = main_rect;
#else
    auto view_rect = main_rect.marginsRemoved(QMargins(1, 1, 1, 1));
#endif
     _playlist->setFixedHeight(view_rect.height() - 102);
    emit sizeChange();

    QTimer::singleShot(100, this, &Platform_PlaylistWidget::batchUpdateSizeHints);

    QWidget::resizeEvent(ev);
}

bool Platform_PlaylistWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_pClearButton) {
        //焦点在清空按键上禁用上下键
        if (event->type() == QEvent::KeyPress) {
            if (static_cast<QKeyEvent *>(event)->key() == Qt::Key_Up ||
                    static_cast<QKeyEvent *>(event)->key() == Qt::Key_Down) {
                return true;
            }
        }
        switch (event->type()) {
        case QEvent::FocusIn:
            ((Platform_ToolboxProxy *)_mw->toolbox())->setBtnFocusSign(true);
            break;
        case QEvent::FocusOut:
            if (_playlist->count() <= 0) {
                //如果播放列表为空，清空按钮上的焦点不向后传递
                return true;
            }
            break;
        default:
            break;
        }
    } else if (obj == _playlist) {
        switch (event->type()) {
        case QEvent::FocusIn: {
            if (_playlist->count()) {
                //The judgment here is to prevent the focus from shifting during mouse operation
                if (!m_bButtonFocusOut) {
                    return true;
                }
                //焦点切换到播放列表，选中第一个条目
                if (_playlist->currentRow() != 0) {
                    _playlist->setCurrentRow(0);
                    _index = 0;
                    m_bButtonFocusOut = false;
                }
            }
            return true;
        }
        case QEvent::KeyPress: {
            QKeyEvent *keyPressEv = static_cast<QKeyEvent *>(event);
            if (keyPressEv->key() == Qt::Key_Tab) {
                //将焦点设置到清空按钮上，实现焦点循环
                m_pClearButton->setFocus();
                return true;
            }
            break;
        }
        default:
            break;
        }
    }
    return QObject::eventFilter(obj, event); // standard event processing
}
}

#include "platform_playlist_widget.moc"
