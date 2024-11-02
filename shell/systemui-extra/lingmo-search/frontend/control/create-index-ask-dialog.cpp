/*
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: zhangjiaping <zhangjiaping@kylinos.cn>
 *
 */

#include "create-index-ask-dialog.h"
#include <QPainterPath>
#include <KWindowSystem>
#include "icon-loader.h"

#define MAIN_SIZE QSize(380, 202)
#define MAIN_SPACING 0
#define MAIN_MARGINS 0,0,0,0
#define TITLE_MARGINS 8,8,8,8
#define TITLE_HEIGHT 40
#define ICON_SIZE QSize(24, 24)
#define TIP_LABEL_HEIGHT 64
#define CLOSE_BTN_SIZE QSize(24, 24)
#define CHECK_BOX_MARGINS 0,0,0,0
#define BTN_FRAME_MARGINS 0,0,0,0
#define CONTENT_MARGINS 32,0,32,24

using namespace LingmoUISearch;
CreateIndexAskDialog::CreateIndexAskDialog(QWidget *parent) : QDialog(parent) {
//    this->setWindowIcon(QIcon::fromTheme("lingmo-search"));
    this->setWindowTitle(tr("lingmo-search"));

    initUi();

    this->installEventFilter(this);
}

void CreateIndexAskDialog::initUi() {
    KWindowSystem::setState(this->winId(),NET::SkipTaskbar | NET::SkipPager);
    this->setFixedSize(MAIN_SIZE);
    m_mainLyt = new QVBoxLayout(this);
    this->setLayout(m_mainLyt);
    m_mainLyt->setContentsMargins(MAIN_MARGINS);
    m_mainLyt->setSpacing(MAIN_SPACING);

    //标题栏
    m_titleFrame = new QFrame(this);
    m_titleFrame->setFixedHeight(TITLE_HEIGHT);
    m_titleLyt = new QHBoxLayout(m_titleFrame);
    m_titleLyt->setContentsMargins(TITLE_MARGINS);
    m_titleFrame->setLayout(m_titleLyt);
    m_iconLabel = new QLabel(m_titleFrame);
    m_iconLabel->setFixedSize(ICON_SIZE);
    m_iconLabel->setPixmap(IconLoader::loadIconQt("lingmo-search").pixmap(QSize(24, 24)));
    //主题改变时，更新自定义标题栏的图标
    connect(qApp, &QApplication::paletteChanged, this, [ = ]() {
        m_iconLabel->setPixmap(IconLoader::loadIconQt("lingmo-search").pixmap(QSize(24, 24)));
    });
    m_titleLabel = new QLabel(m_titleFrame);
    m_titleLabel->setText(tr("Search"));
    m_closeBtn = new QPushButton(m_titleFrame);
    m_closeBtn->setFixedSize(CLOSE_BTN_SIZE);
    m_closeBtn->setIcon(IconLoader::loadIconQt("window-close-symbolic"));
    m_closeBtn->setProperty("isWindowButton", 0x02);
    m_closeBtn->setProperty("useIconHighlightEffect", 0x08);
    m_closeBtn->setFlat(true);
    m_closeBtn->setToolTip(tr("close"));
    connect(m_closeBtn, &QPushButton::clicked, this, [ = ]() {
        this->hide();
    });
    m_titleLyt->addWidget(m_iconLabel);
    m_titleLyt->addWidget(m_titleLabel);
    m_titleLyt->addStretch();
    m_titleLyt->addWidget(m_closeBtn);

    m_mainLyt->addWidget(m_titleFrame);

    //内容区域
    m_contentFrame = new QFrame(this);
    m_contentLyt = new QVBoxLayout(m_contentFrame);
    m_contentFrame->setLayout(m_contentLyt);
    m_contentLyt->setContentsMargins(CONTENT_MARGINS);

    m_tipLabel = new QLabel(m_contentFrame);
    m_tipLabel->setText(tr("Creating index can help you get results more quickly. Would you like to create one?"));
    m_tipLabel->setWordWrap(true);
    m_tipLabel->setAlignment(Qt::AlignVCenter);
    m_tipLabel->setMinimumHeight(TIP_LABEL_HEIGHT);
    m_contentLyt->addWidget(m_tipLabel);

    m_checkFrame = new QFrame(m_contentFrame);
    m_checkLyt = new QHBoxLayout(m_checkFrame);
    m_checkLyt->setContentsMargins(CHECK_BOX_MARGINS);
    m_checkFrame->setLayout(m_checkLyt);
    m_checkBox = new QCheckBox(m_checkFrame);
    m_checkBox->setText(tr("Don't remind me again"));
    m_checkLyt->addWidget(m_checkBox);
    m_checkLyt->addStretch();
    m_contentLyt->addWidget(m_checkFrame);
    m_contentLyt->addStretch();

    m_btnFrame = new QFrame(m_contentFrame);
    m_btnLyt = new QHBoxLayout(m_btnFrame);
    m_btnFrame->setLayout(m_btnLyt);
    m_btnLyt->setContentsMargins(BTN_FRAME_MARGINS);
    m_cancelBtn = new QPushButton(m_btnFrame);
    m_cancelBtn->setText(tr("No"));
    m_confirmBtn = new QPushButton(m_btnFrame);
    m_confirmBtn->setText(tr("Yes"));
    connect(m_cancelBtn, &QPushButton::clicked, this, [ = ]() {
        Q_EMIT this->btnClicked(false, m_checkBox->isChecked());
        this->hide();
    });
    connect(m_confirmBtn, &QPushButton::clicked, this, [ = ]() {
        Q_EMIT this->btnClicked(true, m_checkBox->isChecked());
        this->hide();
    });
    m_btnLyt->addStretch();
    m_btnLyt->addWidget(m_cancelBtn);
    m_btnLyt->addWidget(m_confirmBtn);
    m_contentLyt->addWidget(m_btnFrame);

    m_mainLyt->addWidget(m_contentFrame);
#if (QT_VERSION < QT_VERSION_CHECK(5, 12, 0))
    m_titleFrame->hide();
    this->setFixedSize(380, 162);
#endif
}

/**
 * @brief CreateIndexAskDialog::paintEvent 绘制窗口背景（默认背景较暗）
 */
void CreateIndexAskDialog::paintEvent(QPaintEvent *event) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;
    rectPath.addRect(this->rect());
    p.save();
    p.fillPath(rectPath, palette().color(QPalette::Base));
    p.restore();
    return QDialog::paintEvent(event);
}

// esc按键直接调用hide，产生hideEvent
void CreateIndexAskDialog::hideEvent(QHideEvent *event)
{
    Q_EMIT this->closed();
    QWidget::hideEvent(event);
}

bool CreateIndexAskDialog::eventFilter(QObject *watched, QEvent *event)
{
    // kwin alt+f4发送的close事件会改变窗口的winid,屏蔽掉该事件，并发送hide事件
    if ((watched == this) && (event->type() == QEvent::Close)) {
        event->ignore();
        this->hide();
        return true;
    }
    if (event->type() == QEvent::ActivationChange) {
        if (QApplication::activeWindow() != this) {
            Q_EMIT this->focusChanged();
        }
    }
    return QDialog::eventFilter(watched, event);
}
