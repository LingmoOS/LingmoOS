#include "uploadfilewidget.h"
#include "unzipwoker.h"

#include "../type_defines.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QToolButton>
#include <QStackedWidget>
#include <QFileDialog>
#include <QPushButton>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

#include <net/helper/transferhepler.h>

UploadFileWidget::UploadFileWidget(QWidget *parent)
    : QFrame(parent)
{
    initUI();
}

UploadFileWidget::~UploadFileWidget()
{
}

void UploadFileWidget::initUI()
{
    setStyleSheet(".UploadFileWidget{background-color: white; border-radius: 10px;}");

    QVBoxLayout *mainLayout = new QVBoxLayout();
    setLayout(mainLayout);
    mainLayout->setSpacing(0);
    mainLayout->addSpacing(30);

    QLabel *titileLabel = new QLabel(tr("Select data transfer file"), this);
    titileLabel->setFixedHeight(50);
    StyleHelper::setAutoFont(titileLabel, 24, QFont::DemiBold);
    titileLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    uploadFileFrame = new UploadFileFrame(this);
    QHBoxLayout *uploadLayout = new QHBoxLayout();
    uploadLayout->addWidget(uploadFileFrame, Qt::AlignCenter);

    tipLabel = new QLabel(tr("File error, cannot transfer, please reselect"), this);
    tipLabel->setStyleSheet(StyleHelper::textStyle(StyleHelper::error) + "background-color: rgba(0, 0, 0, 0);border-style: none;");
    tipLabel->setFixedHeight(20);
    tipLabel->setAlignment(Qt::AlignCenter);
    tipLabel->setVisible(false);
    StyleHelper::setAutoFont(tipLabel, 12, QFont::Normal);

    QHBoxLayout *tipLayout = new QHBoxLayout();
    tipLayout->addWidget(tipLabel);
    tipLayout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);

    ButtonLayout *buttonLayout = new ButtonLayout();
    backButton = buttonLayout->getButton1();
    backButton->setText(tr("Back"));
    nextButton = buttonLayout->getButton2();
    nextButton->setText(tr("Next"));
    nextButton->setEnabled(false);

    connect(backButton, &QPushButton::clicked, this, &UploadFileWidget::backPage);
    connect(nextButton, &QPushButton::clicked, this, [this]() {
        if (nextButton->text() == tr("Retry")) {
            emit uploadFileFrame->updateUI(uploadStatus::Initial);
            tipLabel->setVisible(false);
            return;
        }
        if (!checkBackupFile(uploadFileFrame->getZipFilePath())) {
            tipLabel->setVisible(true);
            nextButton->setText(tr("Retry"));
            return;
        }
        UnzipWorker *woker = new UnzipWorker(uploadFileFrame->getZipFilePath());
        woker->start();
        nextPage();
    });

    connect(this, &UploadFileWidget::Initial, this, [this]() {
        emit uploadFileFrame->updateUI(uploadStatus::Initial);
        return;
    });

    IndexLabel *indelabel = new IndexLabel(1, this);

    QHBoxLayout *indexLayout = new QHBoxLayout();
    indexLayout->addWidget(indelabel, Qt::AlignCenter | Qt::AlignBottom);

    mainLayout->addWidget(titileLabel);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(uploadLayout);
    mainLayout->addSpacing(100);
    mainLayout->addLayout(tipLayout);
    mainLayout->addSpacing(5);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(indexLayout);

    connect(uploadFileFrame, &UploadFileFrame::updateUI, this, [this, indelabel](int status) {
        tipLabel->setVisible(false);
        if (status == uploadStatus::valid) {
            nextButton->setEnabled(true);
            nextButton->setText(tr("Next"));
            indelabel->setIndex(2);
        } else {
            indelabel->setIndex(1);
            nextButton->setEnabled(false);
            nextButton->setText(tr("Next"));
        }
    });
}

bool UploadFileWidget::checkBackupFile(const QString &filePath)
{
    //Verify effectiveness
    if (!UnzipWorker::isValid(filePath)) {
        tipLabel->setText(tr("The file is corrupted and cannot be migrated. Please replace it with a backup file."));
        tipLabel->setVisible(true);
        return false;
    }

    // Verify size
    QFileInfo info(filePath);
    LOG << "checkBackupFile .size();" << info.size();
    int size = static_cast<int>(info.size() / 1024 / 1024 / 1024) * 2;
//    if (size >= TransferHelper::getRemainSize()) {
//        tipLabel->setVisible(true);
//        tipLabel->setText(tr("Insufficient space on UOS. Please reserve at least %1G of space and try again.").arg(size));
//        return false;
//    }
    return true;
}

void UploadFileWidget::clear()
{
    emit Initial();
}

void UploadFileWidget::nextPage()
{
    QStackedWidget *stackedWidget = qobject_cast<QStackedWidget *>(this->parent());
    if (stackedWidget) {
        stackedWidget->setCurrentIndex(PageName::transferringwidget);
    } else {
        WLOG << "Jump to next page failed, qobject_cast<QStackedWidget *>(this->parent()) = nullptr";
    }
}

void UploadFileWidget::backPage()
{
    QStackedWidget *stackedWidget = qobject_cast<QStackedWidget *>(this->parent());
    if (stackedWidget) {
        stackedWidget->setCurrentIndex(PageName::choosewidget);
    } else {
        WLOG << "Jump to next page failed, qobject_cast<QStackedWidget *>(this->parent()) = nullptr";
    }
}

void UploadFileWidget::themeChanged(int theme)
{
    //light
    if (theme == 1) {
        setStyleSheet(".UploadFileWidget{background-color: white; border-radius: 10px;}");
        uploadFileFrame->themeChanged(1);
    } else {
        //dark
        setStyleSheet(".UploadFileWidget{background-color: rgba(37, 37, 37,1); border-radius: 10px;}");
        uploadFileFrame->themeChanged(0);
    }
}

UploadFileFrame::UploadFileFrame(QWidget *parent)
    : QFrame(parent)
{
    initUI();
}

UploadFileFrame::~UploadFileFrame()
{
}

void UploadFileFrame::initUI()
{
    setFixedSize(480, 275);
    setAcceptDrops(true);

    QLabel *iconLabel = new QLabel(this);
    iconLabel->setPixmap(QIcon(":/icon/zip-64.svg").pixmap(64, 64));
    iconLabel->setStyleSheet(".QLabel{background-color: rgba(0, 0, 0, 0);border-style: none;}");
    iconLabel->setAlignment(Qt::AlignCenter);

    QLabel *textLabel = new QLabel(QString("<font color='gray' >%1</font>").arg(tr("Drag file here ")), this);
    textLabel->setStyleSheet("background-color: rgba(0, 0, 0, 0);border-style: none;");
    textLabel->setFixedHeight(20);
    textLabel->setAlignment(Qt::AlignCenter);

    QString display = QString("<a href=\"https://\" style=\"text-decoration:none;\">%1</a>").arg(tr("Import file"));
    QLabel *displayLabel = new QLabel(display, this);
    displayLabel->setStyleSheet(".QLabel{background-color: rgba(0, 0, 0, 0);border-style: none;}");
    displayLabel->setAlignment(Qt::AlignCenter);
    displayLabel->setFixedHeight(20);

    StyleHelper::setAutoFont(textLabel, 12, QFont::Normal);
    StyleHelper::setAutoFont(displayLabel, 12, QFont::Normal);

    connect(displayLabel, &QLabel::linkActivated, this, &UploadFileFrame::uploadFile);

    closeBtn = new QPushButton(qobject_cast<QWidget *>(this->parent()));
    closeBtn->setIcon(QIcon(":/icon/tab_close_normal.svg"));
    closeBtn->setIcon(QIcon::fromTheme("tab_close_normal"));
    closeBtn->setWindowOpacity(1.0);
    closeBtn->setStyleSheet("background-color: rgba(0, 0, 0, 0);border-style: none;");
    closeBtn->setIconSize(QSize(35, 35));
    closeBtn->setGeometry(405, 170, 35, 35);
    closeBtn->setVisible(false);

    QLabel *WarningIconLabel = new QLabel(iconLabel);
    WarningIconLabel->setPixmap(QIcon(":/icon/warning.svg").pixmap(30, 30));
    WarningIconLabel->setStyleSheet(".QLabel{background-color: rgba(0, 0, 0, 0);border-style: none;}");
    WarningIconLabel->setGeometry(230, 60, 30, 30);
    WarningIconLabel->setVisible(false);

    initFileFrame();
    QHBoxLayout *fileHLayout = new QHBoxLayout(fileFrame);
    fileHLayout->addWidget(fileFrame);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    setLayout(mainLayout);
    mainLayout->setSpacing(0);
    mainLayout->addSpacing(50);
    mainLayout->addWidget(iconLabel);
    mainLayout->addWidget(textLabel);
    mainLayout->addSpacing(5);
    mainLayout->addWidget(displayLabel);
    mainLayout->addLayout(fileHLayout);
    mainLayout->addSpacing(70);

    connect(closeBtn, &QPushButton::clicked, this, [this] {
        emit updateUI(uploadStatus::Initial);
    });

    connect(this, &UploadFileFrame::updateUI, this, [WarningIconLabel, this, iconLabel, textLabel, displayLabel](int status) {
        switch (status) {
        case uploadStatus::Initial: {
            fileFrame->setVisible(false);
            closeBtn->setVisible(false);
            WarningIconLabel->setVisible(false);
            iconLabel->setVisible(true);
            textLabel->setVisible(true);
            displayLabel->setVisible(true);
            textLabel->setText(QString("<font color='gray' >%1</font>").arg(tr("Drag file here ")));
            displayLabel->setText(QString("<a href=\"https://\" style=\"text-decoration:none;\">%1</a>").arg(tr("Import file")));
            break;
        }
        case uploadStatus::valid: {
            fileFrame->setVisible(true);
            closeBtn->setVisible(true);
            WarningIconLabel->setVisible(false);
            iconLabel->setVisible(false);
            textLabel->setVisible(false);
            displayLabel->setVisible(false);
            break;
        }
        case uploadStatus::formaterror: {
            fileFrame->setVisible(false);
            closeBtn->setVisible(false);
            WarningIconLabel->setVisible(true);
            iconLabel->setVisible(true);
            textLabel->setVisible(true);
            displayLabel->setVisible(true);
            textLabel->setText(QString("<font color='gray' >%1</font>").arg(tr("Only .zip is supported, please")));
            displayLabel->setText(QString("<a href=\"https://\" style=\"text-decoration:none;\">%1</a>").arg(tr("reselect")));
            break;
        }
        default:
            break;
        }
    });
}

void UploadFileFrame::themeChanged(int theme)
{
    //light
    lightTheme = theme;
    if (theme == 1) {
        setStyleSheet(".UploadFileFrame{background-color: rgba(0, 0, 0, 0.03);"
                      "border-radius: 10px;"
                      "border-style: dashed;"
                      "border-width: 2px;"
                      "border-color: rgba(0, 0, 0, 0.06);}");
    } else {
        //dark
        setStyleSheet(".UploadFileFrame{"
                      "background-color: rgba(255,255,255, 0.05);"
                      "border-radius: 10px;"
                      "border-style: dashed;"
                      "border-width: 2px;"
                      "border-color: rgba(255,255,255, 0.1);}");
    }
    auto closeIcon = lightTheme ? QIcon(":/icon/light/tab_close_normal.svg") : QIcon(":/icon/dark/tab_close_normal.svg");
    closeBtn->setIcon(closeIcon);
    QString color = lightTheme ? "background-color: rgba(0, 0, 0, 0.1);"
                               : "background-color: rgba(255,255,255, 0.1);";
    closeBtn->setGeometry(405, lightTheme ? 165 : 170, 35, 35);
    fileFrame->setStyleSheet(".QFrame{" + color + "border-style: none; border-radius: 10px;}");
}

void UploadFileFrame::initFileFrame()
{
    fileFrame = new QFrame(this);
    fileFrame->setStyleSheet(".QFrame{background-color: rgba(230, 230, 230, 0.1);border-style: none; border-radius: 10px;}");
    fileFrame->setFixedSize(124, 111);
    fileFrame->setVisible(false);

    QLabel *iconLabel = new QLabel(this);
    iconLabel->setPixmap(QIcon(":/icon/application-x-zip.svg").pixmap(64, 64));
    iconLabel->setStyleSheet("background-color: rgba(0, 0, 0, 0);border-style: none;");
    iconLabel->setAlignment(Qt::AlignCenter);

    QLabel *textLabel = new QLabel(this);
    textLabel->setStyleSheet("background-color: rgba(0, 0, 0, 0);border-style: none;");
    textLabel->setFixedHeight(20);
    textLabel->setAlignment(Qt::AlignCenter);

    QVBoxLayout *fileFrameLayout = new QVBoxLayout();
    fileFrameLayout->addWidget(iconLabel);
    fileFrameLayout->addWidget(textLabel);
    fileFrame->setLayout(fileFrameLayout);

    connect(this, &UploadFileFrame::updateUI, this, [this, textLabel](int status) {
        if (status == uploadStatus::valid) {
            QFileInfo info(zipFilePath);
            textLabel->setText("<font color='gray' >" + info.fileName() + " </font>");
            StyleHelper::setAutoFont(textLabel, 12, QFont::Normal);
        }
    });
}

QString UploadFileFrame::getZipFilePath() const
{
    return zipFilePath;
}

void UploadFileFrame::uploadFile()
{
    zipFilePath = QFileDialog::getOpenFileName(nullptr, tr("select zip file"), "", tr("ZIP file (*.zip)"));
    LOG << "set zipFilePath =" + zipFilePath.toStdString();
    if (!zipFilePath.isEmpty())
        emit updateUI(uploadStatus::valid);
}

void UploadFileFrame::dragEnterEvent(QDragEnterEvent *event)
{
    event->accept();
}

void UploadFileFrame::dragMoveEvent(QDragMoveEvent *event)
{
    QString bordercolor = lightTheme ? "border-color: rgba(0, 0, 0, 0.06);}"
                                     : "border-color: rgba(255,255,255, 0.1);}";
    setStyleSheet(".UploadFileFrame{background-color: rgba(0, 129, 255, 0.2);"
                  "border-radius: 10px;"
                  "border-style: dashed;"
                  "border-width: 2px;"
                  + bordercolor);
    QString color = lightTheme ? "background-color: rgba(0, 0, 0, 0.1);"
                               : "background-color: rgba(255,255,255, 0.1);";
    fileFrame->setStyleSheet(".QFrame{" + color + "border-style: none; border-radius: 10px;}");
    event->accept();
}

void UploadFileFrame::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event);
    themeChanged(lightTheme);
}

void UploadFileFrame::dropEvent(QDropEvent *event)
{
    themeChanged(lightTheme);
    const QList<QUrl> &urls = event->mimeData()->urls();
    if (urls.size() != 1)
        return;
    QUrl url = urls.first();
    QFileInfo info(url.url());
    if (info.suffix() != "zip") {
        emit updateUI(uploadStatus::formaterror);
        return;
    } else {
        zipFilePath = url.path();
        LOG << "set zipFilePath =" + zipFilePath.toStdString();
        if (!zipFilePath.isEmpty())
            emit updateUI(uploadStatus::valid);
    }
}
