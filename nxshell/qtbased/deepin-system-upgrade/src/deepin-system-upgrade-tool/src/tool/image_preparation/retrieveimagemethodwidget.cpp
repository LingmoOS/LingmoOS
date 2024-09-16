// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DFileDialog>
#include <DGuiApplicationHelper>
#include <DPaletteHelper>

#include <QFileInfo>
#include <QFontMetrics>
#include <QGraphicsOpacityEffect>
#include <QMimeData>
#include <QPixmap>
#include <QButtonGroup>
#include <QSize>
#include <QStandardPaths>
#include <QStyle>

#include <iostream>

#include "../mainwindow.h"
#include "retrieveimagemethodwidget.h"
#include "../application.h"
#include "../../core/utils.h"
#include "retrieveimagemethodwidget.h"

#define BUTTON_W 268
#define BUTTON_H 222
#define DROPAREA_UNSELECTED_OPACITY 0.4

RetrieveImageMethodWidget::RetrieveImageMethodWidget(QWidget *parent)
    : QWidget(parent)
    , m_label(new DLabel(tr("Get Image Files"), this))
    , m_verticalLayout(new QVBoxLayout(this))
    , m_networkRadioButton(new QRadioButton(this))
    , m_localRadioButton(new QRadioButton(this))
    , m_networkPushButton(new DPushButton(this))
    , m_localPushButton(new DPushButton(this))
    , m_buttonsHBoxLayout(new QHBoxLayout(this))
    , m_darkBtnStylesheet(loadStyleSheet(":/qss/dark-custombtn.qss"))
    , m_lightBtnStylesheet(loadStyleSheet(":/qss/light-custombtn.qss"))
    , m_fileLabelSpacerItem(new QSpacerItem(0, 0))

{
    setAcceptDrops(true);
    initUI();
    initConnections();
    m_networkRadioButton->setChecked(true);
}

void RetrieveImageMethodWidget::initUI()
{
    QButtonGroup *radioButtonGroup = new QButtonGroup(this);

    m_verticalLayout->setContentsMargins(82, 0, 82, 0);
    m_verticalLayout->addSpacing(35);
    m_label->setForegroundRole(DPalette::TextTitle);
    DFontSizeManager::instance()->bind(m_label, DFontSizeManager::T3, QFont::DemiBold);
    m_verticalLayout->addWidget(m_label, 0, Qt::AlignCenter);
    m_verticalLayout->addSpacing(40);

    if (isDarkMode())
    {
        m_networkPushButton->setStyleSheet(m_darkBtnStylesheet);
        m_localPushButton->setStyleSheet(m_darkBtnStylesheet);
    }
    else
    {
        m_networkPushButton->setStyleSheet(m_lightBtnStylesheet);
        m_localPushButton->setStyleSheet(m_lightBtnStylesheet);
    }

    // Download from Internet button
    {
        QVBoxLayout *baseLayout = new QVBoxLayout(this);

        m_networkRadioButton->setText(tr("Download from Internet"));
        DFontSizeManager::instance()->bind(m_networkRadioButton, DFontSizeManager::T6, QFont::DemiBold);

        QLabel *networkIcon = new QLabel(this);
        networkIcon->setPixmap(QIcon(":/icons/internet_download.svg").pixmap(120, 120));

        baseLayout->addWidget(m_networkRadioButton);
        baseLayout->addWidget(networkIcon, 0, Qt::AlignCenter);

        m_networkPushButton->setFixedSize(BUTTON_W, BUTTON_H);
        m_networkPushButton->setLayout(baseLayout);
        m_buttonsHBoxLayout->addWidget(m_networkPushButton);

        radioButtonGroup->addButton(m_networkRadioButton);
    }

    // Import local image button
    {
        QVBoxLayout *baseLayout = new QVBoxLayout(this);
        QHBoxLayout *textLayout = new QHBoxLayout(this);
        m_fileTextLabel = new BaseLabel(this);
        m_dropAreaIconLabel = new IconLabel(this, Orientation::Vertical);
        m_dropAreaFrame = new DropFrame(this);
        m_fileTextLabel->setText(tr("Import local image files"));
        m_fileTextLabel->m_label->setForegroundRole(DPalette::ButtonText);
        DFontSizeManager::instance()->bind(m_fileTextLabel->m_label, DFontSizeManager::T6, QFont::DemiBold);
        m_fileTextLabel->setTip(tr("Format: ISO"));
        updateTipsColor();

        textLayout->addWidget(m_localRadioButton, 0, Qt::AlignTop);
        textLayout->addWidget(m_fileTextLabel, 0, Qt::AlignLeft | Qt::AlignTop);
        textLayout->setSpacing(0);
        textLayout->addSpacing(114);

        baseLayout->addLayout(textLayout);

        // Add drag and drop widget
        setDefaultDropFrameStyle();
        m_dropAreaFrame->setFixedSize(248, 150);

        m_removeFileButton = new DPushButton(this);
        m_removeFileButton->setIcon(style()->standardIcon(QStyle::SP_LineEditClearButton));
        m_removeFileButton->setIconSize(QSize(25, 25));
        m_removeFileButton->setVisible(false);
        m_removeFileButton->setFixedSize(25, 25);
        m_removeFileButton->setStyleSheet("background-color: transparent;");
        m_dropAreaIconLabel->m_layout->insertWidget(0, m_removeFileButton, 0, Qt::AlignRight);

        m_dropAreaIconLabel->setIcon(":/icons/local_directory.svg");
        m_filenameLabel = new DLabel(tr("Drag files here"));
        DFontSizeManager::instance()->bind(m_filenameLabel, DFontSizeManager::T8, QFont::Thin);
        // m_filenameLabel->setForegroundRole(DPalette::TextTips);
        m_dropAreaIconLabel->addSpacerItem(m_fileLabelSpacerItem);
        m_dropAreaIconLabel->addWidget(m_filenameLabel);
        m_uploadTextLink = new QLabel(QString("<a href=\"/fuck\" style=\"text-decoration: none;\">%1</a>").arg(tr("Select file")));
        DFontSizeManager::instance()->bind(m_uploadTextLink, DFontSizeManager::T8, QFont::Medium);
        m_dropAreaIconLabel->addWidget(m_uploadTextLink);
        m_dropAreaIconLabel->m_layout->addSpacerItem(new QSpacerItem(0, 28, QSizePolicy::Minimum, QSizePolicy::Maximum));

        m_dropAreaFrame->setLayout(m_dropAreaIconLabel->layout());

        // Setting opacity for dropAreaFrame
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(this);
        effect->setOpacity(DROPAREA_UNSELECTED_OPACITY);
        m_dropAreaFrame->setGraphicsEffect(effect);

        baseLayout->addWidget(m_dropAreaFrame);

        // localPushButton->setStyleSheet("QPushButton {background-color: #08000000; }");
        m_localPushButton->setFixedSize(BUTTON_W, BUTTON_H);
        m_localPushButton->setLayout(baseLayout);

        m_buttonsHBoxLayout->addWidget(m_localPushButton);
        radioButtonGroup->addButton(m_localRadioButton);
    }
    m_verticalLayout->addLayout(m_buttonsHBoxLayout);
    m_verticalLayout->addSpacing(75);

    setLayout(m_verticalLayout);
}

void RetrieveImageMethodWidget::initConnections()
{
    // Setting actions for two image retriving button.
    connect(m_networkPushButton, &QPushButton::clicked, this, [this] {
        m_networkRadioButton->setChecked(!m_networkRadioButton->isChecked());
    });
    connect(m_localPushButton, &QPushButton::clicked, this, [this] {
        m_localRadioButton->setChecked(!m_localRadioButton->isChecked());
    });
    connect(m_localRadioButton, &QRadioButton::toggled, this, &RetrieveImageMethodWidget::ToggleLocalButton);
    connect(m_localRadioButton, &QRadioButton::toggled, [this] {
        // Setting opacity for dropAreaFrame
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(this);
        if (m_localRadioButton->isChecked())
        {
            effect->setOpacity(1);
        }
        else
        {
            effect->setOpacity(DROPAREA_UNSELECTED_OPACITY);
        }
        m_dropAreaFrame->setGraphicsEffect(effect);
    });

    // Setting UI changes for dragging files to the drop area.
    connect(m_dropAreaFrame, &DropFrame::fileAboutAccept, this, [this]() {
        m_dropAreaFrame->setPenColor(QColor("#FF0081FF"));
        m_dropAreaFrame->setBrush(QBrush(QColor("#0D10A5FF")));
        m_dropAreaFrame->setProperty("active", true);
        m_dropAreaFrame->update();
    });
    connect(m_dropAreaFrame, &DropFrame::fileCancel, this, &RetrieveImageMethodWidget::setDefaultDropFrameStyle);

    // Light/Dark themes switching
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, [this] (DGuiApplicationHelper::ColorType themeType) {
        if (themeType == DGuiApplicationHelper::DarkType)
        {
            m_networkPushButton->setStyleSheet(m_darkBtnStylesheet);
            m_localPushButton->setStyleSheet(m_darkBtnStylesheet);

            // Since the original style is broken, setting text color manually instead.
            m_fileTextLabel->setTextColor(QColor("#FFC0C6D4"));
            updateTipsColor();
        }
        else
        {
            m_networkPushButton->setStyleSheet(m_lightBtnStylesheet);
            m_localPushButton->setStyleSheet(m_lightBtnStylesheet);

            m_fileTextLabel->setTextColor(QColor("#FF414D68"));
            updateTipsColor();
        }
        // Change the color of drop area.
        setDefaultDropFrameStyle();
    });

    // Implement methods to add images
    connect(m_dropAreaFrame, &DropFrame::fileDrop, this, &RetrieveImageMethodWidget::onFileSelected);
    connect(m_uploadTextLink, &QLabel::linkActivated, this, [ this ](const QString & /*link*/) {
        // Avoid keeping "import from local" button pressed down.
        m_localPushButton->setDown(false);

        DFileDialog fileDlg(this);
        fileDlg.setViewMode(DFileDialog::Detail);
        fileDlg.setFileMode(DFileDialog::ExistingFile);
        fileDlg.setDirectory(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
        fileDlg.setNameFilter("ISO (*.iso)");
        fileDlg.selectNameFilter("ISO (*.iso)");
        if (DFileDialog::Accepted == fileDlg.exec()) {
            QString text = fileDlg.selectedFiles().first();
            onFileSelected(text);
        }
    });

    // Clear local image signals
    connect(m_removeFileButton, &QPushButton::clicked, this, &RetrieveImageMethodWidget::FileClear);
    connect(this, &RetrieveImageMethodWidget::FileClear, this, &RetrieveImageMethodWidget::onFileCleared);
}

void RetrieveImageMethodWidget::onFileSelected(const QString &filename)
{
    QFileInfo info(filename);
    QFontMetrics fontMetrics(m_filenameLabel->font());
    m_fileUrl = filename;
    QString elidedFilename = fontMetrics.elidedText(info.fileName(), Qt::ElideRight, m_dropAreaFrame->width() - 40);
    m_filenameLabel->setText(elidedFilename);
    m_dropAreaIconLabel->setIcon(":/icons/file_added.svg");
    m_dropAreaIconLabel->scale(80, 80);
    m_uploadTextLink->setVisible(false);
    m_localRadioButton->setChecked(true);
    m_removeFileButton->setVisible(true);

    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(this);
    effect->setOpacity(1.0);
    m_dropAreaFrame->setGraphicsEffect(effect);

    // It is possible that theme switching can happen after adding image files.
    setDefaultDropFrameStyle();
    m_dropAreaFrame->update();

    emit FileAdded();
}

void RetrieveImageMethodWidget::onFileCleared()
{
    m_fileUrl = "";
    m_removeFileButton->setVisible(false);
    m_dropAreaIconLabel->setIcon(":/icons/local_directory.svg");
    m_filenameLabel->setText(tr("Drag files here"));
    m_fileLabelSpacerItem->changeSize(0, 0);
    m_uploadTextLink->setVisible(true);
    setDefaultDropFrameStyle();
}

void RetrieveImageMethodWidget::setDefaultDropFrameStyle()
{
    m_dropAreaFrame->setPenStyle(Qt::DashLine);

    QColor penColor, brushColor;
    qreal opacity;
    if (isDarkMode())
    {
        penColor = QColor("#78D6D6D6");
        brushColor = QColor("#00000000");

        if (m_localRadioButton->isChecked())
        {
            // File Added
            opacity = 1.0;
        }
        else
        {
            opacity = DROPAREA_UNSELECTED_OPACITY;
        }
    }
    else
    {
        brushColor = QColor("#CCFFFFFF");
        if (m_localRadioButton->isChecked())
        {
            penColor = QColor("#99D6D6D6");
            opacity = 1.0;
        }
        else
        {
            penColor = QColor("#99D6D6D6");
            opacity = DROPAREA_UNSELECTED_OPACITY;
        }
    }

    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(this);
    effect->setOpacity(opacity);
    m_dropAreaFrame->setGraphicsEffect(effect);
    m_dropAreaFrame->setPenColor(penColor);
    m_dropAreaFrame->setBrush(QBrush(brushColor));
    m_dropAreaFrame->update();
    m_dropAreaFrame->setProperty("active", true);
}

QString RetrieveImageMethodWidget::loadStyleSheet(const QString &styleFile)
{
    QString styleSheet;
    QFile file(styleFile);
    if (file.open(QFile::ReadOnly))
    {
        styleSheet = QLatin1String(file.readAll());
        file.close();
    }
    return styleSheet;
}

void RetrieveImageMethodWidget::updateTipsColor()
{
    if (!m_fileTextLabel)
        return;

    DPalette pa = DPaletteHelper::instance()->palette(this);
    m_fileTextLabel->setTipColor(pa.color( DPalette::TextTips));

    if (!m_filenameLabel)
        return;

    QPalette palette = m_filenameLabel->palette();
    palette.setColor(m_filenameLabel->foregroundRole(), pa.color(DPalette::TextTips));
    m_filenameLabel->setPalette(palette);

}
