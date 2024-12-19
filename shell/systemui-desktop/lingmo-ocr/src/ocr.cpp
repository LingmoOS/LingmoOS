#include "ocr.h"
#include "view/imageview.h"

#include <QtCore/QVariant>
#include <QTextBlock>
#include <QStandardPaths>
#include <QFileDialog>
#include <QFileInfo>
#include <QThread>
#include <QMutexLocker>
#include <QSplitter>
#include <QTimer>
#include <QShortcut>
#include <QDebug>
#include <QFontMetrics>
#include <QFont>

#define App (static_cast<QApplication*>(QCoreApplication::instance()))
Ocr::Ocr(QWidget *parent) :
    QObject(parent)
{
    setupUi();
    initScaleLabel();
    setupConnect();
}

Ocr::~Ocr()
{
    //程序即将结束,线程标志结束
    m_isEndThread = 0;

}

void Ocr::setupUi()
{


}

QString Ocr::imgName() const
{
    return m_imgPath;
}

QString Ocr::ocrText() const
{
    return m_result;
}

void Ocr::setupConnect()
{
    connect(this, &Ocr::sigResult, this, [ = ](const QString & result) {
        emit ocrTextChanged();
        deleteLoadingUi();
    });
}

void Ocr::retranslateUi(QWidget *Widget)
{
    //m_tiplabel->setText(QApplication::translate("Widget", "Tips: The clearer the image is, the more accurate the text is", nullptr));
}

void Ocr::createLoadingUi()
{
    m_isLoading = true;
//    m_loadingWidget = new TextLoadWidget(this);
//    m_loadingWidget->resize(40, 40);
//    m_loadingWidget->show();
//    m_loadingTip = new QLabel(tr("Recognizing"), this);
//    m_loadingTip->show();

    //识别时
//    if (m_copyBtn) {
//        m_copyBtn->setEnabled(false);
//    }
//    if (m_exportBtn) {
//        m_exportBtn->setEnabled(false);
//    }
}

void Ocr::deleteLoadingUi()
{
    m_isLoading = false;
//    if (m_loadingWidget) {
//        m_loadingWidget->deleteLater();
//        m_loadingWidget = nullptr;
//    }
//    if (m_loadingTip) {
//        m_loadingTip->deleteLater();
//        m_loadingTip = nullptr;
//    }
//    m_imageview->setForegroundBrush(QColor(0, 0, 0, 0)); //设置场景的前景色，类似于遮罩
}

//void Ocr::loadingUi()
//{
//    if (m_loadingWidget && m_loadingTip && m_frameStack) {
//        int x = this->width() - m_frameStack->width() / 2;
//        int y = this->height() / 2 - 50;
//        qDebug() << m_loadingWidget->width();
//        m_loadingWidget->setFixedSize(QSize(24, 24));
//        m_loadingWidget->move(x - m_loadingWidget->width() / 2, y);
//        qDebug() << m_loadingTip->width();
////        m_loadingTip->setAlignment(Qt::AlignCenter);
////        QFontMetrics fm(DFontSizeManager::instance()->get(DFontSizeManager::T8));
//        QFontMetrics fm(QFont(tr("Recognizing")));
//        int mPoint = fm.boundingRect(tr("Recognizing")).width();
//        m_loadingTip->move(x - mPoint / 2 - 4, y + 24);
//    }
//    if (m_pwidget) {
//        m_pwidget->setFixedSize(this->width(), this->height() - 48);
//        m_pwidget->move(0, 0);
//    }
//}

bool Ocr::openImage(const QString &path)
{
    qDebug()<<"Open "+path;
    bool bRet = false;
    QString m_path;
    m_path = path;
    if(path.startsWith("file://"))
    {
        m_path.remove(0,6);
    }
    QImage img(m_path);
    if (!img.isNull()) {
        m_imgName = m_path;
        if(!path.startsWith("file://"))
        {
            m_imgPath = "file://" + m_path;
        }
        emit imgNameChanged();
        openImage(img, m_imgName);
        bRet = true;
    }

    return bRet;
}

void Ocr::openImage(const QImage &img, const QString &name)
{
    createLoadingUi();
    //新打开的窗口需要设置属性
    m_imgName = name;
    m_imgPath = "file://" + name;
    emit imgNameChanged();
    if (m_currentImg) {
        delete m_currentImg;
        m_currentImg = nullptr;
    }
    m_currentImg = new QImage(img);
    if (!m_loadImagethread) {
        m_loadImagethread = QThread::create([ = ]() {
            QMutexLocker locker(&m_mutex);
            m_result = PaddleOCRApp::instance()->getRecogitionResult(*m_currentImg);
            //判断程序是否退出
            if (1 == m_isEndThread) {
                emit sigResult(m_result);
            }
        });
    }
    connect(m_loadImagethread, &QThread::finished, m_loadImagethread, [=]
    {
        m_loadImagethread->deleteLater();
        m_loadImagethread=nullptr;
    });
    m_loadImagethread->start();
}

void Ocr::loadHtml(const QString &html)
{
    if (!html.isEmpty()) {
//        m_frameStackLayout->setContentsMargins(20, 0, 5, 0);
//        m_resultWidget->setCurrentWidget(m_plainTextEdit);
//        m_plainTextEdit->appendHtml(html);
//        m_plainTextEdit->setHtml(html);
    } else {
        resultEmpty();
    }
}

void Ocr::loadString(const QString &string)
{
    if (!string.isEmpty()) {
//        m_frameStackLayout->setContentsMargins(20, 0, 5, 0);
//        m_resultWidget->setCurrentWidget(m_plainTextEdit);
//        m_plainTextEdit->appendPlainText(string);
//        m_plainTextEdit->setText(string);
        //读取完了显示在最上方
//        m_plainTextEdit->moveCursor(QTextCursor::Start) ;
//        m_plainTextEdit->ensureCursorVisible() ;
        //新增识别完成按钮恢复
//        if (m_copyBtn) {
//            m_copyBtn->setEnabled(true);
//        }
//        if (m_exportBtn) {
//            m_exportBtn->setEnabled(true);
//        }
    } else {
        resultEmpty();
    }
}

void Ocr::resultEmpty()
{
    //修复未识别到文字没有居中对齐的问题
//    m_frameStackLayout->setContentsMargins(20, 0, 20, 0);
//    m_resultWidget->setCurrentWidget(m_noResult);
    //新增如果未识别到，按钮置灰
//    if (m_copyBtn) {
//        m_copyBtn->setEnabled(false);
//    }
//    if (m_exportBtn) {
//        m_exportBtn->setEnabled(false);
//    }
}

void Ocr::initScaleLabel()
{
//    QHBoxLayout *layout = new QHBoxLayout();
//    QLabel *label = new QLabel();
//    layout->addWidget(label);

//    label->setAlignment(Qt::AlignCenter);

//    label->setText("100%");

//    QTimer *hideT = new QTimer(this);
//    hideT->setSingleShot(true);

//    connect(m_imageview, &ImageView::scaled, this, [ = ](qreal perc) {
//        label->setText(QString("%1%").arg(int(perc)));
//    });
}

void Ocr::copy()
{
//    //选中内容则复制，未选中内容则不复制
//    if (!m_plainTextEdit->textCursor().selectedText().isEmpty()) {
//        m_plainTextEdit->copy();
//    } else {
//        QTextDocument *document = m_plainTextEdit->document();
//        QPlainTextEdit *tempTextEdit = new QPlainTextEdit(this);
//        tempTextEdit->setDocument(document);
//        tempTextEdit->selectAll();
//        tempTextEdit->copy();
//        tempTextEdit->deleteLater();
//        tempTextEdit = nullptr;
//    }
      QClipboard* clip = QApplication::clipboard();
      clip->setText(m_result);

//    QIcon icon(":/assets/icon_toast_sucess_new.svg");

}

void Ocr::slotExport()
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

//    QString file_path = QFileDialog::getSaveFileName(this, "save as", download + "/" + fileName, "*.txt");
//    qDebug() << file_path;

    //这里不应该增加一个.txt，会导致有两个后缀
//    QString path = file_path;
//    QFile file(path);
//    if (file.open(QFile::WriteOnly | QFile::Text)) {
//        QTextStream out(&file);
//        out << m_plainTextEdit->document()->toPlainText();
//    }
}

void Ocr::setTextType(const QString &text)
{
    if (text == "简体中文")
    {
         PaddleOCRApp::instance()->setLanguages(PaddleOCRApp::CHI_SIM);
    }
    else if (text == "繁体中文"){
         PaddleOCRApp::instance()->setLanguages(PaddleOCRApp::CHI_TRA);
    }
    else if (text == "English"){
         PaddleOCRApp::instance()->setLanguages(PaddleOCRApp::ENG);
    }
    openImage(m_imgName);
}

