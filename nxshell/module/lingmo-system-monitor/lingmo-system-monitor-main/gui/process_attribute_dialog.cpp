// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "process_attribute_dialog.h"

#include "settings.h"
#include "common/common.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DFontSizeManager>
#include <DFrame>
#include <DLabel>
#include <DTextBrowser>
#include <DTitlebar>

#include <QAbstractTextDocumentLayout>
#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QTextBlock>
#include <QVBoxLayout>
#include <QtMath>
#include <QScrollBar>

// prefered text width
static const int kPreferedTextWidth = 260;
// default icon size
static const int kAppIconSize = 80;

// constructor
ProcessAttributeDialog::ProcessAttributeDialog(pid_t pid,
                                               const QString &procName,
                                               const QString &displayName,
                                               const QString &cmdline,
                                               const QIcon &icon,
                                               time_t startTime,
                                               QWidget *parent)
    : DMainWindow(parent)
    , m_procName(procName)
    , m_displayName(displayName)
    , m_cmdline(cmdline)
    , m_startTime(startTime)
    , m_icon(icon)
    , m_pid(pid)
{
    // get backup settings instance
    m_settings = Settings::instance();
    initUI();

    // install event filters for dialog itself & text contents
    installEventFilter(this);
    m_procNameText->viewport()->installEventFilter(this);
    m_procCmdText->viewport()->installEventFilter(this);
    m_procStartText->viewport()->installEventFilter(this);
}

// initialize ui components
void ProcessAttributeDialog::initUI()
{
    m_margin = 10;
    // restore settings from backup config if previous size found, otherwise use default size
    int width = m_settings->getOption(kSettingKeyProcessAttributeDialogWidth, 600).toInt();
    int height = m_settings->getOption(kSettingKeyProcessAttributeDialogHeight, 480).toInt();

    // background frame
    m_frame = new DWidget(this);
    m_frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_frame->setAutoFillBackground(true);

    // resize dialog
    resize(width, height);
    // set minimum size
    setMinimumSize(600, 480);
    // set window attributes
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::WindowModal);
    setWindowFlags(Qt::Dialog);
    // hide menu on titlebar
    titlebar()->setQuitMenuDisabled(true);
    titlebar()->setMenuVisible(false);
    setTitlebarShadowEnabled(false);
    // set dialog title
    titlebar()->setTitle({});

    // shadow bar instance
    m_tbShadow = new DShadowLine(m_frame);
    m_tbShadow->setFixedWidth(m_frame->width());
    m_tbShadow->setFixedHeight(10);
    m_tbShadow->move(0, 0);
    m_tbShadow->raise();
    m_tbShadow->show();

    // frame layout
    auto *vlayout = new QVBoxLayout(m_frame);
    vlayout->setMargin(m_margin);
    vlayout->setSpacing(m_margin);

    vlayout->addStretch(1);

    DLabel *appIcon = new DLabel(this);
    vlayout->addWidget(appIcon, 0, Qt::AlignCenter);

    // process display name label
    m_appNameLabel = new DLabel(this);
    DFontSizeManager::instance()->bind(m_appNameLabel, DFontSizeManager::T5, QFont::Bold);
    // elide display name text in the middle
    m_appNameLabel->setElideMode(Qt::ElideMiddle);
    m_appNameLabel->setAlignment(Qt::AlignCenter);
    vlayout->addWidget(m_appNameLabel, 0, Qt::AlignCenter);

    // background frame
    auto *wnd = new QWidget(m_frame);
    wnd->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    wnd->setAutoFillBackground(false);
    // content grid layout
    auto *grid = new QGridLayout(wnd);
    grid->setHorizontalSpacing(m_margin);
    grid->setVerticalSpacing(0);
    grid->setMargin(0);

    QString buf;
    buf = QString("%1:").arg(DApplication::translate("Process.Attributes.Dialog", "Name"));
    // process name label
    m_procNameLabel = new DLabel(wnd);
    m_procNameLabel->setText(buf);
    m_procNameLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    buf = QString("%1:").arg(DApplication::translate("Process.Attributes.Dialog", "Command line"));
    // process command label
    m_procCmdLabel = new DLabel(wnd);
    m_procCmdLabel->setText(buf);
    m_procCmdLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    buf = QString("%1:").arg(DApplication::translate("Process.Attributes.Dialog", "Start time"));
    // process start time label
    m_procStartLabel = new DLabel(wnd);
    m_procStartLabel->setText(buf);
    m_procStartLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // process name text
    m_procNameText = new DTextBrowser(wnd);
    m_procNameText->setFrameStyle(QFrame::NoFrame);
    m_procNameText->setFocusPolicy(Qt::ClickFocus);
    m_procNameText->setReadOnly(true);
    m_procNameText->viewport()->setBackgroundRole(QPalette::Window);
    m_procNameText->setWordWrapMode(QTextOption::WrapAnywhere);
    m_procNameText->document()->setDocumentMargin(0);

    // process command line text
    m_procCmdText = new DTextBrowser(wnd);
    m_procCmdText->setFrameStyle(QFrame::NoFrame);
    m_procCmdText->setFocusPolicy(Qt::ClickFocus);
    m_procCmdText->setReadOnly(true);
    m_procCmdText->viewport()->setBackgroundRole(QPalette::Window);
    m_procCmdText->setWordWrapMode(QTextOption::WrapAnywhere);
    m_procCmdText->document()->setDocumentMargin(0);

    // process start time text
    m_procStartText = new DTextBrowser(wnd);
    m_procStartText->setFrameStyle(QFrame::NoFrame);
    m_procStartText->setFocusPolicy(Qt::ClickFocus);
    m_procStartText->setReadOnly(true);
    m_procStartText->viewport()->setBackgroundRole(QPalette::Window);
    m_procStartText->setWordWrapMode(QTextOption::WrapAnywhere);
    m_procStartText->document()->setDocumentMargin(0);

    grid->addWidget(m_procNameLabel, 0, 0, Qt::AlignTop | Qt::AlignRight);
    grid->addWidget(m_procNameText, 0, 1, Qt::AlignTop | Qt::AlignLeft);

    grid->addWidget(m_procCmdLabel, 1, 0, Qt::AlignTop | Qt::AlignRight);
    grid->addWidget(m_procCmdText, 1, 1, Qt::AlignTop | Qt::AlignLeft);

    grid->addWidget(m_procStartLabel, 2, 0, Qt::AlignTop | Qt::AlignRight);
    grid->addWidget(m_procStartText, 2, 1, Qt::AlignTop | Qt::AlignLeft);
    wnd->setLayout(grid);
    vlayout->addWidget(wnd, 0, Qt::AlignCenter);

    // spacing
    vlayout->addStretch(1);

    // fill icon & text content
    appIcon->setFixedSize(kAppIconSize, kAppIconSize);
    appIcon->setPixmap(m_icon.pixmap(kAppIconSize, kAppIconSize));
    m_appNameLabel->setText(m_displayName);
    m_appNameLabel->setToolTip(QString("<div>%1</div>").arg(m_displayName));
    m_procNameText->setText(m_procName);
    m_procCmdText->setText(m_cmdline);
    // format date time (ISO style)
    m_procStartText->setText(QDateTime::fromSecsSinceEpoch(qint64(m_startTime)).toString("yyyy-MM-dd hh:mm:ss"));

    //如果命令行无值，隐藏该行
    if(m_cmdline == "")
        m_procCmdLabel->hide();
    else
        m_procCmdLabel->show();

    m_frame->setLayout(vlayout);
    setCentralWidget(m_frame);
}

// resize event handler
void ProcessAttributeDialog::resizeEvent(QResizeEvent *event)
{
    DMainWindow::resizeEvent(event);

    // raise shadow widget on top
    if (m_frame) {
        m_tbShadow->setFixedWidth(m_frame->size().width());
        m_tbShadow->raise();
        m_tbShadow->show();
    }

    resizeItemWidget();
}

void ProcessAttributeDialog::resizeItemWidget()
{
    m_procNameLabel->setFixedSize(m_procNameLabel->fontMetrics().width(m_procNameLabel->text()), m_procNameLabel->fontMetrics().height());
    m_procCmdLabel->setFixedSize(m_procCmdLabel->fontMetrics().width(m_procCmdLabel->text()), m_procCmdLabel->fontMetrics().height());
    m_procStartLabel->setFixedSize(m_procStartLabel->fontMetrics().width(m_procStartLabel->text()), m_procStartLabel->fontMetrics().height());

    int procNametextH = qMin(120, int(m_procNameText->document()->size().height()));
    m_procNameText->setFixedSize(qMin(kPreferedTextWidth, m_procNameText->fontMetrics().size(Qt::TextSingleLine, m_procNameText->toPlainText()).width()), procNametextH);
    m_procStartText->setFixedSize(qMin(kPreferedTextWidth, m_procStartText->fontMetrics().size(Qt::TextSingleLine, m_procStartText->toPlainText()).width()), m_procStartLabel->fontMetrics().height());

    m_cmdh = m_frame->height() - (kAppIconSize + m_appNameLabel->height() + m_margin * 4) - m_procNameText->height() - m_procStartText->height();
    if (m_cmdh > m_procCmdText->document()->size().height()) {
        m_cmdh = int(m_procCmdText->document()->size().height());
    }
    m_procCmdText->setFixedSize(qMin(kPreferedTextWidth, m_procCmdText->fontMetrics().size(Qt::TextSingleLine, m_procCmdText->toPlainText()).width()), m_cmdh);
    m_procCmdText->verticalScrollBar()->setValue(0);
}

// close event handler
void ProcessAttributeDialog::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    DMainWindow::closeEvent(event);
    m_settings->setOption(kSettingKeyProcessAttributeDialogWidth, width());
    m_settings->setOption(kSettingKeyProcessAttributeDialogHeight, height());
}

// event filters
bool ProcessAttributeDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Show || event->type() == QEvent::FontChange) {
        resizeItemWidget();
    } else if (event->type() == QEvent::KeyPress) {
        // key press event
        if (obj == this) {
            auto *kev = dynamic_cast<QKeyEvent *>(event);
            // handle ESC key press event
            if (kev->matches(QKeySequence::Cancel)) {
                close();
                return true;
            }
        }
    } else if (event->type() == QEvent::MouseButtonPress) {
        // mouse button press event
        auto *ev = dynamic_cast<QMouseEvent *>(event);
        if (ev->button() & Qt::LeftButton) {
            DTextBrowser *textbrowser = dynamic_cast<DTextBrowser *>(obj->parent());
            if (textbrowser) textbrowser->setFocus();

            auto cur = m_procNameText->textCursor();
            cur.clearSelection();
            m_procNameText->setTextCursor(cur);

            cur = m_procCmdText->textCursor();
            cur.clearSelection();
            m_procCmdText->setTextCursor(cur);

            cur = m_procStartText->textCursor();
            cur.clearSelection();
            m_procStartText->setTextCursor(cur);
        }
    }

    return DMainWindow::eventFilter(obj, event);
}
