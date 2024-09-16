// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textpreview_global.h"
#include "textview.h"
#include "global/commontools.h"

#include <QHBoxLayout>
#include <QStackedWidget>
#include <QScrollBar>
#include <QTextCodec>
#include <QTimer>
#include <QPainter>
#include <QPainterPath>
#include <QLabel>

GRANDSEARCH_USE_NAMESPACE
using namespace GrandSearch::text_preview;

void PlainTextEdit::mouseMoveEvent(QMouseEvent *e)
{
    //解决鼠标按住向下滑动，当超出本控件区域后会触发scrollbar滚动
    //屏蔽该事件，不再激活autoScrollTimer
    if (e->source() == Qt::MouseEventNotSynthesized) {
        e->accept();
        return;
    }

    QPlainTextEdit::mouseMoveEvent(e);
}

QString TextView::toUnicode(const QByteArray &data)
{
    QString text;
    if (data.isEmpty())
        return text;

    //优先判断
    static const QByteArrayList maybe = {"UTF-8", "GBK"};
    for (const QByteArray &code : maybe) {
        if (QTextCodec *codec = QTextCodec::codecForName(code)) {
            QTextCodec::ConverterState state;
            text = codec->toUnicode(data.constData(), data.size(), &state);
            if (state.invalidChars < 1)
                return text;
        }
    }

    return QString::fromLocal8Bit(data);
}

void TextView::showErrorPage()
{
    //重设边距
    layout()->setContentsMargins(10, 0, 0, 0);
    m_stackedWidget->setCurrentWidget(m_errLabel);

    static int width = 360;
    static int height = 386;
    QImage errImg(":/icons/file_damaged.svg");
    errImg = errImg.scaled(70, 70);
    errImg = CommonTools::creatErrorImage({width, height}, errImg);

    QPixmap roundPixmap(width, height);
    roundPixmap.fill(Qt::transparent);
    QPainter painter(&roundPixmap);
    painter.setRenderHints(QPainter::Antialiasing, true);           // 抗锯齿
    painter.setRenderHints(QPainter::SmoothPixmapTransform, true);  // 平滑

    QPainterPath path;
    QRect rect(0, 0, width, height);
    path.addRoundedRect(rect, 8, 8);            // 圆角
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, width, height, QPixmap::fromImage(errImg));

    m_errLabel->setPixmap(roundPixmap);
}

void TextView::paintEvent(QPaintEvent *event)
{
    if (m_stackedWidget->currentWidget() == m_browser) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        //文本框的背景
        auto view = m_browser->viewport();
        painter.setBrush(view->palette().color(view->backgroundRole()));
        painter.setPen(Qt::NoPen);

        //画圆角背景,背景大小为去除左边距10的区域
        auto r = rect();
        r.setLeft(10);
        painter.drawRoundedRect(r, 8, 8);
    }

    QWidget::paintEvent(event);
}


TextView::TextView(QWidget *parent) : QWidget(parent)
{
}

void TextView::initUI()
{
    auto layout = new QHBoxLayout(this);
    this->setLayout(layout);

    layout->setSpacing(0);
    this->setMinimumHeight(386);

    m_errLabel = new QLabel(this);
    m_stackedWidget = new QStackedWidget(this);
    m_stackedWidget->setContentsMargins(0, 0, 0, 0);

    m_browser = new PlainTextEdit(this);

    //文本界面不绘制背景，自绘圆角背景
    m_browser->viewport()->setAutoFillBackground(false);
    m_browser->setFrameShape(QFrame::NoFrame);

    //无滚动
    m_browser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_browser->verticalScrollBar()->setDisabled(true);
    m_browser->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_browser->horizontalScrollBar()->setDisabled(true);

    //无交互，自动换行
    m_browser->setReadOnly(true);
    m_browser->setTextInteractionFlags(Qt::NoTextInteraction);
    m_browser->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    m_browser->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    m_browser->setFocusPolicy(Qt::NoFocus);

    //样式
    //文本内容上边距是10,左右边距是20,通过DocumentMargin设置10的边距
    //再通过layout增加左右边距各10，来达到上边距与左右边距不一样的效果
    m_browser->document()->setDocumentMargin(10);
    //左边距文本背景与中线10,加上文本内容边距10;补充文本边距10
    //因此在绘制圆角背景时为只去除左边距离中线10的区域
    layout->setContentsMargins(10 + 10, 0, 0 + 10, 0);

    m_stackedWidget->addWidget(m_browser);
    m_stackedWidget->addWidget(m_errLabel);
    m_stackedWidget->setCurrentWidget(m_browser);
    layout->addWidget(m_stackedWidget);
}

void TextView::setSource(const QString &path)
{
    m_browser->clear();

    QFile file(path);
    if (file.open(QFile::ReadOnly)) {
        //恢复边距
        layout()->setContentsMargins(10 + 10, 0, 0 + 10, 0);
        m_stackedWidget->setCurrentWidget(m_browser);
        auto datas = file.read(2048);
        m_browser->setPlainText(toUnicode(datas));
    } else {
        showErrorPage();
    }
}
