#include "type_defines.h"

#include <QPainter>
#include <QStandardItemModel>
#include <QTimer>
#include <QtSvg/QSvgRenderer>

#ifdef linux
#    include <DFontSizeManager>
DWIDGET_USE_NAMESPACE
#endif

ButtonLayout::ButtonLayout(QWidget *parent)
    : QHBoxLayout(parent)
{
    button1 = new QPushButton(parent);
    button1->setFixedSize(120, 36);

    button2 = new CooperationSuggestButton(parent);
    button2->setFixedSize(120, 36);

#if defined(_WIN32) || defined(_WIN64)
    button1->setStyleSheet(StyleHelper::buttonStyle(StyleHelper::gray));
    button2->setStyleSheet(StyleHelper::buttonStyle(StyleHelper::blue));
#else
    StyleHelper::setAutoFont(button1, 14, QFont::Medium);
    StyleHelper::setAutoFont(button2, 14, QFont::Medium);
#endif
    addWidget(button1);
    addWidget(button2);
    setSpacing(10);
    setAlignment(Qt::AlignCenter);
}

ButtonLayout::~ButtonLayout()
{
}

void ButtonLayout::setCount(int count)
{
    switch (count) {
    case 1:
        button1->setFixedSize(250, 36);
        button2->setVisible(false);
        break;
    case 2:
        button1->setFixedSize(120, 36);
        button2->setVisible(true);
        break;
    }
}

QPushButton *ButtonLayout::getButton1() const
{
    return button1;
}

QPushButton *ButtonLayout::getButton2() const
{
    return button2;
}

QFont StyleHelper::font(int type)
{
    QFont font;
    switch (type) {
    case 1:
        font.setPixelSize(24);
        font.setWeight(QFont::DemiBold);
        break;
    case 2:
        font.setPixelSize(17);
        font.setWeight(QFont::DemiBold);
        break;
    case 3:
        font.setPixelSize(12);
        break;
    default:
        break;
    }
    return font;
}

void StyleHelper::setAutoFont(QWidget *widget, int size, int weight)
{
#ifdef linux
    switch (size) {
    case 54:
        DFontSizeManager::instance()->setFontPixelSize(DFontSizeManager::T1, 54);
        DFontSizeManager::instance()->bind(widget, DFontSizeManager::T1, weight);
        break;
    case 24:
        DFontSizeManager::instance()->bind(widget, DFontSizeManager::T3, weight);
        break;
    case 17:
        DFontSizeManager::instance()->setFontPixelSize(DFontSizeManager::T5, 17);
        DFontSizeManager::instance()->bind(widget, DFontSizeManager::T5, weight);
        break;
    case 14:
        DFontSizeManager::instance()->bind(widget, DFontSizeManager::T6, weight);
        break;
    case 12:
        DFontSizeManager::instance()->bind(widget, DFontSizeManager::T8, weight);
        break;
    case 11:
        DFontSizeManager::instance()->bind(widget, DFontSizeManager::T9, weight);
        break;
    case 10:
        DFontSizeManager::instance()->bind(widget, DFontSizeManager::T10, weight);
        break;
    default:
        DFontSizeManager::instance()->bind(widget, DFontSizeManager::T6, weight);
    }
#else
    QFont font;
    font.setPixelSize(size);
    font.setWeight(weight);
    widget->setFont(font);
#endif
}

QString StyleHelper::textStyle(StyleHelper::TextStyle type)
{
    QString style;
    switch (type) {
    case normal:
        style = "color: #000000; font-size: 12px;";
        break;
    case error:
        style = "color: #FF5736;";
        break;
    }
    return style;
}

QString StyleHelper::buttonStyle(int type)
{
    QString style;
    switch (type) {
    case gray:
        style = ".QPushButton{"
                "border-radius: 8px;"
                "opacity: 1;"
                "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 "
                "rgba(230, 230, 230, 1), stop:1 rgba(227, 227, 227, 1));"
                "font-family: \"SourceHanSansSC-Medium\";"
                "font-size: 14px;"
                "font-weight: 500;"
                "color: rgba(65,77,104,1);"
                "font-style: normal;"
                "text-align: center;"
                ";}"
                "QPushButton:disabled {"
                "border-radius: 8px;"
                "opacity: 1;"
                "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 "
                "rgba(230, 230, 230, 1), stop:1 rgba(227, 227, 227, 1));"
                "font-family: \"SourceHanSansSC-Medium\";"
                "font-size: 14px;"
                "font-weight: 500;"
                "color: rgba(65,77,104,0.5);"
                "font-style: normal;"
                "text-align: center;"
                "}";
        break;
    case blue:
        style = ".QPushButton{"
                "border-radius: 8px;"
                "opacity: 1;"
                "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 "
                "rgba(37, 183, 255, 1), stop:1 rgba(0, 152, 255, 1));"
                "font-family: \"SourceHanSansSC-Medium\";"
                "font-size: 14px;"
                "font-weight: 500;"
                "color: rgba(255,255,255,1);"
                "font-style: normal;"
                "text-align: center;"
                "}";
        break;
    }
    return style;
}

QString StyleHelper::textBrowserStyle(int type)
{
    QString style;
    switch (type) {
    case 1:
        style = "QTextBrowser {"
                "border-radius: 10px;"
                "padding-top: 10px;"
                "padding-bottom: 10px;"
                "padding-left: 5px;"
                "padding-right: 5px;"
                "font-size: 12px;"
                "font-weight: 400;"
                "color: rgb(82, 106, 127);"
                "line-height: 300%;"
                "background-color:rgba(0, 0, 0,0.08);}";
        break;
    case 0:
        style = "QTextBrowser {"
                "border-radius: 10px;"
                "padding-top: 10px;"
                "padding-bottom: 10px;"
                "padding-left: 5px;"
                "padding-right: 5px;"
                "font-size: 12px;"
                "font-weight: 400;"
                "color: rgb(82, 106, 127);"
                "line-height: 300%;"
                "background-color:rgba(255,255,255, 0.1);}";
        break;
    }
    return style;
}

IndexLabel::IndexLabel(int index, QWidget *parent)
    : QLabel(parent), index(index)
{
    setFixedSize(60, 10);
}

void IndexLabel::setIndex(int i)
{
    index = i;
    update();
}

void IndexLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    int diam = 6;

    QColor brushColor;
    brushColor.setNamedColor("#0081FF");
    for (int i = 0; i < 4; i++) {
        if (i == index)
            brushColor.setAlpha(190);
        else
            brushColor.setAlpha(40);

        painter.setBrush(brushColor);
        painter.drawEllipse((diam + 8) * i + 6, 0, diam, diam);
    }
}

MovieWidget::MovieWidget(QString filename, QWidget *parent)
    : QWidget(parent), movie(filename)
{
    setFixedSize(200, 160);
    loadFrames();
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MovieWidget::nextFrame);
    timer->start(50);
}

void MovieWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.drawPixmap(0, 0, frames[currentFrame]);
}

void MovieWidget::nextFrame()
{
    currentFrame = (currentFrame + 1) % frames.size();
    update();
}

void MovieWidget::loadFrames()
{
    for (int i = 0; i <= 49; ++i) {
        QPixmap frame = QIcon(":/icon/movie/" + movie + "/" + movie + QString::number(i) + ".png")
                                .pixmap(200, 160);
        frames.append(frame);
    }
}

ProcessDetailsWindow::ProcessDetailsWindow(QFrame *parent)
    : QListView(parent)
{
}

ProcessDetailsWindow::~ProcessDetailsWindow() {}

void ProcessDetailsWindow::clear()
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel *>(this->model());
    model->clear();
}

ProcessWindowItemDelegate::ProcessWindowItemDelegate()
{
}

ProcessWindowItemDelegate::~ProcessWindowItemDelegate() {}

void ProcessWindowItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    paintIcon(painter, option, index);
    paintText(painter, option, index);
}

QSize ProcessWindowItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    return QSize(100, 20);
}

void ProcessWindowItemDelegate::setTheme(int newTheme)
{
    theme = newTheme;
}

void ProcessWindowItemDelegate::addIcon(const QString &path)
{
    QPixmap pixmap = QIcon(path).pixmap(20, 20);
    pixmaps.push_back(pixmap);
}

void ProcessWindowItemDelegate::setStageColor(QColor color)
{
    stageTextColor = color;
}

void ProcessWindowItemDelegate::paintText(QPainter *painter, const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
{
    painter->save();
    QString processName = index.data(Qt::DisplayRole).toString();
    QString processStage = index.data(Qt::ToolTipRole).toString();
    int StatusTipRole = index.data(Qt::StatusTipRole).toInt();
    QColor fontNameColor;
    QColor fontStageColor;
    if (theme == 0) {
        fontNameColor = QColor(255, 255, 255, 155);
        fontStageColor = QColor(123, 159, 191, 255);
    } else {
        fontNameColor = QColor(0, 0, 0, 155);
        fontStageColor = QColor(0, 130, 250, 100);
    }
    if (StatusTipRole != 0) {
        fontStageColor = stageTextColor;
    }
#ifdef linux
    QFont font(qApp->font());
#else
    QFont font;
    font.setPixelSize(14);
#endif
    font.setPixelSize(QFontInfo(font).pixelSize() - 2);
    QPen textNamePen(fontNameColor);
    painter->setFont(font);
    painter->setPen(textNamePen);

    QRect remarkTextPos;
    if (!pixmaps.isEmpty()) {
        remarkTextPos = option.rect.adjusted(40, 0, 0, 0);
    } else {
        remarkTextPos = option.rect.adjusted(20, 0, 0, 0);
    }
    painter->drawText(remarkTextPos, Qt::AlignLeft | Qt::AlignVCenter, processName);

    QFontMetrics fontMetrics(font);
    int firstTextWidth = fontMetrics.horizontalAdvance(processName);

    QPen textStagePen(fontStageColor);
    painter->setPen(textStagePen);
    remarkTextPos.adjust(firstTextWidth + 20, 0, 0, 0);
    painter->drawText(remarkTextPos, Qt::AlignLeft | Qt::AlignVCenter, processStage);

    painter->restore();
}

void ProcessWindowItemDelegate::paintIcon(QPainter *painter, const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
{
    if (pixmaps.isEmpty())
        return;

    painter->save();
    int num = index.data(Qt::UserRole).toInt();
    QPoint pos = option.rect.topLeft();
    QRect iconRect(pos.x() + 10, pos.y(), 20, 20);
    QPixmap pixmap = pixmaps[num];
    painter->drawPixmap(iconRect, pixmap);
    painter->restore();
}
