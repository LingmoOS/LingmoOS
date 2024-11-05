/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "default-preview-page.h"
#include "thumbnail-manager.h"

#include <QLabel>

#include <QFile>
#include <QFileInfo>
#include <QUrl>

#include <QPixmap>
#include <QIcon>
#include <QMimeDatabase>

#include <QVBoxLayout>
#include <QFormLayout>

#include <QLocale>
#include <QDateTime>

#include <QThreadPool>

#include <QResizeEvent>

#include <QImageReader>

#include <QPainter>
#include <QGSettings>
#include <QDBusConnection>
#include <QDBusReply>

#include "icon-container.h"

#include "file-info.h"
#include "file-info-manager.h"
#include "file-watcher.h"

#include "file-info-job.h"

#include "file-count-operation.h"

#include "FMWindowIface.h"
#include "file-utils.h"
#include "global-settings.h"

using namespace Peony;

#define LABEL_MAX_WIDTH       165

DefaultPreviewPage::DefaultPreviewPage(QWidget *parent) : QStackedWidget (parent)
{
    setContentsMargins(10, 20, 10, 20);

    auto label = new QLabel(tr("Select the file you want to preview..."), this);
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignCenter);
    m_empty_tab_widget = label;

    /*
    label = new QLabel(this);
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignCenter);
    */
    auto previewPage = new FilePreviewPage(this);
    previewPage->installEventFilter(this);
    m_preview_tab_widget = previewPage;

    addWidget(m_preview_tab_widget);
    addWidget(m_empty_tab_widget);

    setCurrentWidget(m_empty_tab_widget);

    if (QGSettings::isSchemaInstalled("org.lingmo.style")) {
        QGSettings *gSetting = new QGSettings("org.lingmo.style", QByteArray(), this);
        connect(gSetting, &QGSettings::changed, this, [=](const QString &key) {
            if ("systemFontSize" == key) {
                if (m_support && m_preview_tab_widget) {
                    if (m_info) {
                        m_preview_tab_widget->updateInfo(m_info.get());
                    }
                }
            }
        });
    }

    if (QGSettings::isSchemaInstalled("org.lingmo.control-center.panel.plugins")) {
        QGSettings* settings = new QGSettings("org.lingmo.control-center.panel.plugins", QByteArray(), this);
        connect(settings, &QGSettings::changed, this, [=](const QString &key) {
            if ("hoursystem" == key) {
                if (m_support && m_preview_tab_widget) {
                    if (m_info) {
                        FileInfoJob* infoJob = new FileInfoJob(m_info, this);
                        infoJob->setAutoDelete(true);
                        connect(infoJob, &FileInfoJob::queryAsyncFinished, this, [=] {
                            m_preview_tab_widget->updateInfo(m_info.get());
                        });
                        infoJob->queryAsync();
                    }
                }
            }
        });
    }

#ifdef LINGMO_SDK_DATE
    connect(GlobalSettings::getInstance(),
            &GlobalSettings::updateLongDataFormat,
            this,
            &DefaultPreviewPage::updateDateFormat);
#endif
}

DefaultPreviewPage::~DefaultPreviewPage()
{
    cancel();
}

void DefaultPreviewPage::updateDateFormat(QString dateFormat)
{
    //update date and time show format, task #101605
    qDebug() << "sdk format signal:"<<dateFormat;
    if (m_date_format != dateFormat){
        if (m_support && m_preview_tab_widget) {
            if (m_info) {
                FileInfoJob* infoJob = new FileInfoJob(m_info, this);
                infoJob->setAutoDelete(true);
                connect(infoJob, &FileInfoJob::queryAsyncFinished, this, [=] {
                    m_preview_tab_widget->updateInfo(m_info.get());
                });
                infoJob->queryAsync();
            }
        }
        m_date_format = dateFormat;
    }
}

bool DefaultPreviewPage::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj == m_preview_tab_widget) {
        if (ev->type() == QEvent::Resize) {
            auto e = static_cast<QResizeEvent*>(ev);
            auto page = qobject_cast<FilePreviewPage*>(m_preview_tab_widget);
            int width = e->size().width();
            width = qMax(width, 96);
            width = qMin(width, 256);
            page->resizeIcon(QSize(width* 2/3, width* 3/4));
            page->updateForm(e->size());
        }
    }
    return QStackedWidget::eventFilter(obj, ev);
}

void DefaultPreviewPage::prepare(const QString &uri, PreviewType type)
{
    m_current_uri = uri;
    m_info = FileInfo::fromUri(uri);
    m_current_type = type;
    m_support = (uri.contains("file:///") || uri.contains("mtp://") || uri.contains("gphoto2://")) ? true : false;
    m_watcher = std::make_shared<FileWatcher>(uri);
    connect(m_watcher.get(), &FileWatcher::locationChanged, [=](const QString &, const QString &newUri) {
        this->prepare(newUri);
        this->startPreview();
    });
    m_watcher->startMonitor();
}

void DefaultPreviewPage::prepare(const QString &uri)
{
    prepare(uri, Other);
}

void DefaultPreviewPage::startPreview()
{
    FMWindowIface *iface = dynamic_cast<FMWindowIface *>(this->topLevelWidget());
    bool locationSupport = true;
    if (iface) {
        if (iface->getCurrentUri() == "computer:///") {
            locationSupport = false;
        }
    }
    if (m_support && locationSupport) {
        auto previewPage = qobject_cast<FilePreviewPage*>(m_preview_tab_widget);
        previewPage->updateInfo(m_info.get());
        setCurrentWidget(previewPage);
    } else {
        QLabel *label = qobject_cast<QLabel*>(m_empty_tab_widget);
        label->setText(tr("Can not preview this file."));
    }
}

void DefaultPreviewPage::cancel()
{
    m_preview_tab_widget->cancel();
    setCurrentWidget(m_empty_tab_widget);
    QLabel *label = qobject_cast<QLabel*>(m_empty_tab_widget);
    label->setText(tr("Select the file you want to preview..."));
}

void DefaultPreviewPage::closePreviewPage()
{
    cancel();
    deleteLater();
}

void DefaultPreviewPage::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(this->rect(), this->palette().base());
    QStackedWidget::paintEvent(e);
}

FilePreviewPage::FilePreviewPage(QWidget *parent) : QFrame(parent)
{
    m_layout = new QVBoxLayout(this);
    setLayout(m_layout);

    m_icon = new IconContainer(this);
    //fix bug#201023,200758, 200393 pure picture preview issue
    m_icon->setProperty("useIconHighlightEffect", 0x0);
    m_icon->setIconSize(QSize(96, 96));
    m_layout->addWidget(m_icon);
    m_layout->addSpacing(24);

    m_form = new QFormLayout(this);
    m_form->setSpacing(3);
    m_form->setContentsMargins(0, 0, 0, 0);

    m_display_name_label = new QLabel(this);
    QFont font;
    font.setBold(true);
    m_display_name_label->setFont(font);
    m_layout->addWidget(m_display_name_label);
    m_form_label_map.insert(m_display_name_label, "");
    m_layout->addSpacing(16);

    m_type_label = new QLabel(this);
    m_form->addRow(tr("File Type:"), m_type_label);
    m_type_label->setAlignment(Qt::AlignRight);
    m_form_label_map.insert(m_type_label, "");

    addSeparator();

    m_total_size_label = new QLabel(this);
    m_form->addRow(tr("Size:"), m_total_size_label);
    m_total_size_label->setAlignment(Qt::AlignRight);
    m_form_label_map.insert(m_total_size_label, "");

    addSeparator();

    m_time_create_label = new QLabel(this);
    m_form->addRow(tr("Time Created:"), m_time_create_label);
    m_time_create_label->setAlignment(Qt::AlignRight);
    m_form_label_map.insert(m_time_create_label, "");

    addSeparator();

    m_time_modified_label = new QLabel(this);
    m_form->addRow(tr("Time Modified:"), m_time_modified_label);
    m_time_modified_label->setAlignment(Qt::AlignRight);
    m_form_label_map.insert(m_time_modified_label, "");

    addSeparator();

    m_time_access_label = new QLabel(this);
    m_form->addRow(tr("Time Access:"), m_time_access_label);
    m_time_access_label->setAlignment(Qt::AlignRight);
    m_form_label_map.insert(m_time_access_label, "");

    addSeparator();

    m_file_count_label = new QLabel(this);
    QLabel *children_label = new QLabel(this);
    children_label->setAlignment(Qt::AlignTop);
    children_label->setText(tr("Children Count:"));
    m_form->addRow(children_label, m_file_count_label);
    m_file_count_label->setAlignment(Qt::AlignRight);
    m_form_label_map.insert(m_file_count_label, "");

    addSeparator();

    //image
    m_image_size = new QLabel(this);
    m_form->addRow(tr("Image resolution:"), m_image_size);
    m_image_size->setAlignment(Qt::AlignRight);
    m_form_label_map.insert(m_image_size, "");

    addSeparator();

    m_image_format = new QLabel(this);
    m_form->addRow(tr("color model:"), m_image_format);
    m_image_format->setAlignment(Qt::AlignRight);
    m_form_label_map.insert(m_image_format, "");

    addSeparator();

    m_form->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    m_form->setFormAlignment(Qt::AlignHCenter);

    QWidget *form = new QWidget(this);
    form->setLayout(m_form);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setAlignment(Qt::AlignHCenter);
    m_layout->addWidget(form);
    m_layout->addStretch(1);
}

FilePreviewPage::~FilePreviewPage()
{

}

void FilePreviewPage::wrapData(QLabel *p_label, const QString &text)
{
    QString wrapText = text;
    QFontMetrics fontMetrics = p_label->fontMetrics();
    int textSize = fontMetrics.width(wrapText);

    int width = p_label->width()==0?LABEL_MAX_WIDTH:p_label->width();
    if(textSize > width){
        int lastIndex = 0;
        for(int i = lastIndex+1; i <= wrapText.length(); i++) {
            QString line = wrapText.mid(lastIndex, i - lastIndex);
            if(fontMetrics.width(line) == width) {
                lastIndex = i;
                if (i != wrapText.length()) {
                    wrapText.insert(i, '\n');
                }
            } else if(fontMetrics.width(line) > width) {
                lastIndex = i;
                wrapText.insert(i - 1, '\n');
            } else {
                continue;
            }
        }
    }

    p_label->setText(wrapText);
}

void FilePreviewPage::updateInfo(FileInfo *info)
{
    if (info->displayName().isEmpty() ||
            info->fileType().isEmpty() ||
            info->accessTime() == 0 ||
            info->modifiedTime() == 0) {
        FileInfoJob j(info->uri());
        j.querySync();
    }
    auto thumbnail = ThumbnailManager::getInstance()->tryGetThumbnail(info->uri());
    if (!thumbnail.isNull()) {
        QUrl url = info->uri();
        thumbnail.addFile(url.path());
    }
    auto icon = QIcon::fromTheme(info->iconName(), QIcon::fromTheme("unknown"));
    m_icon->setIcon(thumbnail.isNull()? icon: thumbnail);
    //fix bug:#82320
    QString displayName = info->displayName();
    if (QRegExp("^file:///data/usershare(/{,1})$").exactMatch(info->uri())) {
        displayName = tr("usershare");
    }

    QString accessDate = info->accessDate();
    QString modifyDate = info->modifiedDate();
#ifdef LINGMO_SDK_DATE
    accessDate = GlobalSettings::getInstance()->transToSystemTimeFormat(info->accessTime(), true);
    modifyDate = GlobalSettings::getInstance()->transToSystemTimeFormat(info->modifiedTime(), true);
#endif

    wrapData(m_display_name_label, displayName);
    m_form_label_map[m_display_name_label] = displayName;

    wrapData(m_type_label, info->fileType());
    m_form_label_map[m_type_label] = info->fileType();

    quint64 timeNum = FileUtils::getCreateTimeOfMicro (info->uri());
    quint64 modifiedTime = info->modifiedTime();
    // 客户需要必须显示创建时间，因此使用时间最小时间戳为创建时间
    quint64 minTime = timeNum != 0 ? timeNum : modifiedTime;
    minTime = qMin (minTime, modifiedTime);
    if (info->accessTime() != 0) {
        minTime = qMin (minTime, info->accessTime());
    }
    QDateTime createDate = QDateTime::fromMSecsSinceEpoch(minTime*1000);
    QString createTime = createDate.toString(GlobalSettings::getInstance()->getSystemTimeFormat());
    wrapData(m_time_create_label, createTime);
    m_form_label_map[m_time_create_label] = createTime;

    wrapData(m_time_access_label, accessDate);
    m_form_label_map[m_time_access_label] = accessDate;

    wrapData(m_time_modified_label, modifyDate);
    m_form_label_map[m_time_modified_label] = modifyDate;

    m_file_count_label->setText(tr(""));
    m_form_label_map[m_file_count_label] = "";

    if (info->isDir()) {
        m_form->labelForField(m_file_count_label)->setVisible(true);
        m_file_count_label->setVisible(true);
        m_form->itemAt(11, QFormLayout::SpanningRole)->widget()->setVisible(true);
    } else {
        m_form->labelForField(m_file_count_label)->setVisible(false);
        m_file_count_label->setVisible(false);
        m_form->itemAt(11, QFormLayout::SpanningRole)->widget()->setVisible(false);
    }

    if (info->mimeType().startsWith("image/")) {
        QUrl url = info->uri();
        QImageReader r(url.path());

        auto image_size_row_left = m_form->labelForField(m_image_size);
        image_size_row_left->setVisible(true);

        m_image_size->setText(tr("%1x%2").arg(r.size().width()).arg(r.size().height()));
        m_form_label_map[m_image_size] = tr("%1x%2").arg(r.size().width()).arg(r.size().height());

        auto thumbnail = ThumbnailManager::getInstance()->tryGetThumbnail(info->uri());
        bool rgba = thumbnail.pixmap(r.size()).hasAlphaChannel();
        m_image_size->setVisible(true);

        auto image_format_row_left = m_form->labelForField(m_image_format);
        image_format_row_left->setVisible(true);
        m_image_format->setText(rgba? "RGBA": "RGB");
        m_form_label_map[m_image_format] = rgba? "RGBA": "RGB";
        m_image_format->setVisible(true);
        m_form->itemAt(13, QFormLayout::SpanningRole)->widget()->setVisible(true);
        m_form->itemAt(15, QFormLayout::SpanningRole)->widget()->setVisible(true);
    } else {
        auto image_size_row_left = m_form->labelForField(m_image_size);
        image_size_row_left->setVisible(false);
        m_image_size->setVisible(false);
        auto image_format_row_left = m_form->labelForField(m_image_format);
        image_format_row_left->setVisible(false);
        m_image_format->setVisible(false);
        m_form->itemAt(13, QFormLayout::SpanningRole)->widget()->setVisible(false);
        m_form->itemAt(15, QFormLayout::SpanningRole)->widget()->setVisible(false);
    }
    if (info->fileType().startsWith("video/")) {

    }
    if (info->fileType().startsWith("audio/")) {

    }

    if (info->isSymbolLink()&&!info->symlinkTarget().isEmpty()){
        countAsync("file:///" + info->symlinkTarget());
    } else {
        countAsync(info->uri());
    }
}

void FilePreviewPage::countAsync(const QString &uri)
{
    cancel();
    m_file_count = 0;
    m_hidden_count = 0;
    m_total_size = 0;

    QStringList uris;
    uris<<uri;
    //FIXME: replace BLOCKING api in ui thread.
    auto info = FileInfo::fromUri(uri);
    m_count_op = new FileCountOperation(uris, !info->isDir());
    connect(m_count_op, &FileOperation::operationStarted, this, &FilePreviewPage::resetCount, Qt::BlockingQueuedConnection);
    connect(m_count_op, &FileOperation::operationPreparedOne, this, &FilePreviewPage::onPreparedOne, Qt::BlockingQueuedConnection);
    connect(m_count_op, &FileCountOperation::countDone, this, &FilePreviewPage::onCountDone, Qt::BlockingQueuedConnection);
    QThreadPool::globalInstance()->start(m_count_op);
}

void FilePreviewPage::updateCount()
{
    wrapData(m_file_count_label, tr("%1 total, %2 hidden").arg(m_file_count).arg(m_hidden_count));
    m_form_label_map[m_file_count_label] = tr("%1 total, %2 hidden").arg(m_file_count).arg(m_hidden_count);

    auto format = g_format_size_full(m_total_size,G_FORMAT_SIZE_IEC_UNITS);
    QString fileSize(format);
    if (fileSize.contains("KiB")) {
        fileSize.replace("KiB", "KB");
    } else if (fileSize.contains("MiB")) {
        fileSize.replace("MiB", "MB");
    } else if (fileSize.contains("GiB")) {
        fileSize.replace("GiB", "GB");
    }
    m_total_size_label->setText(fileSize);
    m_form_label_map[m_total_size_label] = fileSize;
    g_free(format);
    updateForm(this->size());
}

void FilePreviewPage::cancel()
{
    if (m_count_op) {
        m_count_op->blockSignals(true);
        m_count_op->cancel();
        onCountDone();
    }
    m_count_op = nullptr;
}

void FilePreviewPage::resizeIcon(QSize size)
{
    m_icon->setIconSize(size);
}

void FilePreviewPage::resetCount()
{
    m_file_count = 0;
    m_hidden_count = 0;
    m_total_size = 0;
    updateCount();
}

void FilePreviewPage::onCountDone()
{
    if (!m_count_op)
        return;
    m_count_op->getInfo(m_file_count, m_hidden_count, m_total_size);
    this->updateCount();
    m_count_op = nullptr;
    m_file_count = 0;
    m_hidden_count = 0;
    m_total_size = 0;
}

void FilePreviewPage::updateForm(QSize size)
{
    int labelWidth = 0;
    QMap<QLabel*, QString>::const_iterator i = m_form_label_map.constBegin();
    int iLongTextWidth = 0;
    while (i != m_form_label_map.constEnd()) {
        if (i.key() == m_display_name_label) {
            iLongTextWidth = qMax(fontMetrics().width(i.value()), iLongTextWidth);
            ++i;
            continue;
        }
        iLongTextWidth = qMax(fontMetrics().width(i.value()), iLongTextWidth);
        QString text =((QLabel*)m_form->labelForField(i.key()))->text();
        labelWidth = qMax(fontMetrics().width(text), labelWidth);
        ++i;
    }
    i = m_form_label_map.constBegin();

    int fieldWidth = size.width() - 3*m_form->spacing() - labelWidth - 30;
    fieldWidth = qMin(fieldWidth,iLongTextWidth);

    while (i != m_form_label_map.constEnd()) {
        auto label = i.key();
        if (label) {
            if (fieldWidth == label->width()) {
                return;
            }
            if (m_display_name_label == label) {
                label->setFixedWidth(fieldWidth + labelWidth);
            } else {
                label->setFixedWidth(fieldWidth);
            }
            wrapData(i.key(), i.value());
        }
        ++i;
    }
}

void FilePreviewPage::addSeparator()
{
    auto separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    m_form->addRow(separator);
}
