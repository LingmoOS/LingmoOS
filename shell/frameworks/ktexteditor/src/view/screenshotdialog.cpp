/*
    SPDX-FileCopyrightText: 2023 Waqar Ahmed <waqar.17a@gmail.com>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "screenshotdialog.h"

#include "katedocument.h"
#include "kateglobal.h"
#include "katelinelayout.h"
#include "katerenderer.h"
#include "kateview.h"

#include <QActionGroup>
#include <QApplication>
#include <QBitmap>
#include <QCheckBox>
#include <QClipboard>
#include <QColorDialog>
#include <QDebug>
#include <QDrag>
#include <QFileDialog>
#include <QGraphicsDropShadowEffect>
#include <QImageWriter>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSyntaxHighlighting/Theme>

using namespace KTextEditor;

static constexpr QPoint noDragStartCandidatePos = {-1, -1};

class BaseWidget : public QWidget
{
public:
    explicit BaseWidget(QWidget *parent = nullptr)
        : QWidget(parent)
        , m_screenshot(new QLabel(this))
    {
        setAutoFillBackground(true);
        setContentsMargins({});
        auto layout = new QHBoxLayout(this);
        setColor(Qt::yellow);

        layout->addStretch();
        layout->addWidget(m_screenshot);
        layout->addStretch();

        m_renableEffects.setInterval(500);
        m_renableEffects.setSingleShot(true);
        m_renableEffects.callOnTimeout(this, &BaseWidget::enableDropShadow);
    }

    void setColor(QColor c)
    {
        auto p = palette();
        p.setColor(QPalette::Base, c);
        p.setColor(QPalette::Window, c);
        setPalette(p);
    }

    void setPixmap(const QPixmap &p)
    {
        temporarilyDisableDropShadow();

        m_screenshot->setPixmap(p);
        m_screenshotSize = p.size();
    }

    QPixmap grabPixmap()
    {
        const int h = m_screenshotSize.height();
        const int y = std::max(((height() - h) / 2), 0);
        const int x = m_screenshot->geometry().x();
        QRect r(x, y, m_screenshotSize.width(), m_screenshotSize.height());
        r.adjust(-6, -6, 6, 6);
        return grab(r);
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        QWidget *childAtEvent = childAt(event->pos());
        if ((childAtEvent != m_screenshot) || (event->buttons() != Qt::LeftButton)) {
            m_dragStartCandidatePos = noDragStartCandidatePos;
            QWidget::mousePressEvent(event);
            return;
        }

        m_dragStartCandidatePos = event->pos();
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        if ((m_dragStartCandidatePos == noDragStartCandidatePos) || (event->buttons() != Qt::LeftButton)
            || ((event->pos() - m_dragStartCandidatePos).manhattanLength() < QApplication::startDragDistance())) {
            QWidget::mouseMoveEvent(event);
            return;
        }

        const QPixmap pixmap = grabPixmap();

        auto *mimeData = new QMimeData;
        mimeData->setImageData(pixmap);

        auto *drag = new QDrag(this);
        drag->setMimeData(mimeData);

        // 256x256, following size used by spectacle 24.05
        drag->setPixmap(pixmap.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation));

        drag->exec(Qt::CopyAction);
    }

    void temporarilyDisableDropShadow()
    {
        // Disable drop shadow because on large pixmaps
        // it is too slow
        m_screenshot->setGraphicsEffect(nullptr);
        m_renableEffects.start();
    }

private:
    void enableDropShadow()
    {
        QGraphicsDropShadowEffect *e = new QGraphicsDropShadowEffect(m_screenshot);
        e->setColor(Qt::black);
        e->setOffset(2.);
        e->setBlurRadius(15.);
        m_screenshot->setGraphicsEffect(e);
    }

    QLabel *const m_screenshot;
    QSize m_screenshotSize;
    QTimer m_renableEffects;
    QPoint m_dragStartCandidatePos;

    friend class ScrollArea;
};

class ScrollArea : public QScrollArea
{
public:
    explicit ScrollArea(BaseWidget *contents, QWidget *parent = nullptr)
        : QScrollArea(parent)
        , m_base(contents)
    {
    }

private:
    void scrollContentsBy(int dx, int dy) override
    {
        m_base->temporarilyDisableDropShadow();
        QScrollArea::scrollContentsBy(dx, dy);
    }

private:
    BaseWidget *const m_base;
};

ScreenshotDialog::ScreenshotDialog(KTextEditor::Range selRange, KTextEditor::ViewPrivate *parent)
    : QDialog(parent)
    , m_base(new BaseWidget(this))
    , m_selRange(selRange)
    , m_scrollArea(new ScrollArea(m_base, this))
    , m_saveButton(new QPushButton(QIcon::fromTheme(QStringLiteral("document-save")), i18n("Save")))
    , m_copyButton(new QPushButton(QIcon::fromTheme(QStringLiteral("edit-copy")), i18n("Copy")))
    , m_changeBGColor(new QPushButton(QIcon::fromTheme(QStringLiteral("color-fill")), i18nc("@action:button", "Background Color…")))
    , m_lineNumButton(new QToolButton(this))
    , m_extraDecorations(new QCheckBox(i18n("Show Extra Decorations"), this))
    , m_windowDecorations(new QCheckBox(i18n("Show Window Decorations"), this))
    , m_lineNumMenu(new QMenu(this))
    , m_resizeTimer(new QTimer(this))
{
    setModal(true);
    setWindowTitle(i18nc("@title:window", "Take Screenshot of Selection"));

    m_scrollArea->setWidget(m_base);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setAutoFillBackground(true);
    m_scrollArea->setAttribute(Qt::WA_Hover, false);
    m_scrollArea->setFrameStyle(QFrame::NoFrame);

    auto baseLayout = new QVBoxLayout(this);
    baseLayout->setContentsMargins(0, 0, 0, 4);
    baseLayout->addWidget(m_scrollArea);

    KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("KTextEditor::Screenshot"));
    const int color = cg.readEntry("BackgroundColor", EditorPrivate::self()->theme().textColor(KSyntaxHighlighting::Theme::Normal));
    const auto c = QColor::fromRgba(color);
    m_base->setColor(c);
    m_scrollArea->setPalette(m_base->palette());

    auto bottomBar = new QHBoxLayout();
    baseLayout->addLayout(bottomBar);
    bottomBar->setContentsMargins(0, 0, 4, 0);
    bottomBar->addStretch();
    bottomBar->addWidget(m_windowDecorations);
    bottomBar->addWidget(m_extraDecorations);
    bottomBar->addWidget(m_lineNumButton);
    bottomBar->addWidget(m_changeBGColor);
    bottomBar->addWidget(m_saveButton);
    bottomBar->addWidget(m_copyButton);
    connect(m_saveButton, &QPushButton::clicked, this, &ScreenshotDialog::onSaveClicked);
    connect(m_copyButton, &QPushButton::clicked, this, &ScreenshotDialog::onCopyClicked);
    connect(m_changeBGColor, &QPushButton::clicked, this, [this] {
        QColorDialog dlg(this);
        int e = dlg.exec();
        if (e == QDialog::Accepted) {
            QColor c = dlg.selectedColor();
            m_base->setColor(c);
            m_scrollArea->setPalette(m_base->palette());

            KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("KTextEditor::Screenshot"));
            cg.writeEntry("BackgroundColor", c.rgba());
        }
    });

    connect(m_extraDecorations, &QCheckBox::toggled, this, [this] {
        renderScreenshot(static_cast<KTextEditor::ViewPrivate *>(parentWidget())->renderer());
        KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("KTextEditor::Screenshot"));
        cg.writeEntry<bool>("ShowExtraDecorations", m_extraDecorations->isChecked());
    });
    m_extraDecorations->setChecked(cg.readEntry<bool>("ShowExtraDecorations", true));

    connect(m_windowDecorations, &QCheckBox::toggled, this, [this] {
        renderScreenshot(static_cast<KTextEditor::ViewPrivate *>(parentWidget())->renderer());
        KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("KTextEditor::Screenshot"));
        cg.writeEntry<bool>("ShowWindowDecorations", m_windowDecorations->isChecked());
    });
    m_windowDecorations->setChecked(cg.readEntry<bool>("ShowWindowDecorations", true));

    {
        KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("KTextEditor::Screenshot"));
        int i = cg.readEntry("LineNumbers", (int)ShowAbsoluteLineNums);

        auto gp = new QActionGroup(m_lineNumMenu);
        auto addMenuAction = [this, gp](const QString &text, int data) {
            auto a = new QAction(text, m_lineNumMenu);
            a->setCheckable(true);
            a->setActionGroup(gp);
            m_lineNumMenu->addAction(a);
            connect(a, &QAction::triggered, this, [this, data] {
                onLineNumChangedClicked(data);
            });
            return a;
        };
        addMenuAction(i18n("Don't Show Line Numbers"), DontShowLineNums)->setChecked(i == DontShowLineNums);
        addMenuAction(i18n("Show Line Numbers From 1"), ShowAbsoluteLineNums)->setChecked(i == ShowAbsoluteLineNums);
        addMenuAction(i18n("Show Actual Line Numbers"), ShowActualLineNums)->setChecked(i == ShowActualLineNums);

        m_showLineNumbers = i != DontShowLineNums;
        m_absoluteLineNumbers = i == ShowAbsoluteLineNums;
    }

    m_lineNumButton->setText(i18n("Line Numbers"));
    m_lineNumButton->setPopupMode(QToolButton::InstantPopup);
    m_lineNumButton->setMenu(m_lineNumMenu);

    m_resizeTimer->setSingleShot(true);
    m_resizeTimer->setInterval(500);
    m_resizeTimer->callOnTimeout(this, [this] {
        renderScreenshot(static_cast<KTextEditor::ViewPrivate *>(parentWidget())->renderer());
        KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("KTextEditor::Screenshot"));
        cg.writeEntry("Geometry", saveGeometry());
    });

    const QByteArray geometry = cg.readEntry("Geometry", QByteArray());
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
}

ScreenshotDialog::~ScreenshotDialog()
{
    m_resizeTimer->stop();
}

void ScreenshotDialog::renderScreenshot(KateRenderer *r)
{
    if (m_selRange.isEmpty()) {
        return;
    }

    constexpr int leftMargin = 16;
    constexpr int rightMargin = 16;
    constexpr int topMargin = 8;
    constexpr int bottomMargin = 8;
    constexpr int lnNoAreaSpacing = 8;

    KateRenderer renderer(r->doc(), r->folding(), r->view());
    renderer.setPrinterFriendly(!m_extraDecorations->isChecked());

    int startLine = m_selRange.start().line();
    int endLine = m_selRange.end().line();

    int width = std::min(1024, std::max(400, this->width() - (m_scrollArea->horizontalScrollBar()->height())));

    // If the font is fixed width, try to find the best width
    const bool fixedWidth = QFontInfo(renderer.currentFont()).fixedPitch();
    if (fixedWidth) {
        int maxLineWidth = 0;
        auto doc = renderer.view()->doc();
        int w = renderer.currentFontMetrics().averageCharWidth();
        for (int line = startLine; line <= endLine; ++line) {
            maxLineWidth = std::max(maxLineWidth, (doc->lineLength(line) * w));
        }

        const int windowWidth = width;
        if (maxLineWidth > windowWidth) {
            maxLineWidth = windowWidth;
        }

        width = std::min(1024, maxLineWidth);
        width = std::max(400, width);
    }

    // Collect line layouts and calculate the needed height
    const int xEnd = width;
    int height = 0;
    std::vector<std::unique_ptr<KateLineLayout>> lineLayouts;
    for (int line = startLine; line <= endLine; ++line) {
        auto lineLayout = std::make_unique<KateLineLayout>(renderer);
        lineLayout->setLine(line, -1);
        renderer.layoutLine(lineLayout.get(), xEnd, false /* no layout cache */);
        height += lineLayout->viewLineCount() * renderer.lineHeight();
        lineLayouts.push_back(std::move(lineLayout));
    }

    if (m_windowDecorations->isChecked()) {
        height += renderer.lineHeight() + topMargin + bottomMargin;
    } else {
        height += topMargin + bottomMargin; // topmargin
    }

    int xStart = -leftMargin;
    int lineNoAreaWidth = 0;
    if (m_showLineNumbers) {
        int lastLine = m_absoluteLineNumbers ? (endLine - startLine) + 1 : endLine;
        const int lnNoWidth = renderer.currentFontMetrics().horizontalAdvance(QString::number(lastLine));
        lineNoAreaWidth = lnNoWidth + lnNoAreaSpacing;
        width += lineNoAreaWidth;
        xStart += -lineNoAreaWidth;
    }

    width += leftMargin + rightMargin;
    QPixmap pix(width, height);
    pix.fill(renderer.view()->rendererConfig()->backgroundColor());

    QPainter paint(&pix);

    paint.translate(0, topMargin);

    if (m_windowDecorations->isChecked()) {
        int midY = (renderer.lineHeight() + 4) / 2;
        int x = 24;
        paint.save();
        paint.setRenderHint(QPainter::Antialiasing, true);
        paint.setPen(Qt::NoPen);

        QBrush b(QColor(0xff5f5a)); // red
        paint.setBrush(b);
        paint.drawEllipse(QPoint(x, midY), 8, 8);

        x += 24;
        b = QColor(0xffbe2e);
        paint.setBrush(b);
        paint.drawEllipse(QPoint(x, midY), 8, 8);

        x += 24;
        b = QColor(0x2aca44);
        paint.setBrush(b);
        paint.drawEllipse(QPoint(x, midY), 8, 8);

        paint.setRenderHint(QPainter::Antialiasing, false);
        paint.restore();

        paint.translate(0, renderer.lineHeight() + 4);
    }

    KateRenderer::PaintTextLineFlags flags;
    flags.setFlag(KateRenderer::SkipDrawFirstInvisibleLineUnderlined);
    flags.setFlag(KateRenderer::SkipDrawLineSelection);
    int lineNo = m_absoluteLineNumbers ? 1 : startLine + 1;
    paint.setFont(renderer.currentFont());
    for (auto &lineLayout : lineLayouts) {
        renderer.paintTextLine(paint, lineLayout.get(), xStart, xEnd, QRectF{}, nullptr, flags);
        // draw line number
        if (lineNoAreaWidth != 0) {
            paint.drawText(QRect(leftMargin - lnNoAreaSpacing, 0, lineNoAreaWidth, renderer.lineHeight()),
                           Qt::TextDontClip | Qt::AlignRight | Qt::AlignVCenter,
                           QString::number(lineNo++));
        }
        // translate for next line
        paint.translate(0, lineLayout->viewLineCount() * renderer.lineHeight());
    }

    m_base->setPixmap(pix);
}

void ScreenshotDialog::onSaveClicked()
{
    const auto name = QFileDialog::getSaveFileName(this);
    if (name.isEmpty()) {
        return;
    }

    QImageWriter writer(name);
    writer.write(m_base->grabPixmap().toImage());
    if (!writer.errorString().isEmpty()) {
        QMessageBox::warning(this, i18nc("@title:window", "Screenshot saving failed"), i18n("Screenshot saving failed: %1", writer.errorString()));
    }
}

void ScreenshotDialog::onCopyClicked()
{
    if (auto clip = qApp->clipboard()) {
        clip->setPixmap(m_base->grabPixmap(), QClipboard::Clipboard);
    }
}

void ScreenshotDialog::resizeEvent(QResizeEvent *e)
{
    QDialog::resizeEvent(e);
    if (!m_firstShow) {
        m_resizeTimer->start();
    }
    m_firstShow = false;
}

void ScreenshotDialog::onLineNumChangedClicked(int i)
{
    m_showLineNumbers = i != DontShowLineNums;
    m_absoluteLineNumbers = i == ShowAbsoluteLineNums;

    KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("KTextEditor::Screenshot"));
    cg.writeEntry("LineNumbers", i);

    renderScreenshot(static_cast<KTextEditor::ViewPrivate *>(parentWidget())->renderer());
}
