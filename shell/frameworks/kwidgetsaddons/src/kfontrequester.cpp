/*
    SPDX-FileCopyrightText: 2003 Nadeem Hasan <nhasan@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kfontrequester.h"
#include "fonthelpers_p.h"

#include <KFontChooserDialog>

#include <QCoreApplication>
#include <QEvent>
#include <QFontDatabase>
#include <QFontInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>

#include <cmath>

class KFontRequesterPrivate
{
    Q_DECLARE_TR_FUNCTIONS(KFontRequester)

public:
    KFontRequesterPrivate(KFontRequester *qq)
        : q(qq)
    {
    }

    void displaySampleText();
    void setToolTip();

    void buttonClicked();

    KFontRequester *q;
    bool m_onlyFixed;
    QString m_sampleText, m_title;
    QLabel *m_sampleLabel = nullptr;
    QPushButton *m_button = nullptr;
    QFont m_selFont;
};

KFontRequester::KFontRequester(QWidget *parent, bool onlyFixed)
    : QWidget(parent)
    , d(new KFontRequesterPrivate(this))
{
    d->m_onlyFixed = onlyFixed;

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    d->m_sampleLabel = new QLabel(this);
    d->m_button = new QPushButton(QIcon::fromTheme(QStringLiteral("document-edit")), QString(), this);

    d->m_sampleLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setFocusProxy(d->m_button);
    setFocusPolicy(d->m_button->focusPolicy());

    layout->addWidget(d->m_sampleLabel, 1);
    layout->addWidget(d->m_button);

    connect(d->m_button, &QPushButton::clicked, this, [this] {
        d->buttonClicked();
    });

    d->displaySampleText();
    d->setToolTip();

    d->m_sampleLabel->installEventFilter(this);
}

KFontRequester::~KFontRequester() = default;

QFont KFontRequester::font() const
{
    return d->m_selFont;
}

bool KFontRequester::isFixedOnly() const
{
    return d->m_onlyFixed;
}

QString KFontRequester::sampleText() const
{
    return d->m_sampleText;
}

QString KFontRequester::title() const
{
    return d->m_title;
}

QLabel *KFontRequester::label() const
{
    return d->m_sampleLabel;
}

QPushButton *KFontRequester::button() const
{
    return d->m_button;
}

void KFontRequester::setFont(const QFont &font, bool onlyFixed)
{
    d->m_selFont = font;
    d->m_onlyFixed = onlyFixed;

    d->displaySampleText();
    Q_EMIT fontSelected(d->m_selFont);
}

void KFontRequester::setSampleText(const QString &text)
{
    d->m_sampleText = text;
    d->displaySampleText();
}

void KFontRequester::setTitle(const QString &title)
{
    d->m_title = title;
    d->setToolTip();
}

bool KFontRequester::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == d->m_sampleLabel) {
        switch (event->type()) {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease: {
            auto *e = static_cast<QMouseEvent *>(event);

            if (e->button() == Qt::LeftButton && rect().contains(e->pos())) {
                if (e->type() == QEvent::MouseButtonRelease) {
                    d->buttonClicked();
                }
                e->accept();
                return true;
            }
            break;
        }
        default:
            break;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void KFontRequesterPrivate::buttonClicked()
{
    KFontChooser::DisplayFlags flags = m_onlyFixed ? KFontChooser::FixedFontsOnly : KFontChooser::NoDisplayFlags;

    const int result = KFontChooserDialog::getFont(m_selFont, flags, q->parentWidget());

    if (result == QDialog::Accepted) {
        displaySampleText();
        Q_EMIT q->fontSelected(m_selFont);
    }
}

void KFontRequesterPrivate::displaySampleText()
{
    m_sampleLabel->setFont(m_selFont);

    qreal size = m_selFont.pointSizeF();
    if (size == -1) {
        size = m_selFont.pixelSize();
    }

    if (m_sampleText.isEmpty()) {
        QString family = translateFontName(m_selFont.family());
        m_sampleLabel->setText(QStringLiteral("%1 %2").arg(family).arg(size));
    } else {
        m_sampleLabel->setText(m_sampleText);
    }
}

void KFontRequesterPrivate::setToolTip()
{
    m_button->setToolTip(tr("Choose font…", "@info:tooltip"));

    m_sampleLabel->setToolTip(QString());
    m_sampleLabel->setWhatsThis(QString());

    if (m_title.isNull()) {
        m_sampleLabel->setToolTip(tr("Preview of the selected font", "@info:tooltip"));
        m_sampleLabel->setWhatsThis(
            tr("This is a preview of the selected font. You can change it"
               " by clicking the \"Choose Font...\" button.",
               "@info:whatsthis"));
    } else {
        m_sampleLabel->setToolTip(tr("Preview of the \"%1\" font", "@info:tooltip").arg(m_title));
        m_sampleLabel->setWhatsThis(tr("This is a preview of the \"%1\" font. You can change it"
                                       " by clicking the \"Choose Font...\" button.",
                                       "@info:whatsthis")
                                        .arg(m_title));
    }
}

#include "moc_kfontrequester.cpp"
