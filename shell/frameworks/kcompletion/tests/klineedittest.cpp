#include "klineedittest.h"

#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QTimer>

#include <klineedit.h>

KLineEditTest::KLineEditTest(QWidget *widget)
    : QWidget(widget)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QStringList list;
    list << QStringLiteral("Tree") << QStringLiteral("Suuupa") << QStringLiteral("Stroustrup") << QStringLiteral("Stone") << QStringLiteral("Slick")
         << QStringLiteral("Slashdot") << QStringLiteral("Send") << QStringLiteral("Peables") << QStringLiteral("Mankind") << QStringLiteral("Ocean")
         << QStringLiteral("Chips") << QStringLiteral("Computer") << QStringLiteral("Sandworm") << QStringLiteral("Sandstorm") << QStringLiteral("Chops");
    list.sort();

    m_lineedit = new KLineEdit(this);
    m_lineedit->setObjectName(QStringLiteral("klineedittest"));
    m_lineedit->completionObject()->setItems(list);
    m_lineedit->setSqueezedTextEnabled(true);
    m_lineedit->setClearButtonEnabled(true);
    connect(m_lineedit, &QLineEdit::returnPressed, this, &KLineEditTest::slotReturnPressed);
    connect(m_lineedit, &KLineEdit::returnKeyPressed, this, &KLineEditTest::slotReturnKeyPressed);

    QHBoxLayout *restrictedHBox = new QHBoxLayout;
    m_restrictedLine = new KLineEdit(this);
    QRegularExpression regex(QStringLiteral("[aeiouyé]*"));
    QRegularExpressionValidator *validator = new QRegularExpressionValidator(regex, m_restrictedLine);
    m_restrictedLine->setValidator(validator);
    // connect(m_restrictedLine, SIGNAL(invalidChar(int)), this, SLOT(slotInvalidChar(int)));
    connect(m_restrictedLine, &QLineEdit::returnPressed, this, &KLineEditTest::slotReturnPressed);
    connect(m_restrictedLine, &KLineEdit::returnKeyPressed, this, &KLineEditTest::slotReturnKeyPressed);
    restrictedHBox->addWidget(new QLabel(QStringLiteral("Vowels only:"), this));
    restrictedHBox->addWidget(m_restrictedLine);
    m_invalidCharLabel = new QLabel(this);
    restrictedHBox->addWidget(m_invalidCharLabel);

    // horizontal button layout
    m_btnExit = new QPushButton(QStringLiteral("E&xit"), this);
    connect(m_btnExit, &QAbstractButton::clicked, this, &KLineEditTest::quitApp);

    m_btnReadOnly = new QPushButton(QStringLiteral("&Read Only"), this);
    m_btnReadOnly->setCheckable(true);
    connect(m_btnReadOnly, &QAbstractButton::toggled, this, &KLineEditTest::slotReadOnly);

    m_btnPassword = new QPushButton(QStringLiteral("&Password"), this);
    m_btnPassword->setCheckable(true);
    connect(m_btnPassword, &QAbstractButton::toggled, this, &KLineEditTest::slotPassword);

    m_btnEnable = new QPushButton(QStringLiteral("Dis&able"), this);
    m_btnEnable->setCheckable(true);
    connect(m_btnEnable, &QAbstractButton::toggled, this, &KLineEditTest::slotEnable);

    m_btnHide = new QPushButton(QStringLiteral("Hi&de"), this);
    connect(m_btnHide, &QAbstractButton::clicked, this, &KLineEditTest::slotHide);

    m_btnPlaceholderText = new QPushButton(QStringLiteral("Place Holder Text"), this);
    m_btnPlaceholderText->setCheckable(true);
    connect(m_btnPlaceholderText, &QAbstractButton::toggled, this, &KLineEditTest::slotPlaceholderText);

    QPushButton *btnStyle = new QPushButton(QStringLiteral("Stylesheet"), this);
    connect(btnStyle, &QAbstractButton::clicked, this, &KLineEditTest::slotSetStyleSheet);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_btnExit);
    buttonLayout->addWidget(m_btnReadOnly);
    buttonLayout->addWidget(m_btnPassword);
    buttonLayout->addWidget(m_btnEnable);
    buttonLayout->addWidget(m_btnHide);
    buttonLayout->addWidget(m_btnPlaceholderText);
    buttonLayout->addWidget(btnStyle);

    layout->addWidget(m_lineedit);
    layout->addLayout(restrictedHBox);
    layout->addLayout(buttonLayout);
    setWindowTitle(QStringLiteral("KLineEdit Unit Test"));
}

KLineEditTest::~KLineEditTest()
{
}

void KLineEditTest::quitApp()
{
    qApp->closeAllWindows();
}

void KLineEditTest::slotSetStyleSheet()
{
    m_lineedit->setStyleSheet(QStringLiteral("QLineEdit{ background-color:#baf9ce }"));
}

void KLineEditTest::show()
{
    if (m_lineedit->isHidden()) {
        m_lineedit->show();
    }

    m_btnHide->setEnabled(true);

    QWidget::show();
}

void KLineEditTest::slotReturnPressed()
{
    qDebug() << "Return pressed";
}

void KLineEditTest::slotReturnKeyPressed(const QString &text)
{
    qDebug() << "Return pressed: " << text;
}

void KLineEditTest::resultOutput(const QString &text)
{
    qDebug() << "KlineEditTest Debug: " << text;
}

void KLineEditTest::slotReadOnly(bool ro)
{
    m_lineedit->setReadOnly(ro);
    QString text = (ro) ? "&Read Write" : "&Read Only";
    m_btnReadOnly->setText(text);
}

void KLineEditTest::slotPassword(bool pw)
{
    m_lineedit->setEchoMode(pw ? QLineEdit::Password : QLineEdit::Normal);
    QString text = (pw) ? "&Normal Text" : "&Password";
    m_btnPassword->setText(text);
}

void KLineEditTest::slotEnable(bool enable)
{
    m_lineedit->setEnabled(!enable);
    QString text = (enable) ? "En&able" : "Dis&able";
    m_btnEnable->setText(text);
}

void KLineEditTest::slotPlaceholderText(bool click)
{
    if (click) {
        m_lineedit->setText(QLatin1String("")); // Clear before to add message
        m_lineedit->setPlaceholderText(QStringLiteral("Click in this lineedit"));
    }
}

void KLineEditTest::slotHide()
{
    m_lineedit->hide();
    m_btnHide->setEnabled(false);
    m_lineedit->setText(
        "My dog ate the homework, whaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaa! I want my mommy!");
    QTimer::singleShot(1000, this, &KLineEditTest::show);
}

void KLineEditTest::slotInvalidChar(int key)
{
    m_invalidCharLabel->setText(QStringLiteral("Invalid char: %1").arg(key));
}

int main(int argc, char **argv)
{
    QApplication a(argc, argv);
    KLineEditTest *t = new KLineEditTest();
    // t->lineEdit()->setTrapReturnKey( true );
    // t->lineEdit()->completionBox()->setTabHandling( false );
    t->lineEdit()->setSqueezedTextEnabled(true);
    t->lineEdit()->setText(
        "This is a really really really really really really "
        "really really long line because I am a talkative fool!"
        "I mean ... REALLY talkative. If you don't believe me, ask my cousin.");
    t->show();
    return a.exec();
}

#include "moc_klineedittest.cpp"
