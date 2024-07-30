#include "kpixmapsequenceoverlaypaintertest.h"
#include "kpixmapsequence.h"
#include "kpixmapsequenceoverlaypainter.h"

#include <QApplication>
#include <QComboBox>
#include <QEvent>
#include <QGridLayout>
#include <QSpinBox>
#include <QTest>
#include <QVariant>

Q_DECLARE_METATYPE(Qt::Alignment)

TestWidget::TestWidget()
    : QWidget()
{
    m_draggingLeft = false;
    m_draggingRight = false;
    m_draggingTop = false;
    m_draggingBottom = false;

    QGridLayout *layout = new QGridLayout(this);
    m_widget = new QWidget(this);
    m_alignment = new QComboBox(this);
    m_offsetX = new QSpinBox(this);
    m_offsetY = new QSpinBox(this);

    layout->addWidget(m_widget, 0, 0, 1, 2);
    layout->addWidget(m_alignment, 1, 0, 1, 2);
    layout->addWidget(m_offsetX, 2, 0, 1, 1);
    layout->addWidget(m_offsetY, 2, 1, 1, 1);

    m_alignment->addItem(QStringLiteral("Center"), QVariant::fromValue(Qt::Alignment(Qt::AlignCenter)));
    m_alignment->addItem(QStringLiteral("Top-left"), QVariant::fromValue(Qt::Alignment(Qt::AlignTop | Qt::AlignLeft)));
    m_alignment->addItem(QStringLiteral("Top"), QVariant::fromValue(Qt::Alignment(Qt::AlignTop | Qt::AlignHCenter)));
    m_alignment->addItem(QStringLiteral("Top-right"), QVariant::fromValue(Qt::Alignment(Qt::AlignTop | Qt::AlignRight)));
    m_alignment->addItem(QStringLiteral("Right"), QVariant::fromValue(Qt::Alignment(Qt::AlignRight | Qt::AlignVCenter)));
    m_alignment->addItem(QStringLiteral("Bottom-right"), QVariant::fromValue(Qt::Alignment(Qt::AlignRight | Qt::AlignBottom)));
    m_alignment->addItem(QStringLiteral("Bottom"), QVariant::fromValue(Qt::Alignment(Qt::AlignHCenter | Qt::AlignBottom)));
    m_alignment->addItem(QStringLiteral("Bottom-left"), QVariant::fromValue(Qt::Alignment(Qt::AlignLeft | Qt::AlignBottom)));
    m_alignment->addItem(QStringLiteral("Left"), QVariant::fromValue(Qt::Alignment(Qt::AlignLeft | Qt::AlignVCenter)));

    connect(m_alignment, &QComboBox::activated, this, &TestWidget::alignementChanged);
    connect(m_offsetX, &QSpinBox::valueChanged, this, &TestWidget::offsetChanged);
    connect(m_offsetY, &QSpinBox::valueChanged, this, &TestWidget::offsetChanged);

    m_painter = new KPixmapSequenceOverlayPainter(KPixmapSequence(QFINDTESTDATA("kpixmap-animation.png"), 22), this);
    m_painter->setWidget(m_widget);
    m_painter->start();
}

TestWidget::~TestWidget()
{
}

void TestWidget::alignementChanged(int i)
{
    m_painter->setAlignment(m_alignment->itemData(i).value<Qt::Alignment>());
}

void TestWidget::offsetChanged()
{
    m_painter->setOffset(QPoint(m_offsetX->value(), m_offsetY->value()));
}

bool TestWidget::eventFilter(QObject *o, QEvent *e)
{
    if (o == m_widget) {
        if (e->type() == QEvent::Paint) {
        } else if (e->type() == QEvent::MouseButtonPress) {
        } else if (e->type() == QEvent::MouseButtonRelease) {
        } else if (e->type() == QEvent::MouseMove) {
        }
    }
    return QWidget::eventFilter(o, e);
}

/* --- MAIN -----------------------*/
int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("test"));

    TestWidget *window;

    QApplication testapp(argc, argv);
    window = new TestWidget();
    window->show();
    return testapp.exec();
}

#include "moc_kpixmapsequenceoverlaypaintertest.cpp"
