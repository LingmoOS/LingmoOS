// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "selectfilepage.h"
#include "disklistitem.h"
#include "partitionlistitem.h"
#include "../corelib/ddevicediskinfo.h"
#include "../corelib/ddevicepartinfo.h"
#include "utilitylist.h"
#include "iconlabel.h"
#include "../corelib/helper.h"

#include <DCommandLinkButton>

#include <QVBoxLayout>
#include <QFileDialog>
#include <QLabel>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QMimeDatabase>
#include <QDebug>
#include <QProcess>
#include <QApplication>

#include <pwd.h>
#include <unistd.h>

DWIDGET_USE_NAMESPACE

class SelectFileWidget : public QWidget
{
    Q_OBJECT

public:
    enum Mode {
        GetSaveName,
        GetFile
    };

    explicit SelectFileWidget(Mode mode, QWidget *parent = 0);

    QString selectFilePath() const;

signals:
    void filePathChanged();

protected:
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    void setFilePath(const QString &path);
    void openFileDialog();

    Mode m_mode;
    QString m_filePath;
    QString m_defaultFileName;
    QLabel *m_dragDropLabel = 0;
    IconLabel *m_label;
    DCommandLinkButton *m_button;

    friend class SelectFilePage;
};

SelectFileWidget::SelectFileWidget(Mode mode, QWidget *parent)
    : QWidget(parent)
    , m_mode(mode)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_label = new IconLabel(this, true);

    m_label->setDirection(QBoxLayout::TopToBottom);

    if (mode == GetSaveName) {
        m_label->setIcon(QIcon::fromTheme("inode-directory"), 80);
    } else {
        m_label->setIcon(QIcon::fromTheme("application-x-deepinclone-dim"), 80);
        setAcceptDrops(true);
    }

    m_button = new DCommandLinkButton(mode == GetSaveName ? tr("Select storage location") : tr("Select image file"), this);

    layout->addStretch();
    layout->addWidget(m_label, 0, Qt::AlignHCenter);

    if (mode == GetFile) {
        m_dragDropLabel = new QLabel(tr("Drag and drop the backup image file here"), this);
        m_dragDropLabel->setObjectName("DragDropLabel");

        QWidget *split_line = new QWidget(this);

        split_line->setFixedSize(220, 2);
        split_line->setObjectName("SplitLine");

        layout->addWidget(m_dragDropLabel, 0, Qt::AlignHCenter);
        layout->addSpacing(10);
        layout->addWidget(split_line, 0, Qt::AlignHCenter);
        layout->addSpacing(10);
    }

    layout->addWidget(m_button, 0, Qt::AlignHCenter);
    layout->addStretch();

    connect(m_button, &DCommandLinkButton::clicked, this, &SelectFileWidget::openFileDialog);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

QString SelectFileWidget::selectFilePath() const
{
    return m_filePath;
}

void SelectFileWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        const QList<QUrl> urls = event->mimeData()->urls();

        if (urls.count() != 1)
            return QWidget::dragEnterEvent(event);

        const QUrl &first = urls.first();

        if (!first.isLocalFile())
            return QWidget::dragEnterEvent(event);

        QMimeDatabase db;
        const QMimeType &type = db.mimeTypeForUrl(first);

        if (type.name() != "application/x-deepinclone-dim")
            return QWidget::dragEnterEvent(event);

        return event->accept();
    }

    return QWidget::dragEnterEvent(event);
}

void SelectFileWidget::dropEvent(QDropEvent *event)
{
    setFilePath(event->mimeData()->urls().first().toLocalFile());

    return event->accept();
}

void SelectFileWidget::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    openFileDialog();
}

void SelectFileWidget::setFilePath(const QString &path)
{
    if (m_filePath == path)
        return;

    m_filePath = path;

    if (m_dragDropLabel) {
        m_button->setText(tr("Reselect image file"));
        m_dragDropLabel->hide();
    } else {
        m_button->setText(tr("Reselect storage location"));

        if (!path.isEmpty() && QFileInfo(path).isDir()) {
            m_filePath = QDir(path).absoluteFilePath(m_defaultFileName);
        }
    }

    m_label->setTitle(m_filePath);

    emit filePathChanged();
}

extern QStringList honestChildEnvironment();
extern QString findHonestChild();

void SelectFileWidget::openFileDialog()
{
    static bool opened = false;

    if (opened)
        return;

    opened = true;

    QString honest_child = findHonestChild();

    do {
        if (honest_child.isEmpty()) {
            break;
        }

        QProcess process;
        QStringList env_list = process.environment();

        env_list << honestChildEnvironment();
        env_list << QString("%1=%2").arg(DEEPIN_CLONE_OPEN_DIALOG).arg(m_mode == GetSaveName ? DEEPIN_CLONE_GET_FILE : "");
        env_list << QString("%1=%2").arg(DEEPIN_CLONE_PARENT_WINID).arg(window()->winId());
        env_list << QString("%1=%2").arg(DEEPIN_CLONE_TITLE).arg(m_button->text());
        env_list << QString("%1=%2").arg(DEEPIN_CLONE_FILE_NAME).arg(m_defaultFileName);
        env_list << QString("%1=%2").arg(DEEPIN_CLONE_NAME_FILTER).arg(tr("Deepin Image File"));
        env_list << QString("QT_QPA_PLATFORMTHEME=deepin");

        process.setEnvironment(env_list);
        process.setProcessChannelMode(QProcess::ForwardedErrorChannel);

        connect(qApp, &QApplication::applicationStateChanged,
                &process, [&process] (Qt::ApplicationState state) {
            if (state == Qt::ApplicationActive) {
                process.write("a\n");
            }
        });

        Helper::processExec(&process, honest_child, {}, -1, QIODevice::ReadWrite);

        if (process.error() == QProcess::FailedToStart)
            break;

        if (!Helper::lastProcessStandardOutput().isEmpty())
            setFilePath(Helper::lastProcessStandardOutput());

        opened = false;

        return;
    } while (false);

    QString home_path;

    if (qEnvironmentVariableIsSet("PKEXEC_UID")) {
        const quint32 pkexec_uid = qgetenv("PKEXEC_UID").toUInt();

        home_path = getpwuid(pkexec_uid)->pw_dir;
    }

    QFileDialog dialog(this, QString(), home_path);

    dialog.setMimeTypeFilters(QStringList() << "application-x-deepinclone-dim");
    dialog.setNameFilters(QStringList() << tr("Deepin Image File") + "(*.dim)");
    dialog.setDefaultSuffix("dim");
    dialog.setWindowTitle(m_button->text());

    if (m_mode == GetSaveName) {
        dialog.setFileMode(QFileDialog::AnyFile);
        dialog.setAcceptMode(QFileDialog::AcceptSave);
        dialog.selectFile(m_defaultFileName);
    } else {
        dialog.setFileMode(QFileDialog::ExistingFile);
        dialog.setAcceptMode(QFileDialog::AcceptOpen);
    }

    if (dialog.exec() == QFileDialog::Accepted) {
        setFilePath(dialog.selectedFiles().first());
    }

    opened = false;
}

static QString getFilePath(const QWidget *widget)
{
    if (const UtilityList *list = qobject_cast<const UtilityList*>(widget)) {
        const QList<QListWidgetItem*> selected_items = list->selectedItems();

        if (selected_items.isEmpty())
            return QString();

        QListWidgetItem *item = selected_items.first();

        if (item->isHidden())
            return QString();

        const QWidget *widget = list->itemWidget(item);

        if (const DiskListItem *item = qobject_cast<const DiskListItem*>(widget)) {
            return item->info().filePath();
        } else if (const PartitionListItem *item = qobject_cast<const PartitionListItem*>(widget)) {
            return item->info().filePath();
        }
    } else if (const SelectFileWidget *file = qobject_cast<const SelectFileWidget*>(widget)) {
        return file->selectFilePath();
    }

    return QString();
}

SelectFilePage::SelectFilePage(SelectActionPage::Mode mode, SelectActionPage::Action action, QWidget *parent)
    : ContentPairPage(parent)
{
    QWidget *left_widget = 0;
    QWidget *right_widget = 0;

    switch (action) {
    case SelectActionPage::Clone: {
        UtilityList *left_list = new UtilityList(this);
        UtilityList *right_list = new UtilityList(this);

        left_widget = left_list;
        right_widget = right_list;

        if (mode == SelectActionPage::Disk) {
            setLeftTitle(tr("Select the source disk"));
            setRightTitle(tr("Select the target disk"));

            for (const DDiskInfo &info : DDeviceDiskInfo::localeDiskList()) {
                DiskListItem *item = new DiskListItem(info, left_list);

                left_list->addItem(item);

                if (!info.isReadonly()) {
                    item = new DiskListItem(info, right_list);

                    right_list->addItem(item);
                }
            }
        } else {
            setLeftTitle(tr("Select the source partition"));
            setRightTitle(tr("Select the target partition"));

            for (const DPartInfo &info : DDevicePartInfo::localePartList()) {
                PartitionListItem *item = new PartitionListItem(info, left_list);

                left_list->addItem(item);

                if (!info.isReadonly()) {
                    item = new PartitionListItem(info, right_list);

                    right_list->addItem(item);
                }
            }
        }

        hideItemForFile(getFilePath(left_widget), right_list);

        connect(left_list, &UtilityList::itemSelectionChanged, this, &SelectFilePage::onSourceChanged);
        connect(left_list, &UtilityList::itemSelectionChanged, this, &SelectFilePage::sourceChanged, Qt::QueuedConnection);
        connect(right_list, &UtilityList::itemSelectionChanged, this, &SelectFilePage::targetChanged, Qt::QueuedConnection);
        break;
    }
    case SelectActionPage::Backup: {
        UtilityList *left_list = new UtilityList(this);

        left_widget = left_list;

        if (mode == SelectActionPage::Disk) {
            setLeftTitle(tr("Select a disk to backup"));

            for (const DDiskInfo &info : DDeviceDiskInfo::localeDiskList()) {
                DiskListItem *item = new DiskListItem(info, left_list);

                left_list->addItem(item);
            }
        } else {
            setLeftTitle(tr("Select a partition to backup"));

            for (const DPartInfo &info : DDevicePartInfo::localePartList()) {
                PartitionListItem *item = new PartitionListItem(info, left_list);

                left_list->addItem(item);
            }
        }

        setRightTitle(tr("Select storage location"));

        SelectFileWidget *sfw = new SelectFileWidget(SelectFileWidget::GetSaveName, this);

        connect(left_list, &UtilityList::itemSelectionChanged, this, [sfw, left_list] {
            QWidget *widget = left_list->itemWidget(left_list->selectedItems().first());
            QString device_model;

            if (DiskListItem *item = qobject_cast<DiskListItem*>(widget)) {
                device_model = item->info().model().simplified();
            } else if (PartitionListItem *item = qobject_cast<PartitionListItem*>(widget)) {
                device_model = QString("%1-%2").arg(DDeviceDiskInfo(item->info().name()).model().simplified()).arg(item->info().indexNumber());
            }

            if (!device_model.isEmpty())
                sfw->m_defaultFileName = QString("%1.dim").arg(device_model);
        });

        emit left_list->itemSelectionChanged();
        right_widget = sfw;

        connect(left_list, &UtilityList::itemSelectionChanged, this, &SelectFilePage::sourceChanged);
        connect(sfw, &SelectFileWidget::filePathChanged, this, &SelectFilePage::targetChanged);
        break;
    }
    case SelectActionPage::Restore: {
        setLeftTitle(tr("Select a backup image file"));

        if (mode == SelectActionPage::Disk)
            setRightTitle(tr("Select a disk to restore"));
        else
            setRightTitle(tr("Select a partition to restore"));

        SelectFileWidget *sfw = new SelectFileWidget(SelectFileWidget::GetFile, this);
        UtilityList *right_list = new UtilityList(this);

        left_widget = sfw;
        right_widget = right_list;

        if (mode == SelectActionPage::Disk) {
            for (const DDiskInfo &info : DDeviceDiskInfo::localeDiskList()) {
                DiskListItem *item = new DiskListItem(info, right_list);

                right_list->addItem(item);
            }
        } else {
            for (const DPartInfo &info : DDevicePartInfo::localePartList()) {
                PartitionListItem *item = new PartitionListItem(info, right_list);

                right_list->addItem(item);
            }
        }

        connect(sfw, &SelectFileWidget::filePathChanged, this, &SelectFilePage::sourceChanged);
        connect(right_list, &UtilityList::itemSelectionChanged, this, &SelectFilePage::targetChanged);
        break;
    }
    default:
        break;
    }

    setLeftContent(left_widget, true);
    setRightContent(right_widget, true);
}

QString SelectFilePage::source() const
{
    return getFilePath(leftContent());
}

QString SelectFilePage::target() const
{
    return getFilePath(rightContent());
}

void SelectFilePage::onSourceChanged()
{
    const QString &left_file = source();

    if (UtilityList *list = qobject_cast<UtilityList*>(rightContent())) {
        hideItemForFile(left_file, list);
    }
}

void SelectFilePage::hideItemForFile(const QString &filePath, UtilityList *list)
{
    bool hide_tareget_is_selected = false;

    for (int i = 0; i < list->count(); ++i) {
        QListWidgetItem *item = list->item(i);
        QString file_path;

        if (const DiskListItem *widget = qobject_cast<const DiskListItem*>(list->itemWidget(item)))
            file_path = widget->info().filePath();
        else if (const PartitionListItem *widget = qobject_cast<const PartitionListItem*>(list->itemWidget(item)))
            file_path = widget->info().filePath();

        if (file_path == filePath) {
            item->setHidden(true);
            hide_tareget_is_selected = item->isSelected();
        } else {
            item->setHidden(false);
        }
    }

    if (!hide_tareget_is_selected)
        return;

    for (int i = 0; i < list->count(); ++i) {
        QListWidgetItem *item = list->item(i);

        if (!item->isHidden())
            item->setSelected(true);
    }
}

void SelectFilePage::selectItemForFile(const QString &filePath, UtilityList *list)
{
    for (int i = 0; i < list->count(); ++i) {
        QListWidgetItem *item = list->item(i);
        QString file_path;

        if (const DiskListItem *widget = qobject_cast<const DiskListItem*>(list->itemWidget(item)))
            file_path = widget->info().filePath();
        else if (const PartitionListItem *widget = qobject_cast<const PartitionListItem*>(list->itemWidget(item)))
            file_path = widget->info().filePath();

        if (file_path == filePath) {
            item->setSelected(true);
        }
    }
}

void SelectFilePage::setSource(const QString &source)
{
    if (UtilityList *list = qobject_cast<UtilityList*>(leftContent())) {
        selectItemForFile(source, list);
    } else if (SelectFileWidget *widget = qobject_cast<SelectFileWidget*>(leftContent())) {
        widget->setFilePath(source);
    }
}

void SelectFilePage::setTarget(const QString &target)
{
    if (UtilityList *list = qobject_cast<UtilityList*>(rightContent())) {
        selectItemForFile(target, list);
    } if (SelectFileWidget *widget = qobject_cast<SelectFileWidget*>(rightContent())) {
        widget->setFilePath(target);
    }
}

#include "selectfilepage.moc"
