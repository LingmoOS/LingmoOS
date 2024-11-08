#include "kdragwidget.h"
#include <QStyleOptionFrame>
#include <QPainterPath>
#include <QBoxLayout>
#include <QMimeData>
#include <QPainter>
#include <QDebug>
#include <QCoreApplication>
#include <QStandardPaths>
#include "themeController.h"

namespace kdk {

class FileDialog : public QFileDialog
{
    Q_OBJECT
public:
    FileDialog(QWidget* parent = nullptr);
    virtual ~FileDialog();

public slots:
    void goAccept();

protected:
    void showEvent(QShowEvent* event) override;
};

class KDragWidgetPrivate : public QObject ,public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KDragWidget)
public:
    KDragWidgetPrivate(KDragWidget* parent);
    void openFile();

protected:
    void changeTheme();

private:
    KDragWidget* q_ptr;
    QColor lineColor;
    Qt::PenStyle mainState;
    KPushButton* m_pIconButton;
    QLabel* m_pTextLabel;
    FileDialog *m_FileDialog;
    QStringList urlList;
};

KDragWidgetPrivate::KDragWidgetPrivate(KDragWidget* parent)
    :q_ptr(parent)
    ,mainState(Qt::DashLine)
{
    Q_Q(KDragWidget);
    m_FileDialog = new FileDialog(q);
    m_FileDialog->setDirectory(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
}

void KDragWidgetPrivate::openFile()
{
    Q_Q(KDragWidget);

    m_FileDialog->setWindowTitle(tr("Please select file"));


    if (m_FileDialog->exec() == QDialog::Accepted) {
        QStringList selectFiles = m_FileDialog->selectedFiles();
        for (const QString& fileName : selectFiles) {
            if (!fileName.isEmpty())
                Q_EMIT q->getPath(fileName);
        }
    }
}

void KDragWidgetPrivate::changeTheme()
{
    Q_Q(KDragWidget);
    initThemeStyle();

    if(ThemeController::themeMode() == LightTheme)
        lineColor = QColor(218, 218, 218);
    else
        lineColor = QColor(95, 95, 98);
}

KDragWidget::KDragWidget(QWidget *parent)
    :QWidget(parent),
      d_ptr(new KDragWidgetPrivate(this))
{
    Q_D(KDragWidget);
    setAcceptDrops(true);//设置接受拖拽

    QVBoxLayout* mainLayout= new QVBoxLayout(this);
    QHBoxLayout* hLayout= new QHBoxLayout();
    QHBoxLayout* hLayout1= new QHBoxLayout();
    d->m_pIconButton = new KPushButton();
    d->m_pIconButton->setFixedSize(54,54);
    d->m_pIconButton->setTranslucent(true);
    d->m_pIconButton->setButtonType(KPushButton::CircleType);
    d->m_pIconButton->setIcon(QIcon::fromTheme("list-add.symbolic"));

    d->m_pTextLabel = new QLabel();
    d->m_pTextLabel->setText(tr("Select or drag and drop the folder identification path"));
    hLayout->setContentsMargins(0,0,0,0);
    hLayout->addStretch();
    hLayout->addWidget(d->m_pIconButton);
    hLayout->addStretch();

    hLayout1->setContentsMargins(0,0,0,0);
    hLayout1->addStretch();
    hLayout1->addWidget(d->m_pTextLabel);
    hLayout1->addStretch();

    mainLayout->setSpacing(0);
    mainLayout->addStretch();
    mainLayout->addLayout(hLayout);
    mainLayout->addSpacing(16);
    mainLayout->addLayout(hLayout1);
    mainLayout->addStretch();

    d->changeTheme();

    connect(d->m_pIconButton,&KPushButton::clicked,this,[=](){d->openFile();});
    connect(d->m_gsetting,&QGSettings::changed,this,[=](){d->changeTheme();});

    installEventFilter(this);
}

KPushButton *KDragWidget::iconButton()
{
    Q_D(KDragWidget);
    return d->m_pIconButton;
}

QLabel *KDragWidget::textLabel()
{
    Q_D(KDragWidget);
    return d->m_pTextLabel;
}

QFileDialog *KDragWidget::fileDialog()
{
    Q_D(KDragWidget);
    return dynamic_cast<QFileDialog*>(d->m_FileDialog);
}

void KDragWidget::setNameFilter(const QString &filter)
{
    Q_D(KDragWidget);
    QString f(filter);
    if (f.isEmpty())
        return ;
    QString sep(QLatin1String(";;"));
    int i = f.indexOf(sep, 0);
    if (i == -1)
    {
        if (f.indexOf(QLatin1Char('\n'), 0) != -1)
        {
            sep = QLatin1Char('\n');
            i = f.indexOf(sep, 0);
        }
    }
    setNameFilters(f.split(sep));
}

void KDragWidget::setNameFilters(const QStringList &filters)
{
    Q_D(KDragWidget);
    d->m_FileDialog->setNameFilters(filters);

    for(int i = 0 ; i < filters.size();i++)
    {
        if(!d->urlList.contains(filters.at(i)))
            d->urlList.append(filters.at(i));
    }
}

void KDragWidget::mouseMoveEvent(QMouseEvent *event)
{
   return QWidget::mouseMoveEvent(event);
}

void KDragWidget::mousePressEvent(QMouseEvent *event)
{
    return QWidget::mousePressEvent(event);
}

void KDragWidget::mouseReleaseEvent(QMouseEvent *event)
{
    return QWidget::mouseReleaseEvent(event);
}

void KDragWidget::keyPressEvent(QKeyEvent *event)
{
    return QWidget::keyPressEvent(event);
}

void KDragWidget::keyReleaseEvent(QKeyEvent *event)
{
    return QWidget::keyReleaseEvent(event);
}

void KDragWidget::paintEvent(QPaintEvent *event)
{
    Q_D(KDragWidget);
    QPainter painter(this);

    QPen pen = painter.pen();
    pen.setColor(d->lineColor);
    pen.setStyle(d->mainState);
    painter.setPen(pen);
    painter.drawRoundedRect(rect().adjusted(0,0,-1,-1),6,6);
}

bool KDragWidget::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(KDragWidget);
    switch (event->type()) {
    case QEvent::MouseButtonPress:
    {
        d->m_pIconButton->setAttribute(Qt::WA_UnderMouse, true);
        d->lineColor = palette().toolTipText().color();
        d->openFile();

        update();
        break;
    }
    case QEvent::MouseButtonRelease:
    {
        d->m_pIconButton->setAttribute(Qt::WA_UnderMouse, true);
        d->lineColor = palette().toolTipText().color();
        update();
        break;
    }
    case QEvent::Leave:
    {
        d->m_pIconButton->setAttribute(Qt::WA_UnderMouse, false);
        d->lineColor = palette().placeholderText().color();
        update();
        break;
    }
    case QEvent::Enter:
    {
        d->m_pIconButton->setAttribute(Qt::WA_UnderMouse, true);
        d->lineColor = palette().toolTipText().color();
        update();
        break;
    }
    case QEvent::DragEnter:
    {
        d->m_pIconButton->setAttribute(Qt::WA_UnderMouse, true);
        d->lineColor = palette().highlight().color();
        update();
        break;
    }
    case QEvent::DragLeave:
    {
        d->m_pIconButton->setAttribute(Qt::WA_UnderMouse, false);
        d->lineColor = palette().placeholderText().color();
        update();
        break;
    }
    default:
        break;
    }
    return QWidget::eventFilter(watched,event);
}

void KDragWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if(event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
    }
    else
    {
        event->ignore();
    }
}

void KDragWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    return QWidget::dragLeaveEvent(event);
}

void KDragWidget::dragMoveEvent(QDragMoveEvent *event)
{
    return QWidget::dragMoveEvent(event);
}

void KDragWidget::dropEvent(QDropEvent* event)
{
    Q_D(KDragWidget);

    const QMimeData* mimeData = event->mimeData();
    if(mimeData->hasUrls())
    {
        QList<QUrl>urlList = mimeData->urls();
        for(int i=0; i<urlList.size(); i++) //选中的文件
        {
            QString fileName = urlList.at(i).toLocalFile();
//            if(!fileName.isEmpty()) //
//            {
                if(fileName.lastIndexOf(".") == -1) //文件夹可正常选中
                    Q_EMIT getPath(fileName);
                else
                {
                    QString endStr = fileName.mid(fileName.lastIndexOf("."),fileName.size()-fileName.lastIndexOf("."));
                    if(d->urlList.isEmpty()) //若无过滤则正常选中
                        Q_EMIT getPath(fileName);
                    else
                    {
                        for(int temp = 0 ;temp <d->urlList.size();temp++)  //遍历是否符合筛选条件
                        {
                            QString curString = d->urlList.at(temp);
                            if(curString.contains(endStr))
                            {
                                Q_EMIT getPath(fileName);
                                return ;
                            }
                        }
                    }
                }
//            }
        }
    }
}

FileDialog::FileDialog(QWidget *parent) :
    QFileDialog(parent)
{
    this->setFileMode(QFileDialog::ExistingFiles);
}

FileDialog::~FileDialog()
{

}

void FileDialog::goAccept()
{
    accept();
}

void FileDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    if (this->isVisible()) {
        for (QWidget* widget : this->findChildren<QWidget*>()) {
            if (widget->objectName() == "acceptButton") {
                QPushButton * btn = qobject_cast<QPushButton*>(widget);
                disconnect(btn, &QPushButton::clicked, 0, 0);
                connect(btn, &QPushButton::clicked, [=](bool checked){
                    Q_UNUSED(checked)
                    this->goAccept();
                });
            }
        }
    }
}

}

#include "kdragwidget.moc"
#include "moc_kdragwidget.cpp"
