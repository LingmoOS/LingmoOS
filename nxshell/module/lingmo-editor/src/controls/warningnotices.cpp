// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "warningnotices.h"

#include <DGuiApplicationHelper>
#include <DDialogCloseButton>

#ifdef DTKWIDGET_CLASS_DSizeMode
#include <DSizeMode>
#endif

const int s_WNCloseBtnSize = 32;
const int s_WNCloseBtnSizeCompact = 26;

WarningNotices::WarningNotices(MessageType notifyType, QWidget *parent)
    : DFloatingMessage(notifyType, parent)
{
    this->setFont(qApp->font());

    setIcon(QIcon(":/images/warning.svg"));
    m_reloadBtn = new QPushButton(tr("Reload"), this);
    m_saveAsBtn = new QPushButton(qApp->translate("Window", "Save as"), this);
    m_reloadBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_saveAsBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    connect(m_reloadBtn, &QPushButton::clicked, this, &WarningNotices::slotreloadBtnClicked);
    connect(m_saveAsBtn, &QPushButton::clicked, this, &WarningNotices::slotsaveAsBtnClicked);

#ifdef DTKWIDGET_CLASS_DSizeMode
    DDialogCloseButton *closeBtn = findChild<DDialogCloseButton *>();
    if (closeBtn) {
        closeBtn->setIconSize(DSizeModeHelper::element(QSize(26, 26), QSize(32, 32)));
        connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [=]() {
            closeBtn->setIconSize(DSizeModeHelper::element(QSize(26, 26), QSize(32, 32)));
        });
    }

    // 紧凑模式下字体切换同样处理
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::fontChanged, this, [this](const QFont &font) {
        this->setFont(font);
    });
#endif
}

WarningNotices::~WarningNotices() {}

void WarningNotices::setReloadBtn()
{
    if (!m_reloadBtn->isVisible()) {
        m_reloadBtn->setVisible(true);
    }
    m_reloadBtn->setVisible(true);
    m_saveAsBtn->setVisible(false);
    setWidget(m_reloadBtn);
}

void WarningNotices::setSaveAsBtn()
{
    if (!m_saveAsBtn->isVisible()) {
        m_saveAsBtn->setVisible(true);
    }
    m_saveAsBtn->setVisible(true);
    m_reloadBtn->setVisible(false);
    setWidget(m_saveAsBtn);
}

void WarningNotices::clearBtn()
{
    m_saveAsBtn->setVisible(false);
    m_reloadBtn->setVisible(false);
}

void WarningNotices::slotreloadBtnClicked()
{
    this->hide();
    emit reloadBtnClicked();
}

void WarningNotices::slotsaveAsBtnClicked()
{
    this->hide();
    emit saveAsBtnClicked();
}
