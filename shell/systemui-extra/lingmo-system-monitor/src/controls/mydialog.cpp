/*
 * Copyright (C) 2020 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Kobe Lee    xiangli@ubuntulingmo.com/kobe24_lixiang@126.com
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mydialog.h"
#include "../xatom-helper.h"
#include "../macro.h"
#include "../util.h"

#include <QLabel>
#include <QDebug>
#include <QCloseEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QAction>
#include <QPushButton>
#include <QAbstractButton>
#include <QPainter>
#include <QPainterPath>

MyDialog::MyDialog(const QString &title, const QString &message, QWidget *parent, SIZE_MODEL sizeModel) :
    QDialog(parent)
    , m_titleWidth(0)
    , mousePressed(false)
{
    QString platform = QGuiApplication::platformName();
    if (!platform.startsWith(QLatin1String("wayland"),Qt::CaseInsensitive)) {
        MotifWmHints hints;
        hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
        hints.functions = MWM_FUNC_ALL;
        hints.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);
    }
    //this->setWindowFlags(this->windowFlags() | Qt::Tool | Qt::WindowCloseButtonHint);
    m_sizeModel = sizeModel;

    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setAttribute(Qt::WA_Resized, false);
    switch (m_sizeModel)
    {
    case LARGE:
    case MIDDLE:
        this->setFixedSize(660, 240);
        break;
    default:
        this->setFixedSize(320, 160);
        break;
    }

    const QByteArray id(THEME_QT_SCHEMA);
    if(QGSettings::isSchemaInstalled(id))
    {
        styleSettings = new QGSettings(id);
    }

    initThemeStyle();

    m_titleIcon = new QLabel;
    m_titleIcon->setPixmap(QIcon::fromTheme("lingmo-system-monitor").pixmap(24,24));
    m_titleIcon->setFixedWidth(24);

    QHBoxLayout* titleLayout = new QHBoxLayout();
    titleLayout->setContentsMargins(8, 4, 4, 0);
    titleLayout->setSpacing(8);

    m_topLayout = new QHBoxLayout;
    m_topLayout->setContentsMargins(24, 0, 24, 0);

    m_titleLabel = new QLabel;
    m_titleLabel->hide();
    titleLayout->addWidget(m_titleIcon, 0, Qt::AlignLeft|Qt::AlignTop);
    titleLayout->addWidget(m_titleLabel, 0, Qt::AlignLeft|Qt::AlignTop);

    m_messageLabel = new QLabel;
    m_messageLabel->hide();
    m_messageLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_messageLabel->setWordWrap(true);//QLabel自动换行

    QHBoxLayout *textLayout = new QHBoxLayout;
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(8);
    if (m_sizeModel == SMALL) {
        QLabel *msgIcon = new QLabel();
        msgIcon->setPixmap(QIcon::fromTheme(QLatin1String("messagebox_warning")).pixmap(32));
        textLayout->addWidget(msgIcon, 0, Qt::AlignLeft|Qt::AlignTop);
    }
    textLayout->addWidget(m_messageLabel, 0, Qt::AlignLeft);
    textLayout->addStretch();

    m_topLayout->addLayout(textLayout);

    closeButton = new QPushButton();
    closeButton->setObjectName("CloseButton");
    closeButton->setFlat(true);
    connect(closeButton,&QPushButton::clicked,this,[=](){
       this->deleteLater();
       this->close();
    });
    closeButton->setIcon(QIcon::fromTheme("window-close-symbolic"));
    closeButton->setAttribute(Qt::WA_NoMousePropagation);
    closeButton->setProperty("isWindowButton", 0x2);
    closeButton->setProperty("useIconHighlightEffect", 0x8);
    closeButton->setFixedSize(30, 30);
    titleLayout->addWidget(closeButton, 0, Qt::AlignTop | Qt::AlignRight);

    m_contentLayout = new QVBoxLayout();
    m_contentLayout->setContentsMargins(24,0,24,0);
    m_contentLayout->setSpacing(0);

    m_buttonLayout = new QHBoxLayout;
    m_buttonLayout->setMargin(0);
    m_buttonLayout->setSpacing(0);
    if (m_sizeModel == SMALL) {
        m_buttonLayout->setSpacing(24);
        m_buttonLayout->addStretch(1);
        m_buttonLayout->setContentsMargins(16, 14, 16, 16);
    } else {
        m_buttonLayout->setContentsMargins(24, 14, 24, 24);
    }

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(4);

    mainLayout->addLayout(titleLayout);
    mainLayout->addStretch(1);
    mainLayout->addLayout(m_topLayout);
    mainLayout->addStretch(1);
    mainLayout->addLayout(m_contentLayout);
    mainLayout->addStretch(1);
    mainLayout->addLayout(m_buttonLayout);

    QAction *button_action = new QAction(this);
    button_action->setShortcuts(QKeySequence::InsertParagraphSeparator);
    button_action->setAutoRepeat(false);
    connect(button_action, SIGNAL(triggered(bool)), this, SLOT(onDefaultButtonTriggered()));

    this->setLayout(mainLayout);
    this->addAction(button_action);
    this->setFocusPolicy(Qt::ClickFocus);
    this->setFocus();

    setTitle(title);
    setMessage(message);
    //this->moveToCenter();
    this->m_titleLabel->setFixedWidth(this->width()-this->m_titleIcon->width()-this->closeButton->width()-20);
    this->m_titleWidth = this->m_titleLabel->width();
    onThemeFontChange(fontSize);
}

MyDialog::~MyDialog()
{
    QLayoutItem *child;
    while ((child = m_topLayout->takeAt(0)) != 0) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }

    this->buttonList.clear();
    while ((child = m_buttonLayout->takeAt(0)) != 0) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }
//    while(this->m_buttonLayout->count()) {
//        QLayoutItem *item = this->m_buttonLayout->takeAt(0);
//        item->widget()->deleteLater();
//        delete item;
//    }
    if (styleSettings) {
        delete styleSettings;
        styleSettings = nullptr;
    }
}

void MyDialog::initThemeStyle()
{
    if (!styleSettings) {
        fontSize = DEFAULT_FONT_SIZE;
        return;
    }
    connect(styleSettings,&QGSettings::changed,[=](QString key)
    {
        if ("systemFont" == key || "systemFontSize" == key) {
            fontSize = styleSettings->get(FONT_SIZE).toString().toFloat();
            this->onThemeFontChange(fontSize);
        } else if ("iconThemeName" == key) {
            if (m_titleIcon)
                m_titleIcon->setPixmap(QIcon::fromTheme("lingmo-system-monitor").pixmap(24,24));
        }
    });
    fontSize = styleSettings->get(FONT_SIZE).toString().toFloat();
}

void MyDialog::onThemeFontChange(qreal lfFontSize)
{
    Q_UNUSED(lfFontSize);
    if (m_titleLabel && this->m_titleWidth > 0) {
        QString strTitle = getElidedText(m_titleLabel->font(), m_title, this->m_titleWidth-2);
        m_titleLabel->setText(strTitle);
        if (strTitle != m_title) {
            m_titleLabel->setToolTip(m_title);
        } else {
            m_titleLabel->setToolTip("");
        }
    }
}

void MyDialog::updateSize()
{
    if (!this->testAttribute(Qt::WA_Resized)) {
        QSize size = this->sizeHint();
        size.setWidth(qMax(size.width(), 234));
        size.setHeight(qMax(size.height(), 196));
        this->resize(size);
        this->setAttribute(Qt::WA_Resized, false);
    }
}

void MyDialog::onButtonClicked()
{
    QAbstractButton *button = qobject_cast<QAbstractButton*>(this->sender());
    if(button) {
        this->close();
        clickedButtonIndex = buttonList.indexOf(button);
        emit this->buttonClicked(clickedButtonIndex, button->text());
//        this->done(clickedButtonIndex);//cancel:0   ok:1
    }
}

void MyDialog::onDefaultButtonTriggered()
{
    QAbstractButton *button = qobject_cast<QAbstractButton*>(this->focusWidget());

    if (button)
        button->click();
    else if (defaultButton)
        defaultButton->click();
}

int MyDialog::buttonCount() const
{
    return this->buttonList.count();
}

int MyDialog::addButton(const QString &text, bool isDefault)
{
    int index = buttonCount();
    QAbstractButton *button = new QPushButton(text, this);
    button->setFocusPolicy(Qt::NoFocus);
    button->setAttribute(Qt::WA_NoMousePropagation);
    button->setFixedSize(158, 36);

    this->m_buttonLayout->insertWidget(index+1, button);
    this->buttonList << button;
    connect(button, SIGNAL(clicked(bool)), this, SLOT(onButtonClicked()));

    if(isDefault) {
        setDefaultButton(button);
    }
    return index;
}

void MyDialog::addContent(QWidget* contentWidget)
{
    if (!contentWidget)
        return;
    m_contentLayout->addWidget(contentWidget);
}

void MyDialog::setDefaultButton(QAbstractButton *button)
{
    this->defaultButton = button;
}

void MyDialog::setTitle(const QString &title)
{
    if (this->m_title == title)
        return;

    this->m_title = title;
    this->m_titleLabel->setText(title);
    this->m_titleLabel->setHidden(title.isEmpty());
}

void MyDialog::setMessage(const QString &message)
{
    if (this->m_message == message)
        return;
    this->m_message = message;
    this->m_messageLabel->setText(message);
    this->m_messageLabel->setHidden(message.isEmpty());
}

int MyDialog::exec()
{
    this->clickedButtonIndex = -1;
    int ret = QDialog::exec();

    return this->clickedButtonIndex >= 0 ? this->clickedButtonIndex : ret;
}

void MyDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    setAttribute(Qt::WA_Resized, false);
    //this->updateSize();
}

void MyDialog::hideEvent(QHideEvent *event)
{
    QDialog::hideEvent(event);
    done(-1);
}

void MyDialog::childEvent(QChildEvent *event)
{
    QDialog::childEvent(event);
    if (event->added()) {
        if (this->closeButton) {
            this->closeButton->raise();
        }
    }
}

QRect MyDialog::getParentGeometry() const
{
    if (this->parentWidget()) {
        return this->parentWidget()->window()->geometry();
    }
    else {
        QPoint pos = QCursor::pos();
        for (QScreen *screen : qApp->screens()) {
            if (screen->geometry().contains(pos)) {
                return screen->geometry();
            }
        }
    }
    return qApp->primaryScreen()->geometry();
}

void MyDialog::moveToCenter()
{
    QRect qr = geometry();
    qr.moveCenter(this->getParentGeometry().center());
    move(qr.topLeft());
}

void MyDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        this->dragPosition = event->globalPos() - frameGeometry().topLeft();
        this->mousePressed = true;
    }
    QDialog::mousePressEvent(event);
}

void MyDialog::mouseReleaseEvent(QMouseEvent *event)
{
    this->mousePressed = false;
    QDialog::mouseReleaseEvent(event);
}

void MyDialog::mouseMoveEvent(QMouseEvent *event)
{
    QDialog::mouseMoveEvent(event);
}

void MyDialog::paintEvent(QPaintEvent *event)
{
    QPainterPath path;
    QPainter painter(this);

    path.addRect(this->rect());
    path.setFillRule(Qt::WindingFill);
    painter.setBrush(this->palette().base());
    painter.setPen(Qt::transparent);
    painter.drawPath(path);
    QDialog::paintEvent(event);
}

void MyDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
}
