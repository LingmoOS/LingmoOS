// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "ddbase.h"
#include "common.h"

#include <QKeyEvent>

DDBase::DDBase(QWidget *parent)
    : DDialog(parent)
{
    setModal(true);
    setIcon(QIcon::fromTheme(appName));

    m_mainFrame = new DFrame(this);
    m_mainFrame->setFrameRounded(false);
    m_mainFrame->setFrameStyle(DFrame::NoFrame);

    addContent(m_mainFrame);
    // updateGeometry();
}

void DDBase::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key::Key_Escape) {
        event->ignore();
    } else {
        DDialog::keyPressEvent(event);
    }
}
