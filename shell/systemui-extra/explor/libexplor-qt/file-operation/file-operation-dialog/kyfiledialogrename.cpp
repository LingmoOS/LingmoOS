/*
 * Peony-Qt
 *
 * Copyright (C) 2023, KylinSoft Information Technology Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifdef LINGMO_FILE_DIALOG

#include "kyfiledialogrename.h"

#include <QStackedWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QTextEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QFileInfo>
#include <QStandardPaths>

#include <QApplication>
#include <QFontMetrics>
#include <QDBusInterface>
#include <QDBusReply>

#include "file-utils.h"
#include "rename-editor.h"

#include "global-settings.h"

#define PEONY_TRUNCATE_NAME_LIMIT 225
#define PEONY_RENAME_LIMIT 255

KyFileDialogRename::KyFileDialogRename(QWidget *parent) : KyFileOperationDialog(parent), Peony::FileOperationErrorHandler()
{
    bool isTabletMode = false;
    m_statusManagerDBus = new QDBusInterface(DBUS_STATUS_MANAGER_IF, "/" ,DBUS_STATUS_MANAGER_IF,QDBusConnection::sessionBus(),this);
    if (m_statusManagerDBus) {
        qDebug() << "[PeonyDesktopApplication::initGSettings] init statusManagerDBus" << m_statusManagerDBus->isValid();
        if (m_statusManagerDBus->isValid()) {
            QDBusReply<bool> message = m_statusManagerDBus->call("get_current_tabletmode");
            if (message.isValid()) {
                isTabletMode = message.value();
            }
        }
    }

    setFixedWidth(600);
    if(true == isTabletMode){
        setFixedHeight(330);
    }else{
        setFixedHeight(300);
    }

}

void KyFileDialogRename::handle(Peony::FileOperationError &error)
{
    Peony::ExceptionResponse responseCode = Peony::ExceptionResponse::Cancel;
    QString newName = Peony::FileUtils::getUriBaseName(error.destDirUri);
    newName = Peony::FileUtils::urlDecode(newName);

    auto stack = new QStackedWidget(this);
    stack->setContentsMargins(20, 0, 20, 20);
    auto layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(stack);
    mainWidget()->setLayout(layout);

    // todo:
    // 根据错误码和文件操作码拼接文件信息
    QString line1;
    QString line2;
    QString line3;

    switch (error.op) {
    case Peony::FileOpRename: {
        line1 = tr("Renaming \"%1\"").arg(newName);
        line2 = tr("Renaming failed, the reason is: %1").arg(error.errorCode == G_IO_ERROR_FILENAME_TOO_LONG? tr("Filename too long"): error.errorStr);
        break;
    }
    case Peony::FileOpCopy:
        line1 = tr("Copying \"%1\"").arg(newName); {
        auto destDir = Peony::FileUtils::urlDecode(error.destDirUri);
        QString fileName = destDir.split("/").last();
        destDir.chop(fileName.count() + 1);
        line2 = tr("To \"%1\"").arg(destDir);
        line3 = tr("Copying failed, the reason is: %1").arg(error.errorCode == G_IO_ERROR_FILENAME_TOO_LONG? tr("Filename too long"): error.errorStr);
        break;
    }
    case Peony::FileOpMove: {
        line1 = tr("Moving \"%1\"").arg(newName);
        auto destDir = Peony::FileUtils::urlDecode(error.destDirUri);
        QString fileName = destDir.split("/").last();
        destDir.chop(fileName.count() + 1);
        line2 = tr("To \"%1\"").arg(destDir);
        line3 = tr("Moving failed, the reason is: %1").arg(error.errorCode == G_IO_ERROR_FILENAME_TOO_LONG? tr("Filename too long"): error.errorStr);
        break;
    }
    default: {
        line1 = tr("File operation error:");
        line2 = tr("The reason is: %1").arg(error.errorCode == G_IO_ERROR_FILENAME_TOO_LONG? tr("Filename too long"): error.errorStr);
        break;
    }
    }

    QString line;
    QStringList messages;
    line = qApp->fontMetrics().elidedText(line1, Qt::ElideMiddle, 500);
    messages.append(line);
    line = qApp->fontMetrics().elidedText(line2, Qt::ElideMiddle, 500);
    messages.append(line);
    line = qApp->fontMetrics().elidedText(line3, Qt::ElideMiddle, 500);
    messages.append(line);

    auto labelText = messages.join('\n');

    // page1
    auto pageOne = new QWidget(this);
    QRadioButton *skip = new QRadioButton(tr("Skip"), this);
    QRadioButton *cutOff = new QRadioButton(tr("Truncation"), this);
    QRadioButton *save = new QRadioButton(tr("Save"), this);
    QRadioButton *rename = new QRadioButton(tr("Rename"), this);
    QButtonGroup *onePageGroup = new QButtonGroup(this);
    QString specification;
    QLabel *specificationLabel = new QLabel(this);
    specificationLabel->setText(specification);
    specificationLabel->setWordWrap(true);

    onePageGroup->addButton(skip, 0);
    onePageGroup->addButton(cutOff, 1);
    onePageGroup->addButton(save, 2);
    onePageGroup->addButton(rename, 3);
    skip->setChecked(true);

    QVBoxLayout *oneLayout = new QVBoxLayout(this);
    auto content = new QLabel;
    //content->setWordWrap(true);
    //int textWidth = (this->width() - 40 - 40 - 24) * 3;
    //QString elidedString = qApp->fontMetrics().elidedText(error.errorStr, Qt::ElideMiddle, textWidth);
    content->setText(labelText);
    auto labelIcon = new QLabel;
    labelIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(24, 24));
    labelIcon->setAlignment(Qt::AlignTop);

    QHBoxLayout *centerLayout = new QHBoxLayout(this);
    centerLayout->addWidget(skip);
    centerLayout->addWidget(cutOff);
    centerLayout->addWidget(save);
    centerLayout->addWidget(rename);
    centerLayout->addStretch();
    QCheckBox *allApp = new QCheckBox(tr("All applications"), this);
    QDialogButtonBox *buttonBoxOne = new QDialogButtonBox(this);
    buttonBoxOne->setStandardButtons(QDialogButtonBox::NoButton);
    QPushButton *cancelBt = buttonBoxOne->addButton(tr("Cancel"), QDialogButtonBox::ActionRole);
    QPushButton *applyBt = buttonBoxOne->addButton(tr("Apply"), QDialogButtonBox::ActionRole);
    applyBt->setDefault(true);
    applyBt->setProperty("isImportant", true);
    QHBoxLayout *bottomLayout = new QHBoxLayout(this);
    bottomLayout->addWidget(allApp);
    bottomLayout->addStretch();
    bottomLayout->addWidget(buttonBoxOne);
    bottomLayout->setAlignment(buttonBoxOne, Qt::AlignRight);

    oneLayout->addWidget(content);
    oneLayout->addLayout(centerLayout);
    oneLayout->addWidget(specificationLabel);
    oneLayout->addLayout(bottomLayout);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(labelIcon);
    mainLayout->addLayout(oneLayout);
    pageOne->setLayout(mainLayout);

    stack->addWidget(pageOne);

    // page2
    auto page2 = new QWidget(this);
    auto layout2 = new QGridLayout(this);
    auto textEdit = new RenameEditor(this);
    textEdit->setBackgroundRole(QPalette::Button);
    textEdit->setAutoFillBackground(true);
    textEdit->viewport()->setBackgroundRole(QPalette::Button);
    textEdit->viewport()->setAutoFillBackground(true);
//    int height = qApp->fontMetrics().lineSpacing() * 3 + qApp->fontMetrics().descent();
//    textEdit->setFixedHeight(height);

    auto num = new QLabel;
    QString totalNum = QString("<span style=\"color:red;\">%1</span>/%2%3").arg(newName.toLocal8Bit().count()).arg(PEONY_RENAME_LIMIT).arg(tr("Bytes"));
    num->setText(totalNum);
    auto buttonBox2 = new QDialogButtonBox(this);
    buttonBox2->setStandardButtons(QDialogButtonBox::NoButton);
    auto cancel2 = buttonBox2->addButton(tr("Cancel"), QDialogButtonBox::ActionRole);
    auto ensure2 = buttonBox2->addButton(tr("Save"), QDialogButtonBox::ActionRole);
    ensure2->setDefault(true);
    ensure2->setEnabled(false);
    ensure2->setProperty("isImportant", true);

    layout2->addWidget(textEdit, 0, 0, 4, 2, Qt::AlignTop);
    layout2->addWidget(num, 4, 1, Qt::AlignBottom|Qt::AlignRight);
    layout2->addWidget(buttonBox2, 5, 1, Qt::AlignBottom|Qt::AlignRight);
    page2->setLayout(layout2);
    stack->addWidget(page2);

    textEdit->setText(newName);
    textEdit->selectAll();
    //textEdit->setFocus();
    auto cursor = textEdit->textCursor();
    cursor.setPosition(0, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    bool isDir = Peony::FileUtils::getFileIsFolder(textEdit->toPlainText());
    bool isDesktopFile = textEdit->toPlainText().endsWith(".desktop");
    bool isSoftLink = Peony::FileUtils::getFileIsSymbolicLink(textEdit->toPlainText());

    if (!isDesktopFile && !isSoftLink && !isDir && textEdit->toPlainText().contains(".") && !textEdit->toPlainText().startsWith(".")) {
        int n = 1;
        if(textEdit->toPlainText().contains(".tar.")) //ex xxx.tar.gz xxx.tar.bz2
            n = 2;
        while(n){
            cursor.movePosition(QTextCursor::WordLeft, QTextCursor::KeepAnchor, 1);
            cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1);
            --n;
        }
    }

    textEdit->setTextCursor(cursor);
    textEdit->activateWindow();
    //textEdit->setFocus();

    connect(textEdit, &QTextEdit::textChanged, this, [=](){
        QString currentNum ;
        if (textEdit->toPlainText().toLocal8Bit().count() > PEONY_RENAME_LIMIT) {
            ensure2->setEnabled(false);
            currentNum = QString("<span style=\"color:red;\">%1</span>").arg(textEdit->toPlainText().toLocal8Bit().count());
        } else {
            currentNum = QString("<span style=\"color:\">%1</span>").arg(textEdit->toPlainText().toLocal8Bit().count());
            ensure2->setEnabled(true);
        }
        QString totalNum = QString("%1/%2%3").arg(currentNum).arg(PEONY_RENAME_LIMIT).arg(tr("Bytes"));
        num->setText(totalNum);
    });

    // page3
    QWidget *truncationWidget = new QWidget(this);
    QRadioButton *frontTruncation = new QRadioButton(tr("Front truncation"), this);
    QRadioButton *laterTruncation = new QRadioButton(tr("Post truncation"), this);
    QButtonGroup *truncationGroup = new QButtonGroup(this);
    truncationGroup->addButton(laterTruncation, 0);
    truncationGroup->addButton(frontTruncation, 1);
    laterTruncation->setChecked(true);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::NoButton);
    QPushButton *cancelButton = buttonBox->addButton(tr("Cancel"), QDialogButtonBox::ActionRole);
    QPushButton *okButton = buttonBox->addButton(tr("OK"), QDialogButtonBox::ActionRole);
    okButton->setDefault(true);
    okButton->setProperty("isImportant", true);

    QHBoxLayout *pageThreebottomLayout = new QHBoxLayout(this);
    pageThreebottomLayout->addWidget(frontTruncation);
    pageThreebottomLayout->addWidget(laterTruncation);
    pageThreebottomLayout->addStretch();
    pageThreebottomLayout->addWidget(cancelButton);
    pageThreebottomLayout->addWidget(okButton);

    QLabel *truncatedLabel = new QLabel(this);
    truncatedLabel->setWordWrap(true);
    truncatedLabel->setText(newName);

    QVBoxLayout *cutOffLayout = new QVBoxLayout(truncationWidget);
    cutOffLayout->addWidget(truncatedLabel);
    cutOffLayout->addStretch();
    cutOffLayout->addLayout(pageThreebottomLayout);
    truncationWidget->setLayout(cutOffLayout);
    stack->addWidget(truncationWidget);
    stack->setCurrentWidget(pageOne);

    connect(cancelButton, &QPushButton::clicked, this, &KyFileDialogRename::reject);
    connect(okButton, &QPushButton::clicked, this, [=, &error]{
        int id = truncationGroup->checkedId();
        error.respValue.insert("cateType", id);
        if (Qt::CheckState::Checked == allApp->checkState()) {
            error.respCode = Peony::ExceptionResponse::TruncateAll;
        } else {
            error.respCode = Peony::ExceptionResponse::TruncateOne;
        }
        accept();
    });

    connect(truncationGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, [=](QAbstractButton* button) {
        const int id = truncationGroup->id(button);
        const QString destName = truncateDestFileName(error.destDirUri, id);
        truncatedLabel->setText(destName);
    });
    Q_EMIT truncationGroup->buttonClicked(laterTruncation);

    m_currentWidget = nullptr;
    connect(onePageGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, [=](QAbstractButton* button) {
        const int id = onePageGroup->id(button);
        switch(id) {
        case 0:
            allApp->show();
            specificationLabel->setText(tr("Description: Skip copying files of the current type"));
            break;
        case 1:{
            allApp->show();
            QString clickText = "<a href=\" \" style=\"color: #3D6BE5;text-decoration: none;\">"
                  + tr("truncate interval")
                  + "</a>" + tr(".");
            QString text = tr("Explanation: Truncate the portion of the file name that exceeds 225 bytes and select");
            text = text + clickText;
            specificationLabel->setText(text);
            m_currentWidget = truncationWidget;
            break;
        }
        case 2:{
            allApp->show();
            QString downloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
            QString path = QString(tr("Description: By default, save to \"%1/扩展\".")).arg(downloadPath);
            specificationLabel->setText(path);
            break;
        }
        case 3:{
            allApp->hide();
            QString clickText = "<a href=\"  \" style=\"color: #3D6BE5;text-decoration: none;\">"
                  + tr("modify the name")
                  + "</a>" + tr(".");
            QString text = tr("Explanation: When renaming a file name, ensure it is within 255 bytes and ");
            text = text + clickText;
            specificationLabel->setText(text);
            m_currentWidget = page2;
            break;
        }
        };
    });
    Q_EMIT onePageGroup->buttonClicked(skip);

    connect(specificationLabel, &QLabel::linkActivated, this, [=]() {
        const int id = onePageGroup->checkedId();
        if (1 == id) {
            QRect textRect = fontMetrics().boundingRect(truncatedLabel->text());
            int len = textRect.width()/(this->width() - stack->contentsMargins().left() - stack->contentsMargins().right());
            int totalHeight = qApp->fontMetrics().lineSpacing() *(len+1) + qApp->fontMetrics().descent();
            totalHeight = totalHeight + 150 > this->height() ? totalHeight + 150 : this->height();
            this->setFixedHeight(totalHeight);
        } else if (3 == id) {
            textEdit->setFocus();
        }
        stack->setCurrentWidget(m_currentWidget);
    });

    if (QGSettings::isSchemaInstalled("org.lingmo.style")) {
        QGSettings *settings = new QGSettings("org.lingmo.style", QByteArray(), this);
        connect(settings, &QGSettings::changed, this, [=](const QString &key) {
            if("iconThemeName" == key){
                labelIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(24, 24));
                //renameIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(24, 24));
            } else if ("systemFont" == key || "systemFontSize" == key) {
                if (0 != stack->currentIndex()) {
                    QFontMetrics font = qApp->fontMetrics();
                    QRect textRect = fontMetrics().boundingRect(truncatedLabel->text());
                    int len = textRect.width()/(this->width() - stack->contentsMargins().left() - stack->contentsMargins().right());
                    int totalHeight = qApp->fontMetrics().lineSpacing() *(len+1) + qApp->fontMetrics().descent();
                    totalHeight = totalHeight + 150 > this->height() ? totalHeight + 150 : this->height();
                    this->setFixedHeight(totalHeight);
                }
//                int height = font.lineSpacing() * 3 + font.descent();
//                textEdit->setFixedHeight(height);
//                textEdit->setTextCursor(cursor);

//                QStringList messages;
//                QString line;
//                line = qApp->fontMetrics().elidedText(line1, Qt::ElideMiddle, 500);
//                messages.append(line);
//                line = qApp->fontMetrics().elidedText(line2, Qt::ElideMiddle, 500);
//                messages.append(line);
//                line = qApp->fontMetrics().elidedText(line3, Qt::ElideMiddle, 500);
//                messages.append(line);
//                auto labelText = messages.join('\n');
                content->setText(labelText);
                this->repaint();
            }
        });
    }

    connect(cancelBt, &QPushButton::clicked, this, &KyFileDialogRename::reject);
    connect(applyBt, &QPushButton::clicked, this, [=, &error]{
        const int id = onePageGroup->checkedId();
        switch(id) {
        case 0:{
            if (Qt::CheckState::Checked == allApp->checkState()) {
                error.respCode = Peony::ExceptionResponse::IgnoreAll;
            } else {
                error.respCode = Peony::ExceptionResponse::IgnoreOne;
            }
            accept();
            break;
        }
        case 1:{
            QRect textRect = fontMetrics().boundingRect(truncatedLabel->text());
            int len = textRect.width()/(this->width() - stack->contentsMargins().left() - stack->contentsMargins().right());
            int totalHeight = qApp->fontMetrics().lineSpacing() *(len+1) + qApp->fontMetrics().descent();
            totalHeight = totalHeight + 150 > this->height() ? totalHeight + 150 : this->height();
            this->setFixedHeight(totalHeight);
            stack->setCurrentWidget(truncationWidget);
            break;
        }
        case 2:{
            if (Qt::CheckState::Checked == allApp->checkState()) {
                error.respCode = Peony::ExceptionResponse::SaveAll;
            } else {
                error.respCode = Peony::ExceptionResponse::SaveOne;
            }
            accept();
            break;
        }
        case 3:
            stack->setCurrentWidget(page2);
            break;
        };
    });
    connect(cancel2, &QPushButton::clicked, this, &KyFileDialogRename::reject);
    connect(ensure2, &QPushButton::clicked, this, [=, &error]{
        error.respValue.insert("newName", textEdit->toPlainText());
        error.respCode = Peony::ExceptionResponse::RenameOne;
        accept();
    });

    connect(textEdit, &RenameEditor::returnPressed, ensure2, &QPushButton::click);

    if (!exec()) {
        error.respCode = responseCode;
    }

}

const QString KyFileDialogRename::truncateDestFileName(const QString &uri, const int cateType)
{
    QFileInfo file(uri);
    auto newName = Peony::FileUtils::getNonSuffixedBaseNameFromUri(uri);
    QString baseName = Peony::FileUtils::getUriBaseName(uri);
    baseName = Peony::FileUtils::urlDecode(baseName);
    auto destDirUri = Peony::FileUtils::getParentUri(uri);
    auto fsType = Peony::FileUtils::getFsTypeFromFile(uri);
    bool setLimitBytes = true;
    if (fsType.contains("ntfs")) {
        setLimitBytes = false;
    }
    auto suffix = baseName;
    suffix = suffix.remove(newName);
    newName.remove(suffix);

    QString truncatedText;
    QString destName;
    if (setLimitBytes) {
        bool useForceChop = false;
        if (suffix.toLocal8Bit().count() > PEONY_TRUNCATE_NAME_LIMIT) {
            qWarning()<<"suffix too long:"<<uri<<"use force chop instead";
            useForceChop = true;
        } else if (newName == baseName) {
            qWarning()<<"failed to truncate suffix of"<<uri<<", use force chop instead";
            useForceChop = true;
        } else if (file.isDir()) {
            useForceChop = true;
        }
        if (useForceChop) {
            newName = baseName;
            int len = newName.length();
            if (Peony::TurnCateType::Post == cateType) {
                while (newName.toLocal8Bit().count() > PEONY_TRUNCATE_NAME_LIMIT) {
                    newName.chop(1);
                }
                truncatedText = baseName.right(len - newName.length());
                destName = newName + "<s>" + truncatedText + "</s>";
            } else if (Peony::TurnCateType::Front == cateType) {
                while (newName.toLocal8Bit().count() > PEONY_TRUNCATE_NAME_LIMIT) {
                    newName.remove(0,1);
                }
                truncatedText = baseName.left(len - newName.length());
                destName = "<s>" + truncatedText + "</s>" + newName;
            }
        } else {
            QString tmp = newName;
            int len = tmp.length();
            int limitBytes = PEONY_TRUNCATE_NAME_LIMIT - suffix.toLocal8Bit().count();
            if (Peony::TurnCateType::Post == cateType) {
                while (newName.toLocal8Bit().count() > limitBytes) {
                    newName.chop(1);
                }
                truncatedText = tmp.right(len - newName.length());
                destName = newName + "<s>" + truncatedText + "</s>";
            } else if (Peony::TurnCateType::Front == cateType){
                while (newName.toLocal8Bit().count() > limitBytes) {
                    newName.remove(0,1);
                }
                truncatedText = tmp.left(len - newName.length());
                destName = "<s>" + truncatedText + "</s>" + newName;
            }
            destName = destName + suffix;
        }
    } else {
        bool useForceChop = false;
        if (suffix.length() > PEONY_TRUNCATE_NAME_LIMIT) {
            qWarning()<<"suffix too long:"<<uri<<"use force chop instead";
            useForceChop = true;
        } else if (newName == baseName) {
            qWarning()<<"failed to truncate suffix of"<<uri<<", use force chop instead";
            useForceChop = true;
        } else if (file.isDir()) {
            useForceChop = true;
        }
        if (useForceChop) {
            newName = baseName;
            while (newName.length() > PEONY_TRUNCATE_NAME_LIMIT) {
                newName.chop(1);
            }
            int len = baseName.length();
            truncatedText = baseName.right(len - newName.length());
            destName = newName + "<s>" + truncatedText + "</s>";
        } else {
            QString tmp = newName;
            int limitBytes = PEONY_TRUNCATE_NAME_LIMIT - suffix.length();
            while (newName.length() > limitBytes) {
                newName.chop(1);
            }
            int len = tmp.length();
            truncatedText = tmp.right(len - newName.length());
            destName = newName + "<s>" + truncatedText + "</s>";
            destName = destName + suffix;
        }
    }
    return destName;
}

#endif
