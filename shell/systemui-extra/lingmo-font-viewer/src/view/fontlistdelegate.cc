#include <QFont>
#include <QPixmap>
#include <QRectF>
#include <QRect>
#include <QDebug>
#include <QApplication>
#include <QPainterPath>

#include "fontlistdelegate.h"
#include "globalsizedata.h"
#include "mainview.h"

QPoint FontListDelegate::m_collectPoint = QPoint(0, 0);
QSize FontListDelegate::m_collectSize = QSize(0, 0);

const QString COLLECT_ICON_W = QString(":/data/image/lingmo-play-love-symbolic-w.svg");
const QString COLLECT_ICON_B = QString(":/data/image/lingmo-play-love-symbolic-b.svg");
const QString COLLECT_ICON_R = QString(":/data/image/lingmo-play-love-red.svg");

FontListDelegate::FontListDelegate(FontListView *fontList)
{
    m_fontList = fontList;
    m_fontSize = m_fontList->getFontSize();
}

FontListDelegate::~FontListDelegate() {}

QSize FontListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);

    /* 获取默认字体，在当前字号的高度 */
    QFont familyFont = QFont();
    QFontMetrics fontm = QFontMetrics(familyFont);
    int fontHeight = fontm.lineSpacing();
    fontHeight = 24 > fontHeight ? 24 : fontHeight;
    familyFont.setPointSizeF(m_fontSize);
    QFontMetrics fontmet = QFontMetrics(familyFont);
    int maxHigh = fontmet.lineSpacing();

    int fontHigh = index.data(GlobalSizeData::FontHeight).toInt();
    if (maxHigh < fontHigh) {
        maxHigh = fontHigh;
    }

    maxHigh = maxHigh + fontHeight + 40; /* maxHigh:预览信息高度 fontHeight:字体信息高度 30:间距 */

    return QSize(588, maxHigh);
}

void FontListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.isValid()) {
        painter->save();

        // 反锯齿
        painter->setRenderHint(QPainter::Antialiasing);

        // 获取数据
        QString fontName = index.data(GlobalSizeData::FontName).toString();
        QString fontStyle = index.data(GlobalSizeData::FontStyle).toString();
        int fontCollect = index.data(GlobalSizeData::CollectState).toInt();
        bool fontInstall = index.data(GlobalSizeData::FontInstall).toBool();

        QString fontInformation = fontName + " " + fontStyle;
        QString fontDisplay = m_fontList->getPreviewValue();

        // 整体item 矩形区域
        QRectF itemRect;
        itemRect.setX(option.rect.x());
        itemRect.setY(option.rect.y());
        itemRect.setWidth(option.rect.width() - 1);
        itemRect.setHeight(option.rect.height() - 1);

        painter->save();

        //画笔设置透明（边框）
        painter->setPen(Qt::transparent);
        // QPainterPath画圆角矩形
//        const qreal radius = 7;
        const qreal radius = MainView::getInstance()->style()->property("normalRadius").toInt();

        QPainterPath path;
        path.moveTo(itemRect.topRight() - QPointF(radius, 0));
        path.lineTo(itemRect.topLeft() + QPointF(radius, 0));
        path.quadTo(itemRect.topLeft(), itemRect.topLeft() + QPointF(0, radius));
        path.lineTo(itemRect.bottomLeft() + QPointF(0, -radius));
        path.quadTo(itemRect.bottomLeft(), itemRect.bottomLeft() + QPointF(radius, 0));
        path.lineTo(itemRect.bottomRight() - QPointF(radius, 0));
        path.quadTo(itemRect.bottomRight(), itemRect.bottomRight() + QPointF(0, -radius));
        path.lineTo(itemRect.topRight() + QPointF(0, radius));
        path.quadTo(itemRect.topRight(), itemRect.topRight() + QPointF(-radius, 0));
        painter->drawPath(path);

        painter->restore();

        //绘制文字
        QTextOption optionText;
        QFont infoFont = painter->font();
        infoFont.setPointSizeF(14);
        QFontMetrics fontmet = QFontMetrics(infoFont);
        int high = fontmet.lineSpacing();

        // 字体信息
        painter->save();
        QPoint infomationPoint(itemRect.left() + 16, itemRect.top() + 8);
        high = 24 > high ? 24 : high;
        QSize infomationSize(option.rect.width() * 0.6, high);
        QRectF infomationRect = QRect(infomationPoint, infomationSize);

        optionText.setAlignment(Qt::AlignLeft | Qt::AlignBottom); // 文字消息内容绘制
        optionText.setWrapMode(QTextOption::WrapAnywhere);

        // 防止应用字体出现截断
        QFontMetrics fontmtsFontInfo = QFontMetrics(infoFont);
        if (fontmtsFontInfo.width(fontInformation) > infomationRect.width()) {
            fontInformation = fontmtsFontInfo.elidedText(fontInformation, Qt::ElideRight, infomationRect.width());
        }

        painter->setFont(infoFont);
        painter->drawText(infomationRect, fontInformation, optionText);
        painter->restore();

        // 展示字体
        painter->save();
        optionText.setAlignment(Qt::AlignLeft | Qt::AlignTop);

        bool isFontItalic = false;
        QFont textFont = QFont();
        textFont.setFamily(fontName);
        textFont.setPointSizeF(m_fontSize);
        QStringList style = fontStyle.split(' ');
        for (int i = 0; i < style.size(); i++) {
            if (style.at(i) == "Bold") {
                textFont.setBold(true);
            } else if (style.at(i) == "Italic") {
                textFont.setItalic(true);
                isFontItalic = true;
            } else if (style.at(i) == "Oblique") {
                textFont.setStyle(QFont::StyleOblique);
            }
        }

        infoFont.setPointSizeF(m_fontSize);
        fontmet = QFontMetrics(infoFont);
        high = fontmet.lineSpacing();

        QPoint displayPoint(itemRect.left() + 16, infomationRect.bottom() + 10);                      /* 确定点 */
        QSize displaySize = getTextRectSize(fontDisplay, textFont, option.rect.width() * 0.85, high); /* 确定大小 */
        QRectF displayRect = QRect(displayPoint, displaySize);                                        /* 绘制矩形 */

        QFontMetrics fontmts = QFontMetrics(textFont);

        int disPalayWidth = displaySize.width();
        if (isFontItalic) {
            if (displaySize.width() > fontmts.width(fontDisplay)) {
                if ((displaySize.width() - fontmts.width(fontDisplay)) < 10) {
                    fontDisplay = fontmts.elidedText(fontDisplay, Qt::ElideRight, displayRect.width() - 10);
                }
            }
        }
        if (fontmts.width(fontDisplay) > disPalayWidth) {
            fontDisplay = fontmts.elidedText(fontDisplay, Qt::ElideRight, displayRect.width());
        }
        painter->setFont(textFont);
        painter->drawText(displayRect, fontDisplay, optionText);
        painter->restore();

        painter->setRenderHints(QPainter::Antialiasing|QPainter::SmoothPixmapTransform);

        // 收藏图标
        QPoint collectPoint = QPoint(itemRect.right() - 35, itemRect.top() + 15);
        QSize collectSize = QSize(18, 16);
        QPixmap collectePixmap;
        QRect collecteRect = QRect(collectPoint, collectSize);
        QRectF collecteRectF = collecteRect;

        if (fontCollect == GlobalSizeData::CollectType::NotCollected) {
            /* 深浅主题控件样式 */
            if (GlobalSizeData::THEME_COLOR == GlobalSizeData::LINGMOLight) {
                collectePixmap =
                    QPixmap(QIcon(COLLECT_ICON_B).pixmap(collecteRect.size()));
            } else {
                collectePixmap =
                    QPixmap(QIcon(COLLECT_ICON_W).pixmap(collecteRect.size()));
            }

        } else {
            collectePixmap = QPixmap(QIcon(COLLECT_ICON_R).pixmap(collecteRect.size()));
        }

        painter->drawPixmap(collecteRect, collectePixmap);
        QColor highLightColor = QApplication::palette().highlight().color();
        QColor lightHoverColor = QColor("#EBEBEB");
        QColor darkHoverColor = QColor("#333333");

        // 鼠标悬停/选中，改变背景颜色
        if (option.state.testFlag(QStyle::State_HasFocus) || fontInstall) {
            FontListDelegate::m_collectPoint = collecteRectF.topLeft().toPoint();
            FontListDelegate::m_collectSize = collecteRectF.size().toSize();
            painter->fillPath(path, QBrush(highLightColor));
            painter->setPen(QPen(Qt::white));

            painter->setFont(QFont(painter->fontInfo().family(), 14));
            optionText.setAlignment(Qt::AlignLeft | Qt::AlignBottom);
            painter->drawText(infomationRect, fontInformation, optionText);

            painter->setFont(textFont);
            optionText.setAlignment(Qt::AlignLeft | Qt::AlignTop);
            painter->drawText(displayRect, fontDisplay, optionText);

            if (fontCollect == GlobalSizeData::CollectType::NotCollected) {
                collectePixmap =
                    QPixmap(QIcon(COLLECT_ICON_W).pixmap(collecteRect.size()));
            } else {
                collectePixmap = QPixmap(QIcon(COLLECT_ICON_R).pixmap(collecteRect.size()));
            }
            painter->drawPixmap(collecteRect, collectePixmap);

            path.setFillRule(Qt::WindingFill);
            path.addRoundedRect(itemRect, 0, 0);
        } else if (option.state.testFlag(QStyle::State_MouseOver)) {
            FontListDelegate::m_collectPoint = collecteRectF.topLeft().toPoint();
            FontListDelegate::m_collectSize = collecteRectF.size().toSize();

            if (GlobalSizeData::getInstance()->THEME_COLOR == GlobalSizeData::ThemeColor::LINGMOLight) {
                painter->fillPath(path, QBrush(lightHoverColor));
                painter->setPen(QPen(Qt::black));
                if (fontCollect == GlobalSizeData::CollectType::NotCollected) {
                    collectePixmap = QPixmap(QIcon(COLLECT_ICON_B).pixmap(collecteRect.size()));
                } else {
                    collectePixmap = QPixmap(QIcon(COLLECT_ICON_R).pixmap(collecteRect.size()));
                }
            } else {
                painter->fillPath(path, QBrush(darkHoverColor));
                painter->setPen(QPen(Qt::white));
                if (fontCollect == GlobalSizeData::CollectType::NotCollected) {
                    collectePixmap = QPixmap(QIcon(COLLECT_ICON_W).pixmap(collecteRect.size()));
                } else {
                    collectePixmap = QPixmap(QIcon(COLLECT_ICON_R).pixmap(collecteRect.size()));
                }
            }
            painter->drawPixmap(collecteRect, collectePixmap);

            painter->setFont(QFont(painter->fontInfo().family(), 14));
            optionText.setAlignment(Qt::AlignLeft | Qt::AlignBottom);
            painter->drawText(infomationRect, fontInformation, optionText);

            painter->setFont(textFont);
            optionText.setAlignment(Qt::AlignLeft | Qt::AlignTop);
            painter->drawText(displayRect, fontDisplay, optionText);

            path.setFillRule(Qt::WindingFill);
            path.addRoundedRect(itemRect, 0, 0);

        }
        painter->restore();
    }
    return;
}


QSize FontListDelegate::getTextRectSize(QString text, QFont font, int textWidth, int familyFontH)
{
    QSize resSize = QSize();
    QFontMetrics fontmts = QFontMetrics(font);
    int fontHeight = fontmts.lineSpacing(); /* 展示字体的高度 */
    int fontWidth = fontmts.width(text);    /* 展示信息的宽度 */

    fontHeight = fontHeight > familyFontH ? fontHeight : familyFontH;
    fontWidth = textWidth;

    resSize.setHeight(fontHeight + 10);
    resSize.setWidth(fontWidth + 10);
    return resSize;
}
