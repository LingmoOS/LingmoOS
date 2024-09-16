// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "SheetSidebar.h"
#include "DocSheet.h"
#include "ThumbnailWidget.h"
#include "CatalogWidget.h"
#include "BookMarkWidget.h"
#include "NotesWidget.h"
#include "SearchResWidget.h"
#include "Model.h"
#include "MsgHeader.h"
#include "Utils.h"

#include <DPushButton>
#include <DGuiApplicationHelper>

#include <QButtonGroup>
#include <QVBoxLayout>
#include <QTimer>

const int LEFTMINWIDTH = 266;
const int LEFTMAXWIDTH = 380;
SheetSidebar::SheetSidebar(DocSheet *parent, PreviewWidgesFlags widgesFlag)
    : BaseWidget(parent)
    , m_sheet(parent)
    , m_widgetsFlag(widgesFlag | PREVIEW_SEARCH)
{
    initWidget();
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &SheetSidebar::onUpdateWidgetTheme);
}

SheetSidebar::~SheetSidebar()
{

}

void SheetSidebar::initWidget()
{
    m_scale           = 1.0;
    m_bOldVisible     = false;
    m_bOpenDocOpenSuccess = false;
    m_thumbnailWidget = nullptr;
    m_catalogWidget   = nullptr;
    m_bookmarkWidget  = nullptr;
    m_notesWidget     = nullptr;
    m_searchWidget    = nullptr;

    setMinimumWidth(LEFTMINWIDTH);
    setMaximumWidth(LEFTMAXWIDTH);

    resize(LEFTMINWIDTH, this->height());

    QVBoxLayout *pVBoxLayout = new QVBoxLayout;
    pVBoxLayout->setContentsMargins(0, 0, 0, 0);
    pVBoxLayout->setSpacing(0);
    this->setLayout(pVBoxLayout);

    m_stackLayout = new QStackedLayout;
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(15, 5, 15, 5);

    QList<QWidget *> tabWidgetlst;

    if (m_widgetsFlag.testFlag(PREVIEW_THUMBNAIL)) {
        m_thumbnailWidget = new ThumbnailWidget(m_sheet, this);
        int index = m_stackLayout->addWidget(m_thumbnailWidget);
        DToolButton *btn = createBtn(tr("Thumbnails"), "thumbnail");
        m_btnGroupMap.insert(index, btn);
        hLayout->addWidget(btn);
        hLayout->addStretch();

        connect(btn, &QToolButton::clicked, [this, index]() {
            onBtnClicked(index);
        });

        m_thumbnailWidget->setTabOrderWidget(tabWidgetlst);
    }

    if (m_widgetsFlag.testFlag(PREVIEW_CATALOG)) {
        m_catalogWidget = new CatalogWidget(m_sheet, this);
        int index = m_stackLayout->addWidget(m_catalogWidget);
        DToolButton *btn = createBtn(tr("Catalog"), "catalog");
        m_btnGroupMap.insert(index, btn);
        hLayout->addWidget(btn);
        hLayout->addStretch();

        connect(btn, &QToolButton::clicked, [this, index]() {
            onBtnClicked(index);
        });
    }

    if (m_widgetsFlag.testFlag(PREVIEW_BOOKMARK)) {
        m_bookmarkWidget = new BookMarkWidget(m_sheet, this);
        int index = m_stackLayout->addWidget(m_bookmarkWidget);
        DToolButton *btn = createBtn(tr("Bookmarks"), "bookmark");
        m_btnGroupMap.insert(index, btn);
        hLayout->addWidget(btn);
        hLayout->addStretch();

        connect(btn, &QToolButton::clicked, [this, index]() {
            onBtnClicked(index);
        });

        m_bookmarkWidget->setTabOrderWidget(tabWidgetlst);
    }

    if (m_widgetsFlag.testFlag(PREVIEW_NOTE)) {
        m_notesWidget = new NotesWidget(m_sheet, this);
        int index = m_stackLayout->addWidget(m_notesWidget);
        DToolButton *btn = createBtn(tr("Annotations"), "annotation");
        m_btnGroupMap.insert(index, btn);
        hLayout->addWidget(btn);
        hLayout->addStretch();

        connect(btn, &QToolButton::clicked, [this, index]() {
            onBtnClicked(index);
        });

        m_notesWidget->setTabOrderWidget(tabWidgetlst);
    }

    //remove last spaceitem
    QLayoutItem *item = hLayout->takeAt(hLayout->count() - 1);
    if (item) delete item;

    if (m_widgetsFlag.testFlag(PREVIEW_SEARCH)) {
        m_searchWidget = new SearchResWidget(m_sheet, this);
        m_searchId = m_stackLayout->addWidget(m_searchWidget);
        DToolButton *btn = createBtn("Search", "search");
        btn->setVisible(false);
        m_btnGroupMap.insert(m_searchId, btn);
        tabWidgetlst << btn;
    }

    pVBoxLayout->addLayout(m_stackLayout);
    pVBoxLayout->addLayout(hLayout);
    onUpdateWidgetTheme();
    this->setVisible(false);

    int nId = qBound(0, m_sheet->operation().sidebarIndex, m_stackLayout->count() - 1);
    onBtnClicked(nId);

    Utils::setObjectNoFocusPolicy(this);
    for (int i = 0; i < tabWidgetlst.size() - 1; i++) {
        QWidget::setTabOrder(tabWidgetlst.at(i), tabWidgetlst.at(i + 1));
    }
#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::NormalMode) {
        setMinimumWidth(LEFTMINWIDTH);
        setMaximumWidth(LEFTMAXWIDTH);
        for(DToolButton *btn: m_btnGroupMap.values()) {
            if(btn) {
                btn->setFixedSize(QSize(36, 36));
            }
        }
    } else {
        setMinimumWidth(LEFTMINWIDTH-30);
        setMaximumWidth(LEFTMAXWIDTH);
        for(DToolButton *btn: m_btnGroupMap.values()) {
            if(btn) {
                btn->setFixedSize(QSize(24, 24));
            }
        }
    }
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [=](DGuiApplicationHelper::SizeMode sizeMode) {
        if (sizeMode == DGuiApplicationHelper::NormalMode) {
            setMinimumWidth(LEFTMINWIDTH);
            setMaximumWidth(LEFTMAXWIDTH);
            for(DToolButton *btn: m_btnGroupMap.values()) {
                if(btn) {
                    btn->setFixedSize(QSize(36, 36));
                }
            }
        } else {
            setMinimumWidth(LEFTMINWIDTH-30);
            setMaximumWidth(LEFTMAXWIDTH);
            for(DToolButton *btn: m_btnGroupMap.values()) {
                if(btn) {
                    btn->setFixedSize(QSize(24, 24));
                }
            }
        }
    });
#endif
}

void SheetSidebar::onBtnClicked(int index)
{
    if (!m_btnGroupMap.contains(index))
        return;

    for (auto iter = m_btnGroupMap.begin(); iter != m_btnGroupMap.end(); iter++) {
        iter.value()->setChecked(false);
    }

    m_btnGroupMap.value(index)->setChecked(true);
    m_stackLayout->setCurrentIndex(index);
    adaptWindowSize(m_scale);
    if (m_stackLayout->currentWidget() != m_searchWidget) {
        m_sheet->m_operation.sidebarIndex = index;
    }
    onHandleOpenSuccessDelay();
}

void SheetSidebar::setBookMark(int index, int state)
{
    if (m_bookmarkWidget) m_bookmarkWidget->handleBookMark(index, state);
    if (m_thumbnailWidget) m_thumbnailWidget->setBookMarkState(index, state);
}

void SheetSidebar::setCurrentPage(int page)
{
    if (m_thumbnailWidget) m_thumbnailWidget->handlePage(page - 1);
    if (m_bookmarkWidget) m_bookmarkWidget->handlePage(page - 1);
    if (m_catalogWidget) m_catalogWidget->handlePage(page - 1);
}

void SheetSidebar::handleOpenSuccess()
{
    m_bOpenDocOpenSuccess = true;
    this->setVisible(m_sheet->operation().sidebarVisible);
    int nId = qBound(0, m_sheet->operation().sidebarIndex, m_stackLayout->count() - 2);
    onBtnClicked(nId);
}

void SheetSidebar::onHandleOpenSuccessDelay()
{
    if (m_bOpenDocOpenSuccess) {
        QTimer::singleShot(100, this, SLOT(onHandWidgetDocOpenSuccess()));
    }
}

void SheetSidebar::onHandWidgetDocOpenSuccess()
{
    if (!this->isVisible())
        return;

    QWidget *curWidget = m_stackLayout->currentWidget();
    if (curWidget == m_thumbnailWidget) {
        m_thumbnailWidget->handleOpenSuccess();
    } else if (curWidget == m_catalogWidget) {
        m_catalogWidget->handleOpenSuccess();
    } else if (curWidget == m_bookmarkWidget) {
        m_bookmarkWidget->handleOpenSuccess();
    } else if (curWidget == m_notesWidget) {
        m_notesWidget->handleOpenSuccess();
    }
}

void SheetSidebar::handleSearchStart(const QString &text)
{
    for (auto iter = m_btnGroupMap.begin(); iter != m_btnGroupMap.end(); iter++) {
        iter.value()->setEnabled(false);
    }

    m_searchWidget->clearFindResult();
    m_searchWidget->searchKey(text);
    onBtnClicked(m_searchId);
    this->setVisible(true);
}

void SheetSidebar::handleSearchStop()
{
    for (auto iter = m_btnGroupMap.begin(); iter != m_btnGroupMap.end(); iter++) {
        iter.value()->setEnabled(true);
    }

    int nId = qBound(0, m_sheet->operation().sidebarIndex, m_stackLayout->count() - 1);
    onBtnClicked(nId);
    this->setVisible(m_sheet->operation().sidebarVisible);
    m_searchWidget->clearFindResult();
}

void SheetSidebar::handleSearchResultComming(const deepin_reader::SearchResult &res)
{
    m_searchWidget->handleSearchResultComming(res);
}

int SheetSidebar::handleFindFinished()
{
    return m_searchWidget->handleFindFinished();
}

void SheetSidebar::handleRotate()
{
    if (m_thumbnailWidget)
        m_thumbnailWidget->handleRotate();
}

void SheetSidebar::handleAnntationMsg(const int &msg, int index, deepin_reader::Annotation *anno)
{
    if (m_notesWidget)
        m_notesWidget->handleAnntationMsg(msg, anno);

    Q_UNUSED(index);
}

DToolButton *SheetSidebar::createBtn(const QString &btnName, const QString &objName)
{
    int tW = 36;
    auto btn = new DToolButton(this);
    btn->setToolTip(btnName);
    btn->setObjectName(objName);
    btn->setAccessibleName("Button_" + objName);
    btn->setFixedSize(QSize(tW, tW));
    btn->setIconSize(QSize(tW, tW));
    btn->setCheckable(true);
    if ("search" != objName) {
        btn->setFocusPolicy(Qt::TabFocus);
    }
    return btn;
}

void SheetSidebar::resizeEvent(QResizeEvent *event)
{
    qreal scale = event->size().width() * 1.0 / LEFTMINWIDTH;
    adaptWindowSize(scale);
    BaseWidget::resizeEvent(event);
}

void SheetSidebar::adaptWindowSize(const double &scale)
{
    m_scale = scale;
    BaseWidget *curWidget = dynamic_cast<BaseWidget *>(m_stackLayout->currentWidget());
    if (curWidget) curWidget->adaptWindowSize(scale);
}

void SheetSidebar::keyPressEvent(QKeyEvent *event)
{
    QStringList pFilterList = QStringList() << Dr::key_pgUp << Dr::key_pgDown
                              << Dr::key_down << Dr::key_up
                              << Dr::key_left << Dr::key_right
                              << Dr::key_delete;

    QString key = Utils::getKeyshortcut(event);

    if (pFilterList.contains(key)) {
        dealWithPressKey(key);
    }

    BaseWidget::keyPressEvent(event);
}

void SheetSidebar::showEvent(QShowEvent *event)
{
    BaseWidget::showEvent(event);
    onHandleOpenSuccessDelay();
}

void SheetSidebar::showMenu()
{
    DToolButton *bookmarkbtn = this->findChild<DToolButton *>("bookmark");
    if (m_bookmarkWidget && bookmarkbtn && bookmarkbtn->isChecked()) {
        m_bookmarkWidget->showMenu();
    }
    DToolButton *annotationbtn = this->findChild<DToolButton *>("annotation");
    if (m_notesWidget && annotationbtn && annotationbtn->isChecked()) {
        m_notesWidget->showMenu();
    }
}

void SheetSidebar::dealWithPressKey(const QString &sKey)
{
    if (sKey == Dr::key_up || sKey == Dr::key_left) {
        onJumpToPrevPage();
    } else if (sKey == Dr::key_pgUp) {
        onJumpToPageUp();
    } else if (sKey == Dr::key_down || sKey == Dr::key_right) {
        onJumpToNextPage();
    } else if (sKey == Dr::key_pgDown) {
        onJumpToPageDown();
    } else if (sKey == Dr::key_delete) {
        deleteItemByKey();
    }
}

void SheetSidebar::onJumpToPrevPage()
{
    QWidget *curWidget = m_stackLayout->currentWidget();
    if (curWidget == m_thumbnailWidget) {
        m_thumbnailWidget->prevPage();
    }  else if (curWidget == m_bookmarkWidget) {
        m_bookmarkWidget->prevPage();
    } else if (curWidget == m_notesWidget) {
        m_notesWidget->prevPage();
    } else if (curWidget == m_catalogWidget) {
        m_catalogWidget->prevPage();
    }
}

void SheetSidebar::onJumpToPageUp()
{
    QWidget *curWidget = m_stackLayout->currentWidget();
    if (curWidget == m_thumbnailWidget) {
        m_thumbnailWidget->pageUp();
    } else if (curWidget == m_bookmarkWidget) {
        m_bookmarkWidget->pageUp();
    } else if (curWidget == m_notesWidget) {
        m_notesWidget->pageUp();
    } else if (curWidget == m_catalogWidget) {
        m_catalogWidget->pageUp();
    }
}

void SheetSidebar::onJumpToNextPage()
{
    QWidget *curWidget = m_stackLayout->currentWidget();
    if (curWidget == m_thumbnailWidget) {
        m_thumbnailWidget->nextPage();
    }  else if (curWidget == m_bookmarkWidget) {
        m_bookmarkWidget->nextPage();
    } else if (curWidget == m_notesWidget) {
        m_notesWidget->nextPage();
    } else if (curWidget == m_catalogWidget) {
        m_catalogWidget->nextPage();
    }
}

void SheetSidebar::onJumpToPageDown()
{
    QWidget *curWidget = m_stackLayout->currentWidget();
    if (curWidget == m_thumbnailWidget) {
        m_thumbnailWidget->pageDown();
    } else if (curWidget == m_bookmarkWidget) {
        m_bookmarkWidget->pageDown();
    } else if (curWidget == m_notesWidget) {
        m_notesWidget->pageDown();
    } else if (curWidget == m_catalogWidget) {
        m_catalogWidget->pageDown();
    }
}

void SheetSidebar::deleteItemByKey()
{
    QWidget *widget = m_stackLayout->currentWidget();
    if (widget == m_bookmarkWidget) {
        m_bookmarkWidget->deleteItemByKey();
    } else if (widget == m_notesWidget) {
        m_notesWidget->deleteItemByKey();
    }
}

bool SheetSidebar::event(QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
        if (key_event->key() == Qt::Key_Menu && !key_event->isAutoRepeat()) {
            showMenu();
        }
        if (key_event->key() == Qt::Key_M && (key_event->modifiers() & Qt::AltModifier) && !key_event->isAutoRepeat()) {
            showMenu();
        }
    }

    return BaseWidget::event(event);
}

void SheetSidebar::onUpdateWidgetTheme()
{
    updateWidgetTheme();
    for (auto iter = m_btnGroupMap.begin(); iter != m_btnGroupMap.end(); iter++) {
        const QString &objName = iter.value()->objectName();
        const QIcon &icon = QIcon::fromTheme(QString("dr_") + objName);
        iter.value()->setIcon(icon);
    }
}

void SheetSidebar::changeResetModelData()
{
    if (m_notesWidget) {
        m_notesWidget->changeResetModelData();
    }
}
