/* Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
*               2020 KylinSoft Co., Ltd.
* This file is part of Lingmo-Screenshot.
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
*/

#include "texttool.h"
#include "textwidget.h"
#include "textconfig.h"
#include <QDebug>
#define BASE_POINT_SIZE 8

TextTool::TextTool(QObject *parent) : CaptureTool(parent),
    m_size(1)
{
}

bool TextTool::isValid() const
{
    return !m_text.isEmpty();
}

bool TextTool::closeOnButtonPressed() const
{
    return false;
}

bool TextTool::isSelectable() const
{
    return true;
}

bool TextTool::showMousePreview() const
{
    return false;
}

QIcon TextTool::icon(const QColor &background, bool inEditor) const
{
    // Q_UNUSED(inEditor);
    // return QIcon(iconPath(background) + "text.svg");
    Q_UNUSED(background);
    return inEditor ? QIcon(QStringLiteral(":/img/material/black/") + "text.svg")
           : QIcon(QStringLiteral(":/img/material/white/") + "format-text.svg");
}

#ifdef SUPPORT_LINGMO
QIcon TextTool::icon(const QColor &background, bool inEditor, const CaptureContext &context) const
{
    Q_UNUSED(background);
    if ((context.style_name.compare("lingmo-dark") == 0)
        || (context.style_name.compare("lingmo-black") == 0)) {
        return inEditor ? QIcon(QStringLiteral(":/img/material/black/") + "text.svg")
               : QIcon(QStringLiteral(":/img/material/dark-theme/") + "format-text.png");
    }
    // if((context.style_name.compare("lingmo-white")==0) || (context.style_name.compare("lingmo-default")==0) || (context.style_name.compare("lingmo-light")==0)){
    else {
        return inEditor ? QIcon(QStringLiteral(":/img/material/black/") + "text.svg")
               : QIcon(QStringLiteral(":/img/material/white/") + "format-text.svg");
    }
}

#endif
QString TextTool::name() const
{
    return tr("text");
}

QString TextTool::nameID()
{
    return QLatin1String("");
}

QString TextTool::description() const
{
    return tr("Add text to your capture");
}

QWidget *TextTool::widget()
{
    TextWidget *w = new TextWidget();
    w->setTextColor(m_color);
    m_font.setPointSize(m_size + BASE_POINT_SIZE);
    w->setFont(m_font);
    connect(w, &TextWidget::textUpdated,
            this, &TextTool::updateText);
    w->setMaxSize(rect.x()+rect.width()-startPos.x()-10,
                  rect.y()+rect.height()-startPos.y());

    m_widget = w;
    return w;
}

QWidget *TextTool::configurationWidget()
{
    m_confW = new TextConfig();
    connect(m_confW, &TextConfig::fontFamilyChanged,
            this, &TextTool::updateFamily);
    connect(m_confW, &TextConfig::fontItalicChanged,
            this, &TextTool::updateFontItalic);
    connect(m_confW, &TextConfig::fontStrikeOutChanged,
            this, &TextTool::updateFontStrikeOut);
    connect(m_confW, &TextConfig::fontUnderlineChanged,
            this, &TextTool::updateFontUnderline);
    connect(m_confW, &TextConfig::fontWeightChanged,
            this, &TextTool::updateFontWeight);
    m_confW->setItalic(m_font.italic());
    m_confW->setUnderline(m_font.underline());
    m_confW->setStrikeOut(m_font.strikeOut());
    m_confW->setWeight(m_font.weight());
    return m_confW;
}

CaptureTool *TextTool::copy(QObject *parent)
{
    TextTool *tt = new TextTool(parent);
    connect(m_confW, &TextConfig::fontFamilyChanged,
            tt, &TextTool::updateFamily);
    connect(m_confW, &TextConfig::fontItalicChanged,
            tt, &TextTool::updateFontItalic);
    connect(m_confW, &TextConfig::fontStrikeOutChanged,
            tt, &TextTool::updateFontStrikeOut);
    connect(m_confW, &TextConfig::fontUnderlineChanged,
            tt, &TextTool::updateFontUnderline);
    connect(m_confW, &TextConfig::fontWeightChanged,
            tt, &TextTool::updateFontWeight);
    tt->m_font = m_font;
    return tt;
}

void TextTool::undo(QPixmap &pixmap)
{
    QPainter p(&pixmap);
    p.drawPixmap(m_backupArea.topLeft(), m_pixmapBackup);
}

void TextTool::process(QPainter &painter, const QPixmap &pixmap, bool recordUndo)
{
    if (m_text.isEmpty()) {
        return;
    }
    QFontMetrics fm(m_font);
    QSize size(fm.boundingRect(QRect(), 0, m_text).size());
    qDebug() << "size" << size;
    m_backupArea.setSize(size);
    // 分割字符
    splitString(m_text, size, rect.x()+rect.width()-m_backupArea.topLeft().x()-size.width());
    // 撤销操作  更新编辑区域
    if (recordUndo) {
        m_pixmapBackup = pixmap.copy(QRect(m_backupArea.x()*devicePixelRatio,
                                           m_backupArea.y()*devicePixelRatio,
                                           m_backupArea.width()*devicePixelRatio,
                                           (m_backupArea.height()+m_listString.length()
                                            *size.height())*devicePixelRatio)
                                     + QMargins(0, 0, 5, 5));
    }
    painter.setFont(m_font);
    painter.setPen(m_color);
    for (int i = 0; i < m_listString.length(); i++) {
        // 绘制文字
        if (m_backupArea.y()+ (i+1)*size.height() <= rect.y()+rect.height()) {
            painter.drawText(QRect(m_backupArea.x(),
                                   m_backupArea.y()+ i*size.height(),
                                   m_backupArea.width(),
                                   m_backupArea.height())
                             + QMargins(-5, -5, 5, 5),
                             m_listString.at(i));
        }
    }
    if (m_widget) {
        m_widget->setFont(m_font);
    }
}

// 分割字符  超出区域的部分 重新储存
// 参数 text：输入的字符串   size: 输入字符串的size信息  n:文字末端距离框选区域的距离
void TextTool::splitString(QString text, QSize size, int n)
{
    // 输入字体未超出区域
    if (n > 0) {
        m_listString.append(text);
    } else {
        // 获取文字起始位置距离框选区域右侧的内容
        int widt = rect.x()+rect.width()-startPos.x()-15;
        // 可以容纳的行数 需要几次换行
        int num = size.width()/widt;
        // 每个字符所占的长度比
        int ii = size.width()/text.length();
        // 每行可容纳几个字符
        int mm = widt/ii;
        // 将每行可容纳的字符依次存储在m_listString中
        for (int i = 0; i < num; i++) {
            m_listString.append(text.mid(i*widt/ii, mm));
        }
        // 最后未能整除 剩余的字符也存储起来
        if (!(0 == size.width()%widt))
            m_listString.append(text.mid(num*widt/ii, size.width()%widt/ii+1));
    }
}

void TextTool::paintMousePreview(QPainter &painter, const CaptureContext &context)
{
    Q_UNUSED(painter);
    m_font = context.font_type;
    if (m_widget) {
        m_widget->setFont(context.font_type);
    }
    m_font.setFamily(context.font_type.family());
    m_font.setBold(context.bold);
    m_font.setItalic(context.italic);
    m_font.setUnderline(context.underline);
    m_font.setStrikeOut(context.deleteline);
}

void TextTool::drawEnd(const QPoint &p)
{
    m_backupArea.moveTo(p);
}

void TextTool::drawMove(const QPoint &p)
{
    m_widget->move(p);
}

void TextTool::drawStart(const CaptureContext &context)
{
    rect = context.selection;
    startPos = context.mousePos;
    m_color = context.color;
    m_size = context.text_thickness;
    m_font.setFamily(context.font_type.family());
    m_font.setBold(context.bold);
    m_font.setItalic(context.italic);
    m_font.setUnderline(context.underline);
    m_font.setStrikeOut(context.deleteline);
    devicePixelRatio = context.screenshot.devicePixelRatio();
    emit requestAction(REQ_ADD_CHILD_WIDGET);
}

void TextTool::pressed(const CaptureContext &context)
{
    rect = context.selection;
    m_color = context.color;
    m_size = context.thickness;
    m_font.setFamily(context.font_type.family());
    m_font.setBold(context.bold);
    m_font.setItalic(context.italic);
    m_font.setUnderline(context.underline);
    m_font.setStrikeOut(context.deleteline);
    devicePixelRatio = context.screenshot.devicePixelRatio();
}

void TextTool::colorChanged(const QColor &c)
{
    m_color = c;
    if (m_widget) {
        m_widget->setTextColor(c);
    }
}

void TextTool::thicknessChanged(const int th)
{
// m_size = th;
// m_font.setPointSize(m_size + BASE_POINT_SIZE);
// if (m_widget) {
// m_widget->setFont(m_font);
// }
    Q_UNUSED(th);
}

void TextTool::textthicknessChanged(const int th)
{
    m_size = th;
    m_font.setPointSize(m_size + BASE_POINT_SIZE);
    if (m_widget) {
        m_widget->setFont(m_font);
    }
}

void TextTool::textChanged(const CaptureContext &context)
{
    m_font.setUnderline(context.underline);
    m_font.setStrikeOut(context.deleteline);
    m_font.setItalic(context.italic);
    m_font.setBold(context.bold);
    m_font.setFamily(context.font_type.family());
    if (m_widget) {
        m_widget->setFont(m_font);
    }
}

void TextTool::updateText(const QString &s)
{
    m_text = s;
}

void TextTool::setFont(const QFont &f)
{
    m_font = f;
    if (m_widget) {
        m_widget->setFont(f);
    }
}

void TextTool::updateFamily(const QString &s)
{
    m_font.setFamily(s);
    if (m_widget) {
        m_widget->setFont(m_font);
    }
}

void TextTool::updateFontUnderline(const bool underlined)
{
    m_font.setUnderline(underlined);
    if (m_widget) {
        m_widget->setFont(m_font);
    }
}

void TextTool::updateFontStrikeOut(const bool s)
{
    m_font.setStrikeOut(s);
    if (m_widget) {
        m_widget->setFont(m_font);
    }
}

void TextTool::updateFontWeight(const QFont::Weight w)
{
    m_font.setWeight(w);
    if (m_widget) {
        m_widget->setFont(m_font);
    }
}

void TextTool::updateFontItalic(const bool italic)
{
    m_font.setItalic(italic);
    if (m_widget) {
        m_widget->setFont(m_font);
    }
}
