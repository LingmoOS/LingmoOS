#include <QPixmap>
#include <QThread>

#include "MainWidget.h"

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , m_image(new QLabel(this))
    , m_caption(new QLabel(tr("Hello Internet!"), m_image))
    , m_status(new QLabel(tr("Please wait..."), m_image))
{
    setFixedSize(800, 450);
    // set style sheet for the text labels:
    m_caption->setContentsMargins(18, 9, 9, 9);
    m_caption->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_caption->setWordWrap(true);
    m_caption->setStyleSheet(QStringLiteral("background-color: rgba(255, 255, 255, 200); font: italic 18pt \"Sans Serif\";"));
    m_status->setContentsMargins(18, 9, 9, 9);
    m_status->setStyleSheet(QStringLiteral("background-color: rgba(255, 255, 255, 200); font: 10pt \"Sans Serif\";"));
}

MainWidget::~MainWidget()
{
}

void MainWidget::resizeEvent(QResizeEvent *)
{
    m_image->setGeometry(0, 0, width(), height());
    m_caption->move(0, 0);
    m_caption->setFixedWidth(width());
    m_caption->setFixedHeight(qMax(m_caption->sizeHint().height(), height() / 3));
    m_status->setFixedWidth(width());
    m_status->setFixedHeight(qMax(m_status->sizeHint().height(), height() / 9));
    m_status->move(0, height() - m_status->height());
}

void MainWidget::setImage(QImage image)
{
    Q_ASSERT(thread() == QThread::currentThread());
    m_image->setPixmap(QPixmap::fromImage(image));
}

void MainWidget::setCaption(QString text)
{
    m_caption->setText(text);
}

void MainWidget::setStatus(QString text)
{
    m_status->setText(text);
}

#include "moc_MainWidget.cpp"
