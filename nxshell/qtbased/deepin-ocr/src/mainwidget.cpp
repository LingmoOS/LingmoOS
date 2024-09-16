// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwidget.h"
#include "view/imageview.h"
#include "loadingwidget.h"
#include "frame.h"

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QTextBlock>
#include <QStandardPaths>
#include <QFileDialog>
#include <QFileInfo>
#include <QThread>
#include <QMutexLocker>
#include <QSplitter>
#include <QTimer>
#include <QShortcut>
#include <QPushButton>

#include <DGuiApplicationHelper>
#include <DMainWindow>
#include <DTitlebar>
#include <DMessageManager>
#include <DToolButton>
#include <DFloatingWidget>
#include <DAnchors>
#include <DFontSizeManager>
#include <DHiDPIHelper>
#include <DDciIcon>
#include <QSettings>
#include <DStandardPaths>

#define App (static_cast<QApplication*>(QCoreApplication::instance()))
MainWidget::MainWidget(QWidget *parent) :
    DWidget(parent)
{
    setupUi(this);
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    setIcons(themeType);
    initScaleLabel();
    setupConnect();
}

MainWidget::~MainWidget()
{
    //程序即将结束,线程标志结束
    m_isEndThread = 0;
}

void MainWidget::setupUi(QWidget *Widget)
{
    m_mainGridLayout = new QGridLayout(Widget);
    m_mainGridLayout->setSpacing(0);
    m_mainGridLayout->setContentsMargins(0, 0, 0, 6);
    m_mainGridLayout->setObjectName(QStringLiteral("gridLayout"));

    m_horizontalLayout = new QHBoxLayout(Widget);
    m_horizontalLayout->setSpacing(0);
    m_horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));

    m_plainTextEdit = new ResultTextView(Widget);
    m_plainTextEdit->setLineWidth(0);

    m_plainTextEdit->setObjectName(QStringLiteral("plainTextEdit"));

    m_frame = new QFrame(this);
    m_frame->setObjectName(QStringLiteral("m_frame"));
    m_frame->setFrameShape(QFrame::StyledPanel);
    m_frame->setFrameShadow(QFrame::Raised);
    m_frame->setLineWidth(0);
    m_frameLayout = new QHBoxLayout(m_frame);
    m_frameLayout->setSpacing(6);
    m_frameLayout->setContentsMargins(11, 11, 11, 11);
    m_frameLayout->setObjectName(QStringLiteral("horizontalLayout"));
    m_frameLayout->setContentsMargins(10, 10, 10, 10);

    //后面加一层窗口,左边空间间距20
    m_frameStack = new Frame(this);
    m_frameStack->setFrameShape(QFrame::StyledPanel);
    m_frameStack->setFrameShadow(QFrame::Raised);
    m_frameStack->setLineWidth(0);
    m_frameStackLayout = new QHBoxLayout(m_frameStack);
    m_frameStackLayout->setSpacing(0);
    m_frameStackLayout->setObjectName(QStringLiteral("horizontalLayoutStack"));
    m_frameStackLayout->setContentsMargins(20, 0, 5, 0);

    m_imageview = new ImageView(m_frame);
    m_imageview->setLineWidth(0);

    m_frameLayout->addWidget(m_imageview);

    m_resultWidget = new DStackedWidget(this);
    m_resultWidget->setFocusPolicy(Qt::NoFocus);

    m_frameStackLayout->addWidget(m_resultWidget);
    m_frameStack->setMinimumWidth(220);
    //宽度最大值为440
    m_frameStack->setMaximumWidth(440);

    connect(m_frameStack, &Frame::sigFrameResize, this, [ = ] {
        m_resultWidget->resize(QSize(m_frameStack->width() - 25, m_frameStack->height()));
    });


    m_loadingOcr = new loadingWidget(this);
    m_loadingOcr->setAlignment(Qt::AlignCenter);
    m_resultWidget->addWidget(m_loadingOcr);

    m_resultWidget->addWidget(m_plainTextEdit);

    m_noResult = new DLabel(this);
    m_noResult->setAlignment(Qt::AlignCenter);
    m_noResult->setText(tr("No text recognized"));
    m_resultWidget->addWidget(m_noResult);

    connect(m_loadingOcr, &loadingWidget::sigChangeSize, [ = ] {
        loadingUi();
    });
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal); //新建水平分割器
    mainSplitter->setHandleWidth(0);//分割线的宽度
    mainSplitter->setChildrenCollapsible(false);//不允许把分割出的子窗口拖小到0，最小值被限定为sizeHint或maxSize/minSize
    mainSplitter->addWidget(m_frame);//把ui中拖出的各个控件拿走，放到分割器里面
    mainSplitter->addWidget(m_frameStack);
    QList<int> list;
    //默认右侧识栏目是220的宽度
    list << 620 << 220;
    mainSplitter->setSizes(list);
    m_horizontalLayout->addWidget(mainSplitter);
    m_horizontalLayout->setStretch(0, 1);

    m_mainGridLayout->addLayout(m_horizontalLayout, 0, 0, 1, 1);
    m_mainGridLayout->setColumnStretch(0, 1);

    m_buttonHorizontalLayout = new QHBoxLayout(Widget);
    m_buttonHorizontalLayout->setContentsMargins(20, 3, 20, 3); //表示控件与窗体的左右边距
    m_buttonHorizontalLayout->setSpacing(10);

    m_tipHorizontalLayout = new QHBoxLayout(Widget);

    m_tipIconLabel = new DLabel(Widget);
    m_tipIconLabel->setObjectName(QStringLiteral("tipIconLabel"));

    m_tipHorizontalLayout->addWidget(m_tipIconLabel);

    m_tiplabel = new DLabel(Widget);
    m_tiplabel->setObjectName(QStringLiteral("tiplabel"));

    m_tipHorizontalLayout->setSpacing(4);
    m_tipHorizontalLayout->addWidget(m_tiplabel);

    m_buttonHorizontalLayout->addLayout(m_tipHorizontalLayout);
    QSpacerItem *horizontalSpacer = new QSpacerItem(159, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_buttonHorizontalLayout->addItem(horizontalSpacer);

    //配置文件读写
    ocrSetting = new QSettings(Dtk::Core::DStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config.conf", QSettings::IniFormat);

    //语种读写设置
    //目前仅支持默认插件，默认插件支持的语种字符串：zh-Hans_en，zh-Hant_en，en
    auto currentLanguage = ocrSetting->value("language", "zh-Hans_en").toString();
    OCREngine::instance()->setLanguage(currentLanguage);

    //设置语种选择框
    auto recLabel = new DLabel(tr("Recognize language"));
    languageSelectBox = new DComboBox(this);
    languageSelectBox->setFixedSize(160, 36);
    languageSelectBox->addItems({tr("Simplified Chinese"), tr("English"), tr("Traditional Chinese")});
    static std::map<QString, int> languageIndexMap{ {"zh-Hans_en", 0},
                                                    {"en", 1},
                                                    {"zh-Hant_en", 2}
                                                  };
    languageSelectBox->setCurrentIndex(languageIndexMap[currentLanguage]);
    connect(languageSelectBox, static_cast<void(DComboBox::*)(int)>(&DComboBox::currentIndexChanged), [this](int index) {
        QString resultLanguage;
        switch(index) {
        default:
            resultLanguage = "zh-Hans_en";
            break;
        case 0:
            resultLanguage = "zh-Hans_en";
            break;
        case 1:
            resultLanguage = "en";
            break;
        case 2:
            resultLanguage = "zh-Hant_en";
            break;
        };
        if(!OCREngine::instance()->setLanguage(resultLanguage)) {
            return;
        }
        ocrSetting->setValue("language", resultLanguage);
        runRec(false);
    });

    m_buttonHorizontalLayout->addWidget(recLabel, 0, Qt::AlignRight);
    m_buttonHorizontalLayout->addWidget(languageSelectBox, 0, Qt::AlignRight);

    m_copyBtn = new DIconButton(Widget);
    m_copyBtn->setObjectName(QStringLiteral("Copy text"));
    m_copyBtn->setMaximumSize(QSize(36, 36));
    m_copyBtn->setToolTip(tr("Copy text"));

    m_buttonHorizontalLayout->addWidget(m_copyBtn, 0, Qt::AlignRight);
    m_exportBtn = new DIconButton(Widget);
    m_exportBtn->setObjectName(QStringLiteral("Save as TXT"));
    m_exportBtn->setMaximumSize(QSize(36, 36));
    m_exportBtn->setToolTip(tr("Save as TXT"));

    m_buttonHorizontalLayout->addWidget(m_exportBtn, 0, Qt::AlignRight);

    m_line = new DHorizontalLine(this);
    m_line->setObjectName(QStringLiteral("m_line"));

    m_mainGridLayout->addWidget(m_line, 1, 0, 1, 1);
    m_mainGridLayout->addLayout(m_buttonHorizontalLayout, 2, 0, 1, 1, Qt::AlignRight);

    retranslateUi(Widget);

    QMetaObject::connectSlotsByName(Widget);
    m_pwidget = new QWidget(this);
    m_pwidget->setFocusPolicy(Qt::NoFocus);
    m_pwidget->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_pwidget->setFixedSize(this->width(), this->height() - 23);
    m_pwidget->move(0, 0);

    DMainWindow *mainWindow = static_cast<DMainWindow *>(this->parent());
    if (mainWindow) {
        mainWindow->titlebar()->setMenuVisible(false);
    }

    //撤销
    auto undoBtn = new DIconButton;
    undoBtn->setToolTip(tr("Undo (Ctrl+Z)"));
    undoBtn->setIcon(DDciIcon(QString(":/assets/icon_return.dci")));
    undoBtn->setMaximumSize(36, 36);
    undoBtn->setIconSize(QSize(14, 14));
    mainWindow->titlebar()->addWidget(undoBtn, Qt::AlignRight);
    connect(undoBtn, &QPushButton::clicked, m_plainTextEdit, &QPlainTextEdit::undo);
    connect(m_plainTextEdit, &QPlainTextEdit::undoAvailable, undoBtn, &QPushButton::setEnabled);
    undoBtn->setEnabled(false);

    //重做
    auto redoBtn = new DIconButton;
    redoBtn->setToolTip(tr("Redo (Ctrl+Shift+Z)"));
    redoBtn->setIcon(DDciIcon(QString(":/assets/icon_advance.dci")));
    redoBtn->setMaximumSize(36, 36);
    redoBtn->setIconSize(QSize(14, 14));
    mainWindow->titlebar()->addWidget(redoBtn, Qt::AlignRight);
    connect(redoBtn, &QPushButton::clicked, m_plainTextEdit, &QPlainTextEdit::redo);
    connect(m_plainTextEdit, &QPlainTextEdit::redoAvailable, redoBtn, &QPushButton::setEnabled);
    redoBtn->setEnabled(false);

    //占位用空白控件
    m_emptyWidget = new QWidget;
    m_emptyWidget->setMinimumSize(36 * 2, 36 * 2);
    mainWindow->titlebar()->addWidget(m_emptyWidget, Qt::AlignRight);
}

void MainWidget::setupConnect()
{
    initShortcut();
    //增加适配运行时的风格切换
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &MainWidget::setIcons);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::paletteTypeChanged, this, &MainWidget::setIcons);
    connect(m_exportBtn, &DIconButton::clicked, this, &MainWidget::slotExport);
    connect(m_copyBtn, &DIconButton::clicked, this, &MainWidget::slotCopy);
    connect(this, &MainWidget::sigResult, this, [ = ](const QString & result) {
        loadString(result);
        deleteLoadingUi();
        if(m_needReRunRec) {
            m_needReRunRec = false;
            runRec(false);
        }
    });
}

void MainWidget::retranslateUi(QWidget *Widget)
{
//    Widget->setWindowTitle("Widget");
    m_tiplabel->setText(QApplication::translate("Widget", "Tips: The clearer the image is, the more accurate the text is", nullptr));
//    m_copyBtn->setText(QApplication::translate("Widget", "Copy", nullptr));
    //    m_exportBtn->setText(QApplication::translate("Widget", "Export", nullptr));
}

void MainWidget::createLoadingUi()
{
    m_isLoading = true;
    m_loadingWidget = new TextLoadWidget(this);
    m_loadingWidget->resize(40, 40);
    m_loadingWidget->show();
    m_loadingTip = new DLabel(tr("Recognizing"), this);
    m_loadingTip->show();
//    if (m_imageview) {
//        m_imageview->setForegroundBrush(QColor(0, 0, 0, 77)); //设置场景的前景色，类似于遮罩
//    }
    //识别时
    if (m_copyBtn) {
        m_copyBtn->setEnabled(false);
    }
    if (m_exportBtn) {
        m_exportBtn->setEnabled(false);
    }
}

void MainWidget::deleteLoadingUi()
{
    m_isLoading = false;
    if (m_loadingWidget) {
        m_loadingWidget->deleteLater();
        m_loadingWidget = nullptr;
    }
    if (m_loadingTip) {
        m_loadingTip->deleteLater();
        m_loadingTip = nullptr;
    }
    m_imageview->setForegroundBrush(QColor(0, 0, 0, 0)); //设置场景的前景色，类似于遮罩
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    setIcons(themeType);
}

void MainWidget::loadingUi()
{
    if (m_loadingWidget && m_loadingTip && m_frameStack) {
        int x = this->width() - m_frameStack->width() / 2;
        int y = this->height() / 2 - 50;
        qDebug() << m_loadingWidget->width();
        m_loadingWidget->setFixedSize(QSize(24, 24));
        m_loadingWidget->move(x - m_loadingWidget->width() / 2, y);
        qDebug() << m_loadingTip->width();
//        m_loadingTip->setAlignment(Qt::AlignCenter);
        QFontMetrics fm(DFontSizeManager::instance()->get(DFontSizeManager::T8));
        int mPoint = fm.boundingRect(tr("Recognizing")).width();
        m_loadingTip->move(x - mPoint / 2 - 4, y + 24);
    }
    if (m_pwidget) {
        m_pwidget->setFixedSize(this->width(), this->height() - 48);
        m_pwidget->move(0, 0);
    }

    //对齐撤销和重做按钮
    m_emptyWidget->setFixedWidth(this->width() - 150 - m_imageview->width() - 20);
}

void MainWidget::initShortcut()
{
    m_scAddView = new QShortcut(QKeySequence("Ctrl+="), this);
    m_scAddView->setContext(Qt::WindowShortcut);
    connect(m_scAddView, &QShortcut::activated, this, [ = ] {
        if (m_imageview)
        {
            m_imageview->setScaleValue(1.1);
        }
    });

    m_scReduceView = new QShortcut(QKeySequence("Ctrl+-"), this);
    m_scReduceView->setContext(Qt::WindowShortcut);
    connect(m_scReduceView, &QShortcut::activated, this, [ = ] {
        if (m_imageview)
        {
            m_imageview->setScaleValue(0.9);
        }
    });

}

bool MainWidget::openImage(const QString &path)
{
    bool bRet = false;
    if (m_imageview) {
        QImage img(path);
        if (!img.isNull()) {
            m_imgName = path;
            openImage(img, m_imgName);
            bRet = true;
        }
    }
    return bRet;
}

void MainWidget::openImage(const QImage &img, const QString &name)
{
    //新打开的窗口需要设置属性
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    setIcons(themeType);
    if (m_imageview) {
        m_imageview->openFilterImage(img);
        QTimer::singleShot(100, [ = ] {
            //分辨率大于window的采用适应窗口，没超过，则适应图片
            QRect rect1 = m_imageview->image().rect();
            if ((rect1.width() >= m_imageview->width() || rect1.height() >= m_imageview->height() - 150) && m_imageview->width() > 0 &&
                    height() > 0)
            {
                m_imageview->fitWindow();
            } else
            {
                m_imageview->fitImage();
            }
        });
        m_imgName = name;
    }
    if (m_currentImg) {
        delete m_currentImg;
        m_currentImg = nullptr;
    }
    m_currentImg = new QImage(img);
    runRec(true);
}

void MainWidget::runRec(bool needSetImage)
{
    if(m_loadImagethread && m_loadImagethread->isRunning()) {
        m_needReRunRec = true;
        return;
    }

    createLoadingUi();
    m_plainTextEdit->clear();
    if(needSetImage) {
        OCREngine::instance()->setImage(*m_currentImg);
    }
    if (!m_loadImagethread) {
        m_loadImagethread = QThread::create([ = ]() {
            m_result = OCREngine::instance()->getRecogitionResult();
            //判断程序是否退出
            if (1 == m_isEndThread) {
                emit sigResult(m_result);
            }
        });
    }
    connect(m_loadImagethread, &QThread::finished, [this](){
        m_loadImagethread->deleteLater();
        m_loadImagethread = nullptr;
    });
    m_loadImagethread->start();
}

void MainWidget::loadHtml(const QString &html)
{
    if (!html.isEmpty()) {
        m_plainTextEdit->setUndoRedoEnabled(false);
        m_frameStackLayout->setContentsMargins(20, 0, 5, 0);
        m_resultWidget->setCurrentWidget(m_plainTextEdit);
        m_plainTextEdit->appendHtml(html);
//        m_plainTextEdit->setHtml(html);
        m_plainTextEdit->setUndoRedoEnabled(true);
    } else {
        resultEmpty();
    }
}

void MainWidget::loadString(const QString &string)
{
    if (!string.isEmpty()) {
        m_plainTextEdit->setUndoRedoEnabled(false);
        m_frameStackLayout->setContentsMargins(20, 0, 5, 0);
        m_resultWidget->setCurrentWidget(m_plainTextEdit);
        m_plainTextEdit->appendPlainText(string);
//        m_plainTextEdit->setText(string);
        //读取完了显示在最上方
        m_plainTextEdit->moveCursor(QTextCursor::Start) ;
        m_plainTextEdit->ensureCursorVisible();
        m_plainTextEdit->setUndoRedoEnabled(true);
        //新增识别完成按钮恢复
        if (m_copyBtn) {
            m_copyBtn->setEnabled(true);
        }
        if (m_exportBtn) {
            m_exportBtn->setEnabled(true);
        }
    } else {
        resultEmpty();
    }
}

void MainWidget::resultEmpty()
{
    if(m_needReRunRec) {
        return;
    }

    //修复未识别到文字没有居中对齐的问题
    m_frameStackLayout->setContentsMargins(20, 0, 20, 0);
    m_resultWidget->setCurrentWidget(m_noResult);
    //新增如果未识别到，按钮置灰
    if (m_copyBtn) {
        m_copyBtn->setEnabled(false);
    }
    if (m_exportBtn) {
        m_exportBtn->setEnabled(false);
    }
}

void MainWidget::initScaleLabel()
{
    DAnchors<DFloatingWidget> scalePerc = new DFloatingWidget(this);
    scalePerc->setBlurBackgroundEnabled(true);

    QHBoxLayout *layout = new QHBoxLayout();
    scalePerc->setLayout(layout);
    DLabel *label = new DLabel();
    layout->addWidget(label);
    scalePerc->setAttribute(Qt::WA_TransparentForMouseEvents);
    scalePerc.setAnchor(Qt::AnchorHorizontalCenter, this, Qt::AnchorHorizontalCenter);
    scalePerc.setAnchor(Qt::AnchorBottom, this, Qt::AnchorBottom);
    scalePerc.setBottomMargin(75 + 14);
    label->setAlignment(Qt::AlignCenter);
//    scalePerc->setFixedSize(82, 48);
    scalePerc->setFixedWidth(90 + 10);
    scalePerc->setFixedHeight(40 + 10);
    scalePerc->adjustSize();
    label->setText("100%");
    DFontSizeManager::instance()->bind(label, DFontSizeManager::T6);
    scalePerc->hide();

    QTimer *hideT = new QTimer(this);
    hideT->setSingleShot(true);
    connect(hideT, &QTimer::timeout, scalePerc, &DLabel::hide);

    connect(m_imageview, &ImageView::scaled, this, [ = ](qreal perc) {
        label->setText(QString("%1%").arg(int(perc)));
    });
    connect(m_imageview, &ImageView::showScaleLabel, this, [ = ]() {
        scalePerc->show();
        scalePerc->move(m_imageview->width() / 2 - 50, m_imageview->height() - 48);
        hideT->start(1000);
    });
}

void MainWidget::resizeEvent(QResizeEvent *event)
{
    return DWidget::resizeEvent(event);
}

void MainWidget::paintEvent(QPaintEvent *event)
{
    loadingUi();

    return DWidget::paintEvent(event);
}


void MainWidget::slotCopy()
{
    //选中内容则复制，未选中内容则不复制
    if (!m_plainTextEdit->textCursor().selectedText().isEmpty()) {
        m_plainTextEdit->copy();
    } else {
        QTextDocument *document = m_plainTextEdit->document();
        QPlainTextEdit *tempTextEdit = new QPlainTextEdit(this);
        tempTextEdit->setDocument(document);
        tempTextEdit->selectAll();
        tempTextEdit->copy();
        tempTextEdit->deleteLater();
        tempTextEdit = nullptr;
    }

    QIcon icon(":/assets/icon_toast_sucess_new.svg");
    DFloatingMessage *pDFloatingMessage = new DFloatingMessage(DFloatingMessage::MessageType::TransientType, m_pwidget);
    pDFloatingMessage->setBlurBackgroundEnabled(true);
    pDFloatingMessage->setMessage(tr("Copied"));
    pDFloatingMessage->setIcon(icon);
    pDFloatingMessage->raise();
    DMessageManager::instance()->sendMessage(m_pwidget, pDFloatingMessage);

}

void MainWidget::slotExport()
{
    QStringList list = QStandardPaths::standardLocations(QStandardPaths::DownloadLocation);
    QString download = "";
    if (list.size() > 0) {
        download = list.at(0);
    } else {
        QStringList home = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
        if (home.size() > 0) {
            download = home.at(0) + "/Downloads";
        }
    }

    QString fileName;
    if (!m_imgName.isEmpty()) {
        fileName = QFileInfo(m_imgName).completeBaseName();
    } else {
        fileName = "Results";
    }

    QString file_path = QFileDialog::getSaveFileName(this, "save as", download + "/" + fileName, "*.txt");
    qDebug() << file_path;

    //这里不应该增加一个.txt，会导致有两个后缀
    QString path = file_path;
    QFile file(path);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream out(&file);
        out << m_plainTextEdit->document()->toPlainText();
    }
}

void MainWidget::setIcons(DGuiApplicationHelper::ColorType themeType)
{
    //可以直接用DCI图标的部分
    if (m_copyBtn) {
        m_copyBtn->setIcon(DDciIcon(QString(":/assets/icon_copy.dci")));
        m_copyBtn->setIconSize(QSize(14, 14));
    }
    if (m_exportBtn) {
        m_exportBtn->setIcon(DDciIcon(QString(":/assets/icon_download.dci")));
        m_exportBtn->setIconSize(QSize(14, 14));
    }

    // 手动设置，防止被后续更新冲掉调色版
    if (languageSelectBox) {
        QPalette originPal;
        originPal.setBrush(QPalette::Window, originPal.window());
        languageSelectBox->setPalette(originPal);
    }

    //无法使用DCI图标的部分
    if (themeType == DGuiApplicationHelper::DarkType) {
        QPalette pal;
        pal.setColor(QPalette::Window, QColor(32, 32, 32, 179));
        setAutoFillBackground(true);
        setPalette(pal);
        if (m_resultWidget) {
            QPalette pal;
            pal.setColor(QPalette::Window, QColor(40, 40, 40));
            m_resultWidget->setAutoFillBackground(true);
            m_resultWidget->setPalette(pal);
            //增加frame的颜色设置
            m_frameStack->setAutoFillBackground(true);
            m_frameStack->setPalette(pal);
        }
        if (m_tipIconLabel) {
            QPixmap m_tipImage = DDciIcon(QString(":/assets/icon_tips.dci")).pixmap(App->devicePixelRatio(), 16, DDciIcon::Theme::Dark);
            m_tipIconLabel->setPixmap(m_tipImage);
            m_tipIconLabel->setFixedSize(QSize(16, 16));
        }
        App->setWindowIcon(QIcon(":/assets/appicon_dark.svg"));
        DMainWindow *mainWindow = static_cast<DMainWindow *>(this->parent());
        if (mainWindow) {
            mainWindow->titlebar()->setIcon(QIcon(":/assets/appicon_dark.svg"));
        }
        if (m_isLoading && m_imageview) {
            m_imageview->setForegroundBrush(QColor(0, 0, 0, 127)); //设置场景的前景色，类似于遮罩
            m_imageview->setBackgroundBrush(QColor(34, 34, 34));
            QPalette pal;
            pal.setColor(QPalette::Window, QColor(0, 0, 0, 125));
            m_frame->setAutoFillBackground(true);
            m_frame->setPalette(pal);
        } else if (m_imageview) {
            m_imageview->setBackgroundBrush(QColor(35, 35, 35));
            QPalette pal;
            pal.setColor(QPalette::Window, QColor(35, 35, 35));
            m_frame->setAutoFillBackground(true);
            m_frame->setPalette(pal);
        }
        if (m_tiplabel) {
            QPalette pe;
            pe.setColor(QPalette::WindowText, QColor(109, 124, 136));
            m_tiplabel->setPalette(pe);
        }
        if (m_noResult) {
            QPalette pe;
            pe.setColor(QPalette::WindowText, QColor(192, 198, 212, 102));
            m_noResult->setPalette(pe);
        }
        if (m_loadingTip) {
            QPalette pe;
            pe.setColor(QPalette::WindowText, QColor(192, 198, 212));
            m_loadingTip->setPalette(pe);
        }
    } else {
        QPalette pal;
        pal.setColor(QPalette::Window, QColor(255, 255, 255, 179));
        setAutoFillBackground(true);
        setPalette(pal);
        //修复因为切换导致的颜色差
        if (m_resultWidget && m_frameStack) {
            QPalette pal;
            pal.setColor(QPalette::Window, QColor(255, 255, 255));
            m_resultWidget->setAutoFillBackground(true);
            m_resultWidget->setPalette(pal);
            //增加frame的颜色设置
            m_frameStack->setAutoFillBackground(true);
            m_frameStack->setPalette(pal);
        }
        if (m_tipIconLabel) {
            QPixmap m_tipImage = DDciIcon(QString(":/assets/icon_tips.dci")).pixmap(App->devicePixelRatio(), 16, DDciIcon::Theme::Light);
            m_tipIconLabel->setPixmap(m_tipImage);
            m_tipIconLabel->setFixedSize(QSize(16, 16));
        }
        App->setWindowIcon(QIcon(":/assets/appicon_light.svg"));
        DMainWindow *mainWindow = static_cast<DMainWindow *>(this->parent());
        if (mainWindow) {
            mainWindow->titlebar()->setIcon(QIcon(":/assets/appicon_light.svg"));
        }
        if (m_isLoading && m_imageview) {
            m_imageview->setForegroundBrush(QColor(0, 0, 0, 77)); //设置场景的前景色，类似于遮罩
            m_imageview->setBackgroundBrush(QColor(248, 248, 248));
            QPalette pal;
            pal.setColor(QPalette::Window, QColor(172, 172, 172));
            m_frame->setAutoFillBackground(true);
            m_frame->setPalette(pal);
        } else if (m_imageview) {
            //修改为正确的背景setBackgroundBrush
            m_imageview->setBackgroundBrush(QColor(248, 248, 248));
            QPalette pal;
            pal.setColor(QPalette::Window, QColor(248, 248, 248));
            m_frame->setAutoFillBackground(true);
            m_frame->setPalette(pal);
        }
        if (m_tiplabel) {
            QPalette pe;
            pe.setColor(QPalette::WindowText, QColor(138, 161, 180));
            m_tiplabel->setPalette(pe);
        }
        if (m_noResult) {
            QPalette pe;
            pe.setColor(QPalette::WindowText, QColor(85, 85, 85, 102));
            m_noResult->setPalette(pe);
        }
        if (m_loadingTip) {
            QPalette pe;
            pe.setColor(QPalette::WindowText, QColor(65, 77, 104));
            m_loadingTip->setPalette(pe);
        }
    }
}
