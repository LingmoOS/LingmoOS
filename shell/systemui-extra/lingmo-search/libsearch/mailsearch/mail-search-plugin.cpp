/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 *
 */
#include "mail-search-plugin.h"
#include <QDebug>
#include "file-utils.h"
#include "chinese-segmentation.h"
using namespace LingmoUISearch;

#define ICON_SIZE QSize(120,120)
#define DETAIL_WIDTH 360
#define DETAIL_MARGINS 8, 0, 16, 0
#define DETAIL_ICON_HEIGHT 128
#define NAME_LABEL_MARGINS 8, 0, 0, 0
#define NAME_LABEL_WIDTH 282
#define DETAIL_ACTION_MARGINS 8, 0, 0, 0
#define DESC_PAGE_WIDTH 316
#define DESC_LABEL_MARGINS 0, 0, 0, 0
#define DESC_LABEL_SIZE QSize(87, 34)
#define DESC_LABEL_SPACING 16
#define SCROLL_BAR_COLOR QColor(0, 0, 0, 0)
#define SCROLL_AREA_MARGINS 8, 0, 0, 0
#define SCROLL_AREA_MAX_HEIGHT 212
#define SCROLL_AREA_MAX_WIDTH 344
#define AREA_CONTENT_MARGINS 8, 0, 0, 0

size_t MailSearchPlugin::uniqueSymbol = 0;
QMutex MailSearchPlugin::m_mutex;

MailSearchPlugin::MailSearchPlugin(QObject *parent) : QObject(parent)
{
    SearchPluginIface::Actioninfo open { 0, tr("open")};
    m_actionInfo << open;
    m_pool.setMaxThreadCount(1);
    m_pool.setExpiryTimeout(1000);
    initDetailPage();
}

const QString MailSearchPlugin::name()
{
    return tr("Mail Search");
}

const QString MailSearchPlugin::description()
{
    return tr("Mail Search");
}

QString MailSearchPlugin::getPluginName()
{
    return tr("Mail Search");
}

void MailSearchPlugin::KeywordSearch(QString keyword, DataQueue<SearchPluginIface::ResultInfo> *searchResult)
{
    m_mutex.lock();
    ++uniqueSymbol;
    m_mutex.unlock();
    m_keyword = keyword;
    MailSearch *mailSearch = new MailSearch(searchResult, keyword, uniqueSymbol);
    m_pool.start(mailSearch);
}

void MailSearchPlugin::stopSearch()
{
    m_mutex.lock();
    ++uniqueSymbol;
    m_mutex.unlock();
}

QList<SearchPluginIface::Actioninfo> MailSearchPlugin::getActioninfo(int type)
{
    return m_actionInfo;
}

void MailSearchPlugin::openAction(int actionkey, QString key, int type)
{
    QProcess process;
    switch(actionkey) {
    case 0:
        process.startDetached(QString("evolution"));
        break;
    default:
        break;
    }
}

QWidget *MailSearchPlugin::detailPage(const SearchPluginIface::ResultInfo &ri)
{
    m_currentActionKey = ri.actionKey;
    m_iconLabel->setPixmap(ri.icon.pixmap(ICON_SIZE));
    QFontMetrics fontMetrics = m_nameLabel->fontMetrics();
    QString showname = fontMetrics.elidedText(ri.name, Qt::ElideRight, 215);//当字体长度超过215时显示为省略号
    m_nameLabel->setText(QString("<h3 style=\"font-weight:normal;\">%1</h3>").arg(FileUtils::escapeHtml(showname)));
    if (QString::compare(showname, ri.name)) {
        m_nameLabel->setToolTip(ri.name);
    }
    m_pluginLabel->setText(tr("Mail"));

    m_senderLabel->setText(ri.description.at(0).key);
    QString senderName = fontMetrics.elidedText(ri.description.at(0).value, Qt::ElideRight, 246);//当字体长度超过246时显示为省略号
    m_senderFieldsLabel->setText(FileUtils::escapeHtml(senderName));
    if (QString::compare(senderName, ri.description.at(0).value)) {
        m_senderFieldsLabel->setToolTip(ri.description.at(0).value);
    } else {
        m_nameLabel->setToolTip("");
    }

    m_timeLabel->setText(ri.description.at(1).key);
    m_timeFieldsLabel->setText(ri.description.at(1).value);

    m_recipientsLabel->setText(ri.description.at(2).key);
    QString recipientsStr = ri.description.at(2).value;
//    QString recipientsStr = fontMetrics.elidedText(ri.description.at(2).value, Qt::ElideRight, 200);
//    recipientsStr.replace(QRegExp("\\, "), "\n");
    m_recipientsFieldsLabel->setText(FileUtils::getHtmlText(recipientsStr, m_keyword));
    resizeTextEdit(m_recipientsFieldsLabel);


    m_ccLabel->setText(ri.description.at(3).key);
    QString ccStr = ri.description.at(3).value;
//    QString ccStr = fontMetrics.elidedText(ri.description.at(3).value, Qt::ElideRight, 200);
    m_ccFieldsLabel->setText(FileUtils::getHtmlText(ccStr, m_keyword));
    resizeTextEdit(m_ccFieldsLabel);
//    if (QString::compare(ccStr, ri.description.at(3).value)) {
//        m_ccFieldsLabel->setToolTip(ri.description.at(3).value);
//    }
    //重置滚动条位置
    m_descListArea->verticalScrollBar()->setValue(0);
    return m_detailPage;
}

void MailSearchPlugin::initDetailPage()
{
    m_detailPage = new QWidget();
    m_detailPage->setFixedWidth(DETAIL_WIDTH);
    m_detailPage->setAttribute(Qt::WA_TranslucentBackground);
    m_detailLyt = new QVBoxLayout(m_detailPage);
    m_detailLyt->setContentsMargins(DETAIL_MARGINS);

    m_iconLabel = new QLabel(m_detailPage);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setFixedHeight(DETAIL_ICON_HEIGHT);

    m_nameFrame = new QFrame(m_detailPage);
    m_nameFrameLyt = new QHBoxLayout(m_nameFrame);
    m_nameFrame->setLayout(m_nameFrameLyt);
    m_nameFrameLyt->setContentsMargins(NAME_LABEL_MARGINS);
    m_nameLabel = new QLabel(m_nameFrame);
    m_nameLabel->setMaximumWidth(NAME_LABEL_WIDTH);
    m_pluginLabel = new QLabel(m_nameFrame);
    m_pluginLabel->setEnabled(false);
    m_nameFrameLyt->addWidget(m_nameLabel);
    m_nameFrameLyt->addStretch();
    m_nameFrameLyt->addWidget(m_pluginLabel);

    m_line_1 = new SeparationLine(m_detailPage);

    m_descListArea = new MailDescArea(m_detailPage);
    QPalette scroll_bar_pal = m_descListArea->verticalScrollBar()->palette();
    scroll_bar_pal.setColor(QPalette::Base, SCROLL_BAR_COLOR);
    m_descListArea->verticalScrollBar()->setPalette(scroll_bar_pal);

    m_descPage = new QWidget(m_descListArea);
    m_descListArea->setWidget(m_descPage);

    m_descPageLyt = new QFormLayout(m_descPage);
    m_descPage->setLayout(m_descPageLyt);
    m_descPage->setContentsMargins(DESC_LABEL_MARGINS);
    m_descPage->setFixedWidth(DESC_PAGE_WIDTH);

    m_senderLabel = new QLabel(m_descPage);
    m_senderFieldsLabel = new QLabel(m_descPage);
    m_senderLabel->setMaximumSize(DESC_LABEL_SIZE);
    m_senderFieldsLabel->setAlignment(Qt::AlignRight);

    m_timeLabel = new QLabel(m_descPage);
    m_timeFieldsLabel = new QLabel(m_descPage);
    m_timeLabel->setMaximumSize(DESC_LABEL_SIZE);
    m_timeFieldsLabel->setAlignment(Qt::AlignRight);

    m_recipientsLabel = new QLabel(m_descPage);
//    m_recipientsFieldsLabel = new QLabel(m_descPage);
    m_recipientsFieldsLabel = new QTextBrowser(m_descPage);
    m_recipientsLabel->setMaximumSize(DESC_LABEL_SIZE);
//    m_recipientsLabel->setMaximumHeight(35);
//    m_recipientsFieldsLabel->setAlignment(Qt::AlignRight);
//    m_recipientsFieldsLabel->setFixedWidth(260);
//    m_recipientsFieldsLabel->setWordWrap(true);

    m_recipientsFieldsLabel->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_recipientsFieldsLabel->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_recipientsFieldsLabel->setWordWrapMode(QTextOption::WrapAnywhere);
//    m_recipientsFieldsLabel->setLineWrapMode(QTextEdit::FixedPixelWidth);
//    m_recipientsFieldsLabel->setLineWrapColumnOrWidth(260);
    m_recipientsFieldsLabel->setPalette(scroll_bar_pal);
    m_recipientsFieldsLabel->setFrameShape(QFrame::Shape::NoFrame);

    m_ccLabel = new QLabel(m_descPage);
//    m_ccFieldsLabel = new QLabel(m_descPage);
    m_ccFieldsLabel = new QTextBrowser(m_descPage);
    m_ccLabel->setMaximumSize(DESC_LABEL_SIZE);
//    m_ccFieldsLabel->setAlignment(Qt::AlignRight);
//    m_ccFieldsLabel->setFixedWidth(262);
//    m_ccFieldsLabel->setWordWrap(true);

    m_ccFieldsLabel->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_ccFieldsLabel->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_ccFieldsLabel->setWordWrapMode(QTextOption::WrapAnywhere);
    m_ccFieldsLabel->setPalette(scroll_bar_pal);
    m_ccFieldsLabel->setFrameShape(QFrame::Shape::NoFrame);


    /*
    m_attachmentLabel = new QLabel(m_descPage);
    m_attachmentFieldsLabel = new QLabel(m_descPage);
    m_attachmentLabel->setMaximumSize(DESC_LABEL_SIZE);
    m_attachmentLabel->setText(QString(tr("Attachment")));
    m_attachmentFieldsLabel->setAlignment(Qt::AlignRight);
    */

    m_descPageLyt->setContentsMargins(DESC_LABEL_MARGINS);
    m_descPageLyt->setVerticalSpacing(DESC_LABEL_SPACING);
    m_descPageLyt->addRow(m_senderLabel, m_senderFieldsLabel);
    m_descPageLyt->addRow(m_timeLabel, m_timeFieldsLabel);
    m_descPageLyt->addRow(m_recipientsLabel, m_recipientsFieldsLabel);
    m_descPageLyt->addRow(m_ccLabel, m_ccFieldsLabel);
//    m_descPageLyt->addRow(m_attachmentLabel, m_attachmentFieldsLabel);

    m_line_2 = new SeparationLine(m_detailPage);

    m_actionFrame = new QFrame(m_detailPage);
    m_actionFrameLyt = new QVBoxLayout(m_actionFrame);
    m_actionFrameLyt->setContentsMargins(DETAIL_ACTION_MARGINS);
    m_actionLabel1 = new ActionLabel(tr("Open"), m_currentActionKey, m_actionFrame);
    m_actionFrameLyt->addWidget(m_actionLabel1);
    m_actionFrame->setLayout(m_actionFrameLyt);

    m_detailLyt->addSpacing(50);
    m_detailLyt->addWidget(m_iconLabel);
    m_detailLyt->addWidget(m_nameFrame);
    m_detailLyt->addWidget(m_line_1);
    m_detailLyt->addWidget(m_descListArea);
    m_detailLyt->addWidget(m_line_2);
    m_detailLyt->addWidget(m_actionFrame);
    m_detailLyt->addStretch();
    m_detailPage->setLayout(m_detailLyt);

    connect(m_actionLabel1, &ActionLabel::actionTriggered, [ & ](){
        openAction(m_actionInfo.at(0).actionkey, m_currentActionKey,0);
    });
    //    connect(m_recipientsFieldsLabel->document(), &QTextDocument::contentsChanged, this, &MailSearchPlugin::resetHeight);
}

void MailSearchPlugin::resizeTextEdit(QTextEdit *textEdit)
{
    QTextDocument *doc = textEdit->document();
    int height = doc->size().height();
    textEdit->setMinimumHeight(32);
    int minheight = textEdit->minimumHeight();
        if (height < minheight)
        {
            height = minheight;
        }
    textEdit->setFixedHeight(height);
}

void MailSearchPlugin::resetHeight()
{
    QTextDocument *document=qobject_cast<QTextDocument*>(sender());
    if (!document) {
        return;
    }
    QTextEdit *editor=qobject_cast<QTextEdit*>(document->parent()->parent());
    if (!editor) {
        return;
    }

    int newheight = document->size().rheight();
    if (newheight != editor->height()) {
        editor->setFixedHeight(newheight);
    } else {
        return;
    }
//    m_recipientsFieldsLabel->setFixedHeight(m_recipientsFieldsLabel->document()->size().rheight());
}

MailSearch::MailSearch(DataQueue<SearchPluginIface::ResultInfo> *searchResult, const QString &keyword, size_t uniqueSymbol)
{
    this->setAutoDelete(true);
    this->m_searchResult = searchResult;
    this->m_keyword = keyword;
    this->m_uniqueSymbol = uniqueSymbol;
}

void MailSearch::run()
{
    QDBusInterface mailDataInterface("org.gnome.EvolutionMailData", "/", "org.gnome.EvolutionMailData", QDBusConnection::sessionBus());
    QDBusReply<QVariantMap> reply = mailDataInterface.call("keywordMatch",m_keyword);
    if (reply.isValid()) {
        if (m_uniqueSymbol == MailSearchPlugin::uniqueSymbol) {
            for (std::pair<QString, QVariant> result : reply.value().toStdMap()) {
                const QDBusArgument &dbusArgs = result.second.value<QDBusArgument>();
                QStringList resultList;
                dbusArgs.beginArray();
                while (!dbusArgs.atEnd()) {
                    QVariant tmp;
                    dbusArgs >> tmp;
                    resultList.append(tmp.toString());
                }
                dbusArgs.endArray();
                SearchPluginIface::ResultInfo ri;
                createResultInfo(ri, resultList);
                if (m_uniqueSymbol == MailSearchPlugin::uniqueSymbol) {
                    m_searchResult->enqueue(ri);
                } else {
                    qDebug() << "uniqueSymbol don't match:" << m_uniqueSymbol << MailSearchPlugin::uniqueSymbol;
                    return;
                }
            }
        } else {
            qDebug() << "uniqueSymbol don't match:" << m_uniqueSymbol << MailSearchPlugin::uniqueSymbol;
            return;
        }
    } else {
        qWarning() << "Error!Mail dbus call failed:" << mailDataInterface.lastError();
        return;
    }
}

void MailSearch::createResultInfo(SearchPluginIface::ResultInfo &ri, QStringList &resultList)
{
    QString recipientsStr = resultList.at(5);
    recipientsStr.replace(QRegExp("\\, "), "\n");
//    recipientsStr.toUtf8().data();
    QString ccStr = resultList.at(6);
    ccStr.replace(QRegExp("\\, "), "\n");
    ri.icon = XdgIcon::fromTheme("evolution");
    ri.name = resultList.at(3);
    ri.description = QVector<SearchPluginIface::DescriptionInfo>() \
                    << SearchPluginIface::DescriptionInfo{tr("From"), resultList.at(4)} \
                    << SearchPluginIface::DescriptionInfo{tr("Time"), resultList.at(7)} \
                    << SearchPluginIface::DescriptionInfo{tr("To"), recipientsStr/*resultList.at(5)*/} \
                    << SearchPluginIface::DescriptionInfo{tr("Cc"), ccStr/*resultList.at(6)*/};
    ri.type = 0;
}

MailDescArea::MailDescArea(QWidget *parent) : QScrollArea(parent)
{
    initUi();
}

void MailDescArea::initUi()
{
    this->setFrameShape(QFrame::Shape::NoFrame);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    this->setWidgetResizable(true);
    this->setFixedWidth(SCROLL_AREA_MAX_WIDTH);
    this->setMaximumHeight(SCROLL_AREA_MAX_HEIGHT);
    this->setViewportMargins(AREA_CONTENT_MARGINS);
//    this->setContentsMargins(SCROLL_AREA_MARGINS);
    /*
    m_descPage = new QWidget(this);
    this->setWidget(m_descPage);
    m_descPageLyt = new QFormLayout(m_descPage);
    m_descPage->setLayout(m_descPageLyt);
    m_descPage->setContentsMargins(8, 0, 8, 0);

    m_senderLabel = new QLabel(m_descPage);
    m_senderFieldsLabel = new QLabel(m_descPage);
    m_senderLabel->setMaximumSize(DESC_LABEL_SIZE);
    m_senderLabel->setText(QString(tr("From:")));
    m_senderFieldsLabel->setText("<jianbingguozi@kylinos.cn>");
    m_senderFieldsLabel->setAlignment(Qt::AlignRight);
    m_senderFieldsLabel->setStyleSheet(LINE_STYLE);

    m_timeLabel = new QLabel(m_descPage);
    m_timeFieldsLabel = new QLabel(m_descPage);
    m_timeLabel->setMaximumSize(DESC_LABEL_SIZE);
    m_timeLabel->setText(QString(tr("Time:")));
    m_timeFieldsLabel->setAlignment(Qt::AlignRight);

    m_recipientsLabel = new QLabel(m_descPage);
    m_recipientsFieldsLabel = new QLabel(m_descPage);
    m_recipientsLabel->setMaximumSize(DESC_LABEL_SIZE);
    m_recipientsLabel->setText(QString(tr("To:")));
    m_recipientsFieldsLabel->setAlignment(Qt::AlignRight);

    m_ccLabel = new QLabel(m_descPage);
    m_ccFieldsLabel = new QLabel(m_descPage);
    m_ccLabel->setMaximumSize(DESC_LABEL_SIZE);
    m_ccLabel->setText(QString(tr("Cc:")));
    m_ccFieldsLabel->setAlignment(Qt::AlignRight);

    m_attachmentLabel = new QLabel(m_descPage);
    m_attachmentFieldsLabel = new QLabel(m_descPage);
    m_attachmentLabel->setMaximumSize(DESC_LABEL_SIZE);
    m_attachmentLabel->setText(QString(tr("Attachment:")));
    m_attachmentFieldsLabel->setAlignment(Qt::AlignRight);

    m_descPageLyt->setContentsMargins(DESC_LABEL_MARGINS);
    m_descPageLyt->setVerticalSpacing(DESC_LABEL_SPACING);
    m_descPageLyt->addRow(m_senderLabel, m_senderFieldsLabel);
    m_descPageLyt->addRow(m_timeLabel, m_timeFieldsLabel);
    m_descPageLyt->addRow(m_recipientsLabel, m_recipientsFieldsLabel);
    m_descPageLyt->addRow(m_ccLabel, m_ccFieldsLabel);
    m_descPageLyt->addRow(m_attachmentLabel, m_attachmentFieldsLabel);
    */

}
