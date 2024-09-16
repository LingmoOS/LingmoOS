// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "buttontool.h"
#include "globaldefine.h"
#include "pageview.h"
#include "pagescene.h"
#include "cundoredocommand.h"
#include "layeritem.h"

#include <QStandardPaths>

NCButtonTool::NCButtonTool(int tp, QObject *parent): DrawTool(parent),
    _toolType((tp >= ECustomNoCheckedTool && tp < MoreTool) ? tp : ECustomNoCheckedTool)

{
    auto m_selectBtn = toolButton();
    m_selectBtn->setIconSize(QSize(48, 48));
    m_selectBtn->setFixedSize(QSize(40, 40));
    m_selectBtn->setCheckable(true);
//    connect(m_selectBtn, &QToolButton::toggled, m_selectBtn, [ = ](bool b) {
//        QIcon icon       = QIcon::fromTheme("ddc_choose tools_normal", QIcon(RCC_DRAWBASEPATH + "ddc_choose tools_normal_48px.svg"));
//        QIcon activeIcon = QIcon::fromTheme("ddc_choose tools_active", QIcon(RCC_DRAWBASEPATH + "ddc_choose tools_active_48px.svg"));
//        m_selectBtn->setIcon(b ? activeIcon : icon);
//    });
    m_selectBtn->setIcon(QIcon::fromTheme("ddc_choose tools_normal", QIcon(RCC_DRAWBASEPATH + "ddc_choose tools_normal_48px.svg")));
}

int NCButtonTool::toolType() const
{
    return _toolType;
}

void NCButtonTool::pressOnScene(ToolSceneEvent *event)
{
    Q_UNUSED(event)
}

void NCButtonTool::moveOnScene(ToolSceneEvent *event)
{
    Q_UNUSED(event)
}

void NCButtonTool::releaseOnScene(ToolSceneEvent *event)
{
    Q_UNUSED(event)
}

void NCButtonTool::onStatusChanged(EStatus oldStatus, EStatus nowStatus)
{
    if (toolType() == EClearTool)
        qWarning() << "NCButtonTool::onStatusChanged " << oldStatus << nowStatus;
    if (oldStatus == EIdle && nowStatus == EReady) {
        emit clicked();
        onClicked();
        drawBoard()->setCurrentTool(selection);
    }
}

UndoTool::UndoTool(QObject *parent): NCButtonTool(EUndoTool, parent)
{
    toolButton()->setToolTip(tr("Undo"));
    toolButton()->setIconSize(TOOL_ICON_RECT);
    toolButton()->setIcon(QIcon::fromTheme("revoke_normal"));
}

void UndoTool::onClicked()
{
    auto page = currentPage();
    if (page != nullptr) {
        page->view()->stack()->undo();
    }
}

void UndoTool::onCurrentPageChanged(Page *newPage)
{
    QUndoStack *nowStack = (newPage == nullptr ? nullptr : newPage->view()->stack());

    if (nowStack != lastStack) {
        if (lastStack != nullptr) {
            disconnect(lastStack, &QUndoStack::canUndoChanged, this, &RedoTool::setEnable);
        }

        lastStack = nowStack;

        if (lastStack != nullptr) {
            connect(lastStack, &QUndoStack::canUndoChanged, this, &RedoTool::setEnable);
            setEnable(lastStack->canUndo());
        }
    }
}

RedoTool::RedoTool(QObject *parent): NCButtonTool(ERedoTool, parent)
{

    toolButton()->setToolTip("Redo");
    toolButton()->setIconSize(QSize(20, 20));
    toolButton()->setIcon(QIcon::fromTheme("redo", QIcon(RCC_DRAWBASEPATH + "redo_20px.svg")));

}

void RedoTool::onClicked()
{
    auto page = currentPage();
    if (page != nullptr) {
        page->view()->stack()->redo();
    }
}

void RedoTool::onCurrentPageChanged(Page *newPage)
{
    QUndoStack *nowStack = (newPage == nullptr ? nullptr : newPage->view()->stack());

    if (nowStack != lastStack) {
        if (lastStack != nullptr) {
            disconnect(lastStack, &QUndoStack::canRedoChanged, this, &RedoTool::setEnable);
        }

        lastStack = nowStack;

        if (lastStack != nullptr) {
            connect(lastStack, &QUndoStack::canRedoChanged, this, &RedoTool::setEnable);
            setEnable(lastStack->canRedo());
        }
    }
}

ClearTool::ClearTool(QObject *parent): NCButtonTool(EClearTool, parent)
{

    toolButton()->setToolTip("Clear");
    toolButton()->setIconSize(QSize(20, 20));
    toolButton()->setIcon(QIcon::fromTheme("clear", QIcon(RCC_DRAWBASEPATH + "clear_20px.svg")));

}

void ClearTool::onClicked()
{
    auto page = currentPage();
    if (page != nullptr) {

        UndoRecorder recordUndo(page->scene()->currentTopLayer(), LayerUndoCommand::ChildItemRemoved,
                                page->scene()->currentTopLayer()->items());

        page->scene()->currentTopLayer()->clear();
    }
}

SaveTool::SaveTool(QObject *parent): NCButtonTool(ESaveTool, parent)
{
    toolButton()->setIconSize(QSize(16, 16));
    qobject_cast<QToolButton *>(toolButton())->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolButton()->setIcon(QIcon::fromTheme("save", QIcon(RCC_DRAWBASEPATH + "save_16px.svg")));
    toolButton()->setText(tr("Save"));
    toolButton()->setFixedWidth(72);
    toolButton()->setToolTip(tr("Save"));

}

void SaveTool::onClicked()
{
    QMetaObject::invokeMethod(this, [ = ] {
        auto page = currentPage();
        if (page != nullptr)
        {
            page->save();
        }
    }, Qt::QueuedConnection);
}

CloseTool::CloseTool(QObject *parent): NCButtonTool(ECloseTool, parent)
{
    toolButton()->setIconSize(QSize(30, 30));
    toolButton()->setIcon(QIcon::fromTheme("close", QIcon(RCC_DRAWBASEPATH + "close_30px.svg")));
    toolButton()->setToolTip(tr("Close"));
}

void CloseTool::onClicked()
{
    QMetaObject::invokeMethod(this, [ = ] {
        drawBoard()->close();
    }, Qt::QueuedConnection);
}

OpenTool::OpenTool(QObject *parent): NCButtonTool(EOpenTool, parent)
{

}

void OpenTool::onClicked()
{
    QFileDialog fileDialog(drawBoard());
    fileDialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    fileDialog.setWindowTitle(tr("Import Picture"));
    QStringList filters;
    filters << "*.png *.jpg *.jpeg *.bmp *.tif *.tiff *.ddf";
    fileDialog.setNameFilters(filters);
    fileDialog.setFileMode(QFileDialog::ExistingFiles);

    if (fileDialog.exec() == QDialog::Accepted) {
        QStringList filenames = fileDialog.selectedFiles();

        if (drawBoard() != nullptr) {
            drawBoard()->loadFiles(filenames);
        }
    }
}
