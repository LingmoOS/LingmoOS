#include <QPainter>
#include <QStyleOption>
#include <QDebug>
#include <gsettings.hpp>
#include "ocrresultwidget.h"
#include "controller/interaction.h"

OCRResultWidget::OCRResultWidget(QWidget *parent) : QTextEdit(parent)
{
    this->setReadOnly(true);
    this->setFrameShape(QFrame::NoFrame);
    this->setFontPointSize(10.5);
    this->setThemeStyle();
    this->setFormatedText(tr("OCR recognition..."));
    connect(kdk::kabase::Gsettings::getPoint(), &kdk::kabase::Gsettings::systemThemeChange, this,
            &OCRResultWidget::setThemeStyle);
}

void OCRResultWidget::setFormatedText(QString plainText)
{

    this->clear();
    plainText = plainText.isEmpty() ? tr("No text dected") : plainText;
    QTextCursor textCursor = this->textCursor();
    QTextBlockFormat textFormat;
    if (plainText == tr("OCR recognition...") || plainText == tr("No text dected")) {
        textFormat.setAlignment(Qt::AlignCenter);
        textFormat.setTopMargin(this->height() / 2 - 40 - 14);
    } else {
        QString locale = QLocale::system().name();
        if (locale == "kk_KZ" || locale =="ug_CN" || locale == "ky_KG") {
            textFormat.setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        } else {
            textFormat.setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        }
        textFormat.setLeftMargin(35);
        textFormat.setRightMargin(35);
        textFormat.setLineHeight(21, QTextBlockFormat::FixedHeight);
    }
    textCursor.insertBlock(textFormat);
    textCursor.insertText(plainText);
}

void OCRResultWidget::setFormatedText(QVector<QString> textContain)
{
    QString result;
    for (QString line : textContain) {
        if (line.isEmpty()) {
            continue;
        }
        result += line + '\n';
    }
    setFormatedText(result);
}

void OCRResultWidget::setThemeStyle()
{
    QVariant theme = kdk::kabase::Gsettings::getSystemTheme();
    QPalette pal = this->palette();
    QColor backgroundColor;
    QColor textColor;
    if (theme.toString() == "lingmo-dark" || theme.toString() == "lingmo-black") {
        backgroundColor = QColor(15, 15, 15, 165);
        textColor = QColor(217, 217, 217, 255);
    } else {
        backgroundColor = QColor(217, 217, 217, 165);
        textColor = QColor(38, 38, 38, 255);
    }
    pal.setBrush(QPalette::Base, backgroundColor);
    pal.setBrush(QPalette::Text, textColor);
    this->setPalette(pal);
}
