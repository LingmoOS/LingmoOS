// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "BrowserMenu.h"
#include "ColorWidgetAction.h"
#include "DocSheet.h"
#include "Utils.h"
#include "Global.h"

#include <DFontSizeManager>

BrowserMenu::BrowserMenu(QWidget *parent) : DMenu(parent)
{
    DFontSizeManager::instance()->bind(this, DFontSizeManager::T6);
}

void BrowserMenu::initActions(DocSheet *sheet, int index, SheetMenuType_e type, const QString &copytext)
{
    m_type = type;
    m_pColorWidgetAction = nullptr;
    if (type == DOC_MENU_ANNO_ICON) {
        createAction(tr("Copy"), "CopyAnnoText");
        this->addSeparator();

        if (sheet->fileType() == Dr::FileType::PDF || sheet->fileType() == Dr::FileType::DOCX) {
            createAction(tr("Remove annotation"), "RemoveAnnotation");
            createAction(tr("Add annotation"), "AddAnnotationIcon");
        }
        if (sheet->hasBookMark(index))
            createAction(tr("Remove bookmark"), "RemoveBookmark");
        else
            createAction(tr("Add bookmark"), "AddBookmark");
    } else if (type == DOC_MENU_ANNO_HIGHLIGHT || type == DOC_MENU_SELECT_TEXT) {
        if (type == DOC_MENU_ANNO_HIGHLIGHT) {
            QAction *copyAnnoAction = createAction(tr("Copy"), "CopyAnnoText");
            if (copytext.isEmpty())
                copyAnnoAction->setDisabled(true);
        } else {
            createAction(tr("Copy"), "Copy");
        }
        this->addSeparator();

        QAction *highAct = createAction(tr("Highlight"), type != DOC_MENU_SELECT_TEXT ? "ChangeAnnotationColor" : "AddTextHighlight");
        if (type == DOC_MENU_ANNO_HIGHLIGHT)
            highAct->setDisabled(true);

        m_pColorWidgetAction = new ColorWidgetAction(this);
        connect(m_pColorWidgetAction, SIGNAL(sigBtnGroupClicked()), this, SLOT(onSetHighLight()));
        this->addAction(m_pColorWidgetAction);

        if (sheet->fileType() == Dr::FileType::PDF || sheet->fileType() == Dr::FileType::DOCX) {
            QAction *rmHighAct = createAction(tr("Remove highlight"), "RemoveHighlight");
            rmHighAct->setDisabled(true);
            if (type == DOC_MENU_ANNO_HIGHLIGHT) rmHighAct->setDisabled(false);
            this->addSeparator();
            createAction(tr("Add annotation"), "AddAnnotationHighlight");
        }

        if (sheet->hasBookMark(index))
            createAction(tr("Remove bookmark"), "RemoveBookmark");
        else
            createAction(tr("Add bookmark"), "AddBookmark");
    } else if (type == DOC_MENU_KEY) {
        createAction(tr("Search"), "Search");
        this->addSeparator();

        if (sheet->hasBookMark(index))
            createAction(tr("Remove bookmark"), "RemoveBookmark");
        else
            createAction(tr("Add bookmark"), "AddBookmark");

        this->addSeparator();

        if (sheet->isFullScreen()) {
            createAction(tr("Exit fullscreen"), "ExitFullscreen");
        } else {
            createAction(tr("Fullscreen"), "Fullscreen");
        }

        createAction(tr("Slide show"), "SlideShow");
        this->addSeparator();

        QAction *pFirstPage = createAction(tr("First page"), "FirstPage");
        QAction *pPrevPage = createAction(tr("Previous page"), "PreviousPage");
        if (sheet->currentIndex() == 0) {
            pFirstPage->setDisabled(true);
            pPrevPage->setDisabled(true);
        }

        QAction *pNextPage = createAction(tr("Next page"), "NextPage");
        QAction *pEndPage = createAction(tr("Last page"), "LastPage");
        if (sheet->currentIndex() == sheet->pageCount() - 1) {
            pNextPage->setDisabled(true);
            pEndPage->setDisabled(true);
        }

        this->addSeparator();

        createAction(tr("Rotate left"), "RotateLeft");
        createAction(tr("Rotate right"), "RotateRight");
        this->addSeparator();

        createAction(tr("Print"), "Print");
        createAction(tr("Document info"), "DocumentInfo");
    } else {
        if (sheet->fileType() == Dr::FileType::PDF || sheet->fileType() == Dr::FileType::DOCX) {
            createAction(tr("Search"), "Search");
            this->addSeparator();
        }

        if (sheet->hasBookMark(index))
            createAction(tr("Remove bookmark"), "RemoveBookmark");
        else
            createAction(tr("Add bookmark"), "AddBookmark");

        if (sheet->fileType() == Dr::FileType::PDF || sheet->fileType() == Dr::FileType::DOCX) {
            createAction(tr("Add annotation"), "AddAnnotationIcon");
        }

        this->addSeparator();

        if (sheet->isFullScreen()) {
            createAction(tr("Exit fullscreen"), "ExitFullscreen");
        } else {
            createAction(tr("Fullscreen"), "Fullscreen");
        }

        createAction(tr("Slide show"), "SlideShow");
        this->addSeparator();

        QAction *pFirstPage = createAction(tr("First page"), "FirstPage");
        QAction *pPrevPage = createAction(tr("Previous page"), "PreviousPage");
        if (sheet->currentIndex() == 0) {
            pFirstPage->setDisabled(true);
            pPrevPage->setDisabled(true);
        }

        QAction *pNextPage = createAction(tr("Next page"), "NextPage");
        QAction *pEndPage = createAction(tr("Last page"), "LastPage");
        if (sheet->currentIndex() == sheet->pageCount() - 1) {
            pNextPage->setDisabled(true);
            pEndPage->setDisabled(true);
        }

        this->addSeparator();

        createAction(tr("Rotate left"), "RotateLeft");
        createAction(tr("Rotate right"), "RotateRight");
        this->addSeparator();

        createAction(tr("Print"), "Print");
        createAction(tr("Document info"), "DocumentInfo");
    }
}

void BrowserMenu::hideEvent(QHideEvent *event)
{
    emit sigMenuHide();

    DMenu::hideEvent(event);
}

QAction *BrowserMenu::createAction(const QString &displayname, const QString &objectname)
{
    QAction *action = new  QAction(displayname, this);
    action->setObjectName(objectname);
    connect(action, SIGNAL(triggered()), this, SLOT(onItemClicked()));
    this->addAction(action);
    return action;
}

void BrowserMenu::onItemClicked()
{
    emit signalMenuItemClicked(sender()->objectName());
}

void BrowserMenu::onSetHighLight()
{
    if (m_type == DOC_MENU_SELECT_TEXT) {
        emit signalMenuItemClicked("AddTextHighlight");
    } else if (m_type == DOC_MENU_ANNO_HIGHLIGHT) {
        emit signalMenuItemClicked("ChangeAnnotationColor");
    }
    this->close();
}
