#include "transferringwidget.h"

#include "errorwidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QToolButton>
#include <QStackedWidget>
#include <QPropertyAnimation>
#include <QEventLoop>
#include <QPainterPath>
#include <QMovie>
#include <QStandardItemModel>

#include <net/helper/transferhepler.h>
#include <utils/optionsmanager.h>
#include <utils/transferutil.h>
#include <common/commonutils.h>

TransferringWidget::TransferringWidget(QWidget *parent)
    : QFrame(parent)
{
    initUI();
    initConnect();
}

TransferringWidget::~TransferringWidget() {}

void TransferringWidget::initUI()
{
    setStyleSheet(".TransferringWidget{background-color: white; border-radius: 10px;}");

    QVBoxLayout *mainLayout = new QVBoxLayout();
    setLayout(mainLayout);
    mainLayout->setSpacing(0);
    mainLayout->addSpacing(30);

    iconWidget = new MovieWidget("transferring", this);
    QHBoxLayout *iconLayout = new QHBoxLayout();
    iconLayout->addWidget(iconWidget, Qt::AlignCenter);

    titileLabel = new QLabel(tr("Transferring..."), this);
    titileLabel->setFixedHeight(50);
    StyleHelper::setAutoFont(titileLabel, 24, QFont::DemiBold);
    titileLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    progressLabel = new ProgressBarLabel(this);
    progressLabel->setAlignment(Qt::AlignCenter);
    progressLabel->setProgress(0);

    QHBoxLayout *progressLayout = new QHBoxLayout();
    progressLayout->addWidget(progressLabel, Qt::AlignCenter);

    timeLabel = new QLabel(this);
    timeLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    StyleHelper::setAutoFont(timeLabel, 12, QFont::Normal);
    timeLabel->setText(QString(tr("Calculationing...")));

    fileLabel = new QLabel(this);
    fileLabel->setAlignment(Qt::AlignCenter);

    QString display = QString("<a href=\"https://\" style=\"text-decoration:none;\">%1</a>")
                              .arg(tr("Show processes"));
    displayLabel = new QLabel(display, this);
    StyleHelper::setAutoFont(displayLabel, 12, QFont::Normal);
    displayLabel->setAlignment(Qt::AlignCenter);
    QObject::connect(displayLabel, &QLabel::linkActivated, this,
                     &TransferringWidget::updateInformationPage);

    IndexLabel *indelabel = new IndexLabel(3, this);
    indelabel->setAlignment(Qt::AlignCenter);

    QHBoxLayout *indexLayout = new QHBoxLayout();
    indexLayout->addWidget(indelabel, Qt::AlignCenter);

    fileNameFrame = new QFrame(this);
    fileNameFrame->setFixedSize(500, 250);

    processWindow = new ProcessWindow(this);

    QHBoxLayout *textBrowerlayout = new QHBoxLayout(fileNameFrame);
    fileNameFrame->setLayout(textBrowerlayout);
    textBrowerlayout->addWidget(processWindow);

    mainLayout->setAlignment(Qt::AlignHCenter);
    mainLayout->addLayout(iconLayout);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(titileLabel);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(progressLayout);
    mainLayout->addSpacing(7);
    mainLayout->addWidget(timeLabel);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(fileLabel);
    mainLayout->addWidget(displayLabel);
    mainLayout->addWidget(fileNameFrame);
    mainLayout->addSpacing(5);
    mainLayout->addLayout(indexLayout);
    fileNameFrame->setVisible(false);
}

void TransferringWidget::initConnect()
{
    connect(TransferHelper::instance(), &TransferHelper::transferContent, this,
            &TransferringWidget::updateProcess);
    connect(TransferHelper::instance(), &TransferHelper::disconnected, this,
            &TransferringWidget::clear);
}

void TransferringWidget::updateInformationPage()
{
    if (!isVisible) {
        isVisible = true;
        iconWidget->setVisible(false);
        fileLabel->setVisible(false);
        fileNameFrame->setVisible(true);

        QString display = QString("<a href=\"https://\" style=\"text-decoration:none;\">%1</a>")
                                  .arg(tr("Hide processes"));
        displayLabel->setText(display);
        QPropertyAnimation *showAnimation = new QPropertyAnimation(processWindow, "pos");
        showAnimation->setDuration(200);
        showAnimation->setStartValue(QPoint(0, 250));
        showAnimation->setEndValue(QPoint(0, 0));
        showAnimation->setEasingCurve(QEasingCurve::Linear);
        showAnimation->start();

    } else {
        isVisible = false;

        QString display = QString("<a href=\"https://\" style=\"text-decoration:none;\">%1</a>")
                                  .arg(tr("Show processes"));
        displayLabel->setText(display);

        QPropertyAnimation *hideAnimation = new QPropertyAnimation(processWindow, "pos");
        hideAnimation->setDuration(100);
        hideAnimation->setStartValue(QPoint(0, 0));
        hideAnimation->setEndValue(QPoint(0, 250));
        hideAnimation->setEasingCurve(QEasingCurve::Linear);

        QEventLoop loop;
        QObject::connect(hideAnimation, &QPropertyAnimation::finished, &loop, &QEventLoop::quit);
        hideAnimation->start();
        loop.exec();

        iconWidget->setVisible(true);
        fileLabel->setVisible(true);
        fileNameFrame->setVisible(false);
    }
}

void TransferringWidget::changeTimeLabel(const QString &time)
{
    timeLabel->setText(QString(tr("Transfer will be completed in %1 minutes")).arg(time));
}

void TransferringWidget::changeProgressLabel(const int &ratio)
{
    progressLabel->setProgress(ratio);
}

void TransferringWidget::updateProcess(const QString &tpye, const QString &content, int progressbar,
                                       int estimatedtime)
{
#if defined(_WIN32) || defined(_WIN64)
    if (OptionsManager::instance()->getUserOption(Options::kTransferMethod)[0]
        == TransferMethod::kLocalExport) {
        return;
    }
#else
    if (tpye == tr("Transfering") && content.contains("transfer.json"))
        TransferUtil::checkSize(content);
#endif

    //处理过程内容，只显示一级目录文件
    QString str = resetContent(tpye, content);

    if (!str.isEmpty()) {
        processWindow->updateContent(str, tpye);
        StyleHelper::setAutoFont(fileLabel, 12, QFont::Normal);
        fileLabel->setText(
                QString("<font>%1 %2<font style='color: rgba(0, 0, 0, 0.6);'>&nbsp;&nbsp;&nbsp;")
                        .arg(tpye, str));
    }

    if (estimatedtime == -1)
        return;

    progressLabel->setProgress(progressbar);

    timeLabel->setText(QString(tr("Calculationing...")));
    if (estimatedtime > 0) {
        titileLabel->setText(tr("Transferring..."));
        if (estimatedtime > 60)
            timeLabel->setText(QString(tr("Transfer will be completed in %1 minutes"))
                                       .arg(estimatedtime / 60));
        else
            timeLabel->setText(
                    QString(tr("Transfer will be completed in %1 secondes")).arg(estimatedtime));
    }
    if (estimatedtime == -2) {
        timeLabel->setText(QString(tr("Transfer will be completed in --")));
    }

#ifdef __linux__
    //通知对方进程情况
    QString mes = tpye + " " + content + " " + QString::number(progressbar) + " "
            + QString::number(estimatedtime) + ";";
    TransferHelper::instance()->sendMessage("transfer_content", mes);
#endif
}

void TransferringWidget::themeChanged(int theme)
{
    // light
    if (theme == 1) {
        setStyleSheet(".TransferringWidget{background-color: white; border-radius: 10px;}");
    } else {
        // dark
        setStyleSheet(
                ".TransferringWidget{background-color: rgb(37, 37, 37); border-radius: 10px;}");
    }
    processWindow->changeTheme(theme);
}

void TransferringWidget::clear()
{
    processWindow->clear();
    progressLabel->setProgress(0);
    timeLabel->setText(tr("Calculationing..."));
    titileLabel->setText(tr("Transferring..."));
    fileLabel->setText("");
    finishJobs.clear();
}

QString TransferringWidget::resetContent(const QString &type, const QString &content)
{
    if (!type.startsWith(tr("Decompressing")))
        return content;

    QString res = content;

    if (finishJobs.isEmpty()) {
        if (type.startsWith(tr("Transfering"))) {
            QStringList parts = content.split("/");
            if (parts.size() > 3)
                res = "/" + parts[1] + "/" + parts[2] + "/" + parts[3];
        }
        finishJobs.append(res);
        return QString();
    }

    res = getTransferFileName(content, finishJobs.first());
    if (finishJobs.contains(res))
        return QString();
    else
        finishJobs.append(res);
    return res;
}

QString TransferringWidget::getTransferFileName(const QString &fullPath, const QString &targetPath)
{
    std::string path = fullPath.toStdString();
    std::string toRemove = targetPath.toStdString();

    size_t found = path.find(toRemove);   // 查找子字符串的位置
    auto index = found + toRemove.length() + 1;
    if (found != std::string::npos && index <= path.length()) {   // 如果找到了子字符串
        std::string result = path.substr(index);   // 截取子字符串之后的部分
        found = result.find('/');   // 查找第一个路径名
        if (found != std::string::npos) {
            result = result.substr(0, found);   // 截取第一个路径名
        }
        return QString::fromStdString(result);
    } else {
        return QString();
    }
}

ProcessWindow::ProcessWindow(QFrame *parent)
    : ProcessDetailsWindow(parent)
{
    init();
}

ProcessWindow::~ProcessWindow()
{
}

void ProcessWindow::updateContent(const QString &name, const QString &type)
{
    int maxWith = 100;
    QString nameT = QFontMetrics(StyleHelper::font(3)).elidedText(name, Qt::ElideRight, maxWith);
    QString typeT = QFontMetrics(StyleHelper::font(3)).elidedText(type, Qt::ElideRight, maxWith);

    QStandardItemModel *model = qobject_cast<QStandardItemModel *>(this->model());
    int num;
    if (type == tr("Installing")) {
        num = 1;
    } else {
        num = 0;
    }

    for (int col = 0; col < model->columnCount(); ++col) {
        QModelIndex index = model->index(0, col);
        QString itemName = model->data(index, Qt::DisplayRole).toString();
        if (itemName == nameT) {
            model->setData(index, typeT, Qt::ToolTipRole);
            model->setData(index, num, Qt::UserRole);
            return;
        }
    }

    QStandardItem *item = new QStandardItem();
    item->setData(nameT, Qt::DisplayRole);
    item->setData(typeT, Qt::ToolTipRole);
    item->setData(num, Qt::UserRole);
    item->setData(0, Qt::StatusTipRole);
    model->appendRow(item);
}

void ProcessWindow::changeTheme(int theme)
{
    if (theme == 1) {
        setStyleSheet(".ProcessWindow{background-color: rgba(0, 0, 0, 0.08);"
                      "border-radius: 10px;"
                      "padding: 10px 30px 10px 10px;"
                      "}");
    } else {
        // dark
        setStyleSheet(".ProcessWindow{background-color: rgba(255,255,255, 0.08);"
                      "border-radius: 10px;"
                      "padding: 10px 30px 10px 10px;"
                      "}");
    }

    ProcessWindowItemDelegate *delegate = qobject_cast<ProcessWindowItemDelegate *>(this->itemDelegate());
    delegate->setTheme(theme);
}

void ProcessWindow::init()
{
    setStyleSheet(".ProcessWindow{background-color: rgba(0, 0, 0, 0.08);"
                  "border-radius: 10px;"
                  "padding: 10px 30px 10px 10px;"
                  "}");
    QStandardItemModel *model = new QStandardItemModel(this);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setModel(model);
    ProcessWindowItemDelegate *delegate = new ProcessWindowItemDelegate();
    delegate->addIcon(QString(":/icon/working.svg"));
    delegate->addIcon(QString(":/icon/workDone.svg"));
    setItemDelegate(delegate);
}

ProgressBarLabel::ProgressBarLabel(QWidget *parent)
    : QLabel(parent), m_progress(0)
{
    setFixedSize(280, 8);
}

void ProgressBarLabel::setProgress(int progress)
{
    m_progress = progress;
    update();
}

void ProgressBarLabel::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    // 绘制背景
    painter.setBrush(QColor(220, 220, 220));
    painter.drawRoundedRect(rect(), 5, 5);

    // 绘制进度条
    int width = static_cast<int>(rect().width() * (m_progress / 100.0));
    QRectF progressRect(rect().left(), rect().top(), width, rect().height());
    QLinearGradient gradient(progressRect.topLeft(), progressRect.topRight());
    QColor start;
    QColor mid;
    QColor end;
    start.setNamedColor("#0080FF");
    mid.setNamedColor("#0397FE");
    end.setNamedColor("#06BEFD");
    gradient.setColorAt(0, start);
    gradient.setColorAt(0.28, mid);
    gradient.setColorAt(1, end);

    painter.setBrush(gradient);
    painter.drawRoundedRect(progressRect, 5, 5);
}
