/*
    SPDX-FileCopyrightText: 1996 Bernd Johannes Wuebben <wuebben@kde.org>
    SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
    SPDX-FileCopyrightText: 1999 Mario Weilguni <mweilguni@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kfontchooser.h"
#include "fonthelpers_p.h"
#include "ui_kfontchooserwidget.h"

#include "loggingcategory.h"

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFontDatabase>
#include <QGroupBox>
#include <QGuiApplication>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QLocale>
#include <QScrollBar>
#include <QSplitter>
#include <QTextEdit>

#include <algorithm>
#include <cmath>

// When message extraction needs to be avoided.
#define TR_NOX tr

static int minimumListWidth(const QListWidget *list)
{
    QFontMetrics fm = list->fontMetrics();

    const int extraSpace = fm.horizontalAdvance(QLatin1Char(' ')) * 2;

    // Minimum initial size
    int width = 40;
    for (int i = 0, rows = list->count(); i < rows; ++i) {
        int itemWidth = fm.horizontalAdvance(list->item(i)->text());
        // ...and add a space on both sides for a not too tight look.
        itemWidth += extraSpace;
        width = std::max(width, itemWidth);
    }

    width += list->frameWidth() * 2;
    width += list->verticalScrollBar()->sizeHint().width();
    return width;
}

static int minimumListHeight(const QListWidget *list, int numVisibleEntry)
{
    int w = list->count() > 0 ? list->visualItemRect(list->item(0)).height() : list->fontMetrics().lineSpacing();

    if (w < 0) {
        w = 10;
    }
    if (numVisibleEntry <= 0) {
        numVisibleEntry = 4;
    }
    return (w * numVisibleEntry + 2 * list->frameWidth());
}

static QString formatFontSize(qreal size)
{
    return QLocale::system().toString(size, 'f', (size == floor(size)) ? 0 : 1);
}

class KFontChooserPrivate
{
    Q_DECLARE_TR_FUNCTIONS(KFontChooser)

public:
    KFontChooserPrivate(KFontChooser::DisplayFlags flags, KFontChooser *qq)
        : q(qq)
        , m_flags(flags)
    {
        m_palette.setColor(QPalette::Active, QPalette::Text, Qt::black);
        m_palette.setColor(QPalette::Active, QPalette::Base, Qt::white);
    }

    void init();
    void setFamilyBoxItems(const QStringList &fonts = {});
    int nearestSizeRow(qreal val, bool customize);
    qreal fillSizeList(const QList<qreal> &sizes = QList<qreal>());
    qreal setupSizeListBox(const QString &family, const QString &style);

    void setupDisplay();
    QString styleIdentifier(const QFont &font);

    void slotFamilySelected(const QString &);
    void slotSizeSelected(const QString &);
    void slotStyleSelected(const QString &);
    void displaySample(const QFont &font);
    void slotSizeValue(double);

    KFontChooser *q;

    std::unique_ptr<Ui_KFontChooserWidget> m_ui;

    KFontChooser::DisplayFlags m_flags = KFontChooser::NoDisplayFlags;

    QPalette m_palette;

    QFont m_selectedFont;

    QString m_selectedStyle;
    qreal m_selectedSize = -1.0;

    QString m_standardSizeAtCustom;
    int m_customSizeRow = -1;

    bool m_signalsAllowed = true;
    bool m_usingFixed = false;

    // Mappings of translated to Qt originated family and style strings.
    FontFamiliesMap m_qtFamilies;
    std::map<QString, QString> m_qtStyles;
    // Mapping of translated style strings to internal style identifiers.
    std::map<QString, QString> m_styleIDs;
};

KFontChooser::KFontChooser(QWidget *parent)
    : QWidget(parent)
    , d(new KFontChooserPrivate(KFontChooser::DisplayFrame, this))
{
    d->init();
}

KFontChooser::KFontChooser(const DisplayFlags flags, QWidget *parent)
    : QWidget(parent)
    , d(new KFontChooserPrivate(flags, this))
{
    d->init();
}

KFontChooser::~KFontChooser() = default;

void KFontChooserPrivate::init()
{
    m_usingFixed = m_flags & KFontChooser::FixedFontsOnly;

    // The main layout is divided horizontally into a top part with
    // the font attribute widgets (family, style, size) and a bottom
    // part with a preview of the selected font
    QVBoxLayout *mainLayout = new QVBoxLayout(q);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *page = m_flags & KFontChooser::DisplayFrame ? new QGroupBox(KFontChooser::tr("Requested Font", "@title:group"), q) : new QWidget(q);
    mainLayout->addWidget(page);

    m_ui.reset(new Ui_KFontChooserWidget);
    m_ui->setupUi(page);
    // Increase spacing on top of the preview field and then reset the other layouts
    // back to a standard value.
    m_ui->sampleTextEditLayout->setSpacing(q->style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing));
    m_ui->mainHorizontalLayout->setSpacing(q->style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing));
    m_ui->gridLayout->setVerticalSpacing(q->style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing) * 2);
    m_ui->gridLayout->setHorizontalSpacing(q->style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing));

    // Deprecated, we'll call show() if building with deprecated code
    m_ui->sizeIsRelativeCheckBox->hide();

    const bool isDiffMode = m_flags & KFontChooser::ShowDifferences;

    QObject::connect(m_ui->familyListWidget, &QListWidget::currentTextChanged, [this](const QString &family) {
        slotFamilySelected(family);
    });

    if (isDiffMode) {
        m_ui->familyLabel->hide();
        m_ui->familyListWidget->setEnabled(false);
        QObject::connect(m_ui->familyCheckBox, &QCheckBox::toggled, m_ui->familyListWidget, &QWidget::setEnabled);
    } else {
        m_ui->familyCheckBox->hide();
    }

    setFamilyBoxItems();

    // If the calling app sets FixedFontsOnly, don't show the "show fixed only" checkbox
    m_ui->onlyFixedCheckBox->setVisible(!m_usingFixed);

    if (!m_ui->onlyFixedCheckBox->isHidden()) {
        QObject::connect(m_ui->onlyFixedCheckBox, &QCheckBox::toggled, q, [this](const bool state) {
            q->setFont(m_selectedFont, state);
        });

        if (isDiffMode) { // In this mode follow the state of the m_ui->familyCheckBox
            m_ui->onlyFixedCheckBox->setEnabled(false);
            QObject::connect(m_ui->familyCheckBox, &QCheckBox::toggled, m_ui->onlyFixedCheckBox, &QWidget::setEnabled);
        }
    }

    // Populate usual styles, to determine minimum list width;
    // will be replaced later with correct styles.
    m_ui->styleListWidget->addItem(KFontChooser::tr("Normal", "@item font"));
    m_ui->styleListWidget->addItem(KFontChooser::tr("Italic", "@item font"));
    m_ui->styleListWidget->addItem(KFontChooser::tr("Oblique", "@item font"));
    m_ui->styleListWidget->addItem(KFontChooser::tr("Bold", "@item font"));
    m_ui->styleListWidget->addItem(KFontChooser::tr("Bold Condensed Oblique", "@item font"));
    m_ui->styleListWidget->setMinimumWidth(minimumListWidth(m_ui->styleListWidget));

    QObject::connect(m_ui->styleListWidget, &QListWidget::currentTextChanged, [this](const QString &style) {
        slotStyleSelected(style);
    });

    if (isDiffMode) {
        m_ui->styleLabel->hide();
        m_ui->styleListWidget->setEnabled(false);
        QObject::connect(m_ui->styleCheckBox, &QCheckBox::toggled, m_ui->styleListWidget, &QWidget::setEnabled);
    } else {
        m_ui->styleCheckBox->hide();
    }

    // Populate with usual sizes, to determine minimum list width;
    // will be replaced later with correct sizes.
    fillSizeList();

    QObject::connect(m_ui->sizeSpinBox, &QDoubleSpinBox::valueChanged, [this](const double size) {
        slotSizeValue(size);
    });

    QObject::connect(m_ui->sizeListWidget, &QListWidget::currentTextChanged, [this](const QString &size) {
        slotSizeSelected(size);
    });

    if (isDiffMode) {
        m_ui->sizeLabel->hide();
        m_ui->sizeListWidget->setEnabled(false);
        m_ui->sizeSpinBox->setEnabled(false);
        QObject::connect(m_ui->sizeCheckBox, &QCheckBox::toggled, m_ui->sizeListWidget, &QWidget::setEnabled);
        QObject::connect(m_ui->sizeCheckBox, &QCheckBox::toggled, m_ui->sizeSpinBox, &QWidget::setEnabled);
    } else {
        m_ui->sizeCheckBox->hide();
    }

    QFont tmpFont(q->font().family(), 64, QFont::Black);
    m_ui->sampleTextEdit->setFont(tmpFont);
    m_ui->sampleTextEdit->setMinimumHeight(m_ui->sampleTextEdit->fontMetrics().lineSpacing());
    // tr: A classical test phrase, with all letters of the English alphabet.
    // Replace it with a sample text in your language, such that it is
    // representative of language's writing system.
    // If you wish, you can input several lines of text separated by \n.
    q->setSampleText(KFontChooser::tr("The Quick Brown Fox Jumps Over The Lazy Dog"));
    m_ui->sampleTextEdit->setTextCursor(QTextCursor(m_ui->sampleTextEdit->document()));

    QObject::connect(q, &KFontChooser::fontSelected, q, [this](const QFont &font) {
        displaySample(font);
    });

    // lets initialize the display if possible
    if (m_usingFixed) {
        q->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont), true);
    } else {
        q->setFont(QGuiApplication::font(), false);
    }

    // Set the minimum height for the list widgets
    q->setMinVisibleItems(4);

    // Set focus to the size list as this is the most commonly changed property
    m_ui->sizeListWidget->setFocus();
}

void KFontChooser::setColor(const QColor &col)
{
    d->m_palette.setColor(QPalette::Active, QPalette::Text, col);
    QPalette pal = d->m_ui->sampleTextEdit->palette();
    pal.setColor(QPalette::Active, QPalette::Text, col);
    d->m_ui->sampleTextEdit->setPalette(pal);
    QTextCursor cursor = d->m_ui->sampleTextEdit->textCursor();
    d->m_ui->sampleTextEdit->selectAll();
    d->m_ui->sampleTextEdit->setTextColor(col);
    d->m_ui->sampleTextEdit->setTextCursor(cursor);
}

QColor KFontChooser::color() const
{
    return d->m_palette.color(QPalette::Active, QPalette::Text);
}

void KFontChooser::setBackgroundColor(const QColor &col)
{
    d->m_palette.setColor(QPalette::Active, QPalette::Base, col);
    QPalette pal = d->m_ui->sampleTextEdit->palette();
    pal.setColor(QPalette::Active, QPalette::Base, col);
    d->m_ui->sampleTextEdit->setPalette(pal);
}

QColor KFontChooser::backgroundColor() const
{
    return d->m_palette.color(QPalette::Active, QPalette::Base);
}

QString KFontChooser::sampleText() const
{
    return d->m_ui->sampleTextEdit->toPlainText();
}

void KFontChooser::setSampleText(const QString &text)
{
    d->m_ui->sampleTextEdit->setPlainText(text);
}

void KFontChooser::setSampleBoxVisible(bool visible)
{
    d->m_ui->sampleTextEdit->setVisible(visible);
}

QSize KFontChooser::sizeHint(void) const
{
    return minimumSizeHint();
}

void KFontChooser::enableColumn(int column, bool state)
{
    if (column & FamilyList) {
        d->m_ui->familyListWidget->setEnabled(state);
    }
    if (column & StyleList) {
        d->m_ui->styleListWidget->setEnabled(state);
    }
    if (column & SizeList) {
        d->m_ui->sizeListWidget->setEnabled(state);
        d->m_ui->sizeSpinBox->setEnabled(state);
    }
}

void KFontChooser::setFont(const QFont &aFont, bool onlyFixed)
{
    d->m_selectedFont = aFont;
    d->m_selectedSize = aFont.pointSizeF();
    if (d->m_selectedSize == -1) {
        d->m_selectedSize = QFontInfo(aFont).pointSizeF();
    }

    if (onlyFixed != d->m_usingFixed) {
        d->m_usingFixed = onlyFixed;
        d->setFamilyBoxItems();
    }
    d->setupDisplay();
}

KFontChooser::FontDiffFlags KFontChooser::fontDiffFlags() const
{
    FontDiffFlags diffFlags = NoFontDiffFlags;

    if (d->m_ui->familyCheckBox->isChecked()) {
        diffFlags |= FontDiffFamily;
    }

    if (d->m_ui->styleCheckBox->isChecked()) {
        diffFlags |= FontDiffStyle;
    }

    if (d->m_ui->sizeCheckBox->isChecked()) {
        diffFlags |= FontDiffSize;
    }

    return diffFlags;
}

QFont KFontChooser::font() const
{
    return d->m_selectedFont;
}

static bool isDefaultFontStyleName(const QString &style)
{
    /* clang-format off */
    // Ordered by commonness, i.e. "Regular" is the most common
    return style == QLatin1String("Regular")
        || style == QLatin1String("Normal")
        || style == QLatin1String("Book")
        || style == QLatin1String("Roman");
    /* clang-format on */
}

void KFontChooserPrivate::slotFamilySelected(const QString &family)
{
    if (!m_signalsAllowed) {
        return;
    }
    m_signalsAllowed = false;

    QString currentFamily;
    if (family.isEmpty()) {
        Q_ASSERT(m_ui->familyListWidget->currentItem());
        if (m_ui->familyListWidget->currentItem()) {
            currentFamily = m_qtFamilies[m_ui->familyListWidget->currentItem()->text()];
        }
    } else {
        currentFamily = m_qtFamilies[family];
    }

    // Get the list of styles available in this family.
    QStringList styles = QFontDatabase::styles(currentFamily);
    if (styles.isEmpty()) {
        // Avoid extraction, it is in kdeqt.po
        styles.append(TR_NOX("Normal", "QFontDatabase"));
    }

    // Always prepend Regular, Normal, Book or Roman, this way if "m_selectedStyle"
    // in the code below is empty, selecting index 0 should work better
    std::sort(styles.begin(), styles.end(), [](const QString &a, const QString &b) {
        if (isDefaultFontStyleName(a)) {
            return true;
        } else if (isDefaultFontStyleName(b)) {
            return false;
        }
        return false;
    });

    // Filter style strings and add to the listbox.
    QString pureFamily;
    splitFontString(family, &pureFamily);
    QStringList filteredStyles;
    m_qtStyles.clear();
    m_styleIDs.clear();

    const QStringList origStyles = styles;
    for (const QString &style : origStyles) {
        // Sometimes the font database will report an invalid style,
        // that falls back back to another when set.
        // Remove such styles, by checking set/get round-trip.
        QFont testFont = QFontDatabase::font(currentFamily, style, 10);
        if (QFontDatabase::styleString(testFont) != style) {
            styles.removeAll(style);
            continue;
        }

        QString fstyle = tr("%1", "@item Font style").arg(style);
        if (!filteredStyles.contains(fstyle)) {
            filteredStyles.append(fstyle);
            m_qtStyles.insert({fstyle, style});
            m_styleIDs.insert({fstyle, styleIdentifier(testFont)});
        }
    }
    m_ui->styleListWidget->clear();
    m_ui->styleListWidget->addItems(filteredStyles);

    // Try to set the current style in the listbox to that previous.
    int listPos = filteredStyles.indexOf(m_selectedStyle.isEmpty() ? TR_NOX("Normal", "QFontDatabase") : m_selectedStyle);
    if (listPos < 0) {
        // Make extra effort to have Italic selected when Oblique was chosen,
        // and vice versa, as that is what the user would probably want.
        QString styleIt = tr("Italic", "@item font");
        QString styleOb = tr("Oblique", "@item font");
        for (int i = 0; i < 2; ++i) {
            int pos = m_selectedStyle.indexOf(styleIt);
            if (pos >= 0) {
                QString style = m_selectedStyle;
                style.replace(pos, styleIt.length(), styleOb);
                listPos = filteredStyles.indexOf(style);
                if (listPos >= 0) {
                    break;
                }
            }
            qSwap(styleIt, styleOb);
        }
    }
    m_ui->styleListWidget->setCurrentRow(listPos >= 0 ? listPos : 0);
    const QString currentStyle = m_qtStyles[m_ui->styleListWidget->currentItem()->text()];

    // Recompute the size listbox for this family/style.
    qreal currentSize = setupSizeListBox(currentFamily, currentStyle);
    m_ui->sizeSpinBox->setValue(currentSize);

    m_selectedFont = QFontDatabase::font(currentFamily, currentStyle, static_cast<int>(currentSize));
    if (QFontDatabase::isSmoothlyScalable(currentFamily, currentStyle) && m_selectedFont.pointSize() == floor(currentSize)) {
        m_selectedFont.setPointSizeF(currentSize);
    }
    Q_EMIT q->fontSelected(m_selectedFont);

    m_signalsAllowed = true;
}

void KFontChooserPrivate::slotStyleSelected(const QString &style)
{
    if (!m_signalsAllowed) {
        return;
    }
    m_signalsAllowed = false;

    const QString currentFamily = m_qtFamilies[m_ui->familyListWidget->currentItem()->text()];
    const QString currentStyle = !style.isEmpty() ? m_qtStyles[style] : m_qtStyles[m_ui->styleListWidget->currentItem()->text()];

    // Recompute the size listbox for this family/style.
    qreal currentSize = setupSizeListBox(currentFamily, currentStyle);
    m_ui->sizeSpinBox->setValue(currentSize);

    m_selectedFont = QFontDatabase::font(currentFamily, currentStyle, static_cast<int>(currentSize));
    if (QFontDatabase::isSmoothlyScalable(currentFamily, currentStyle) && m_selectedFont.pointSize() == floor(currentSize)) {
        m_selectedFont.setPointSizeF(currentSize);
    }
    Q_EMIT q->fontSelected(m_selectedFont);

    if (!style.isEmpty()) {
        m_selectedStyle = currentStyle;
    }

    m_signalsAllowed = true;
}

void KFontChooserPrivate::slotSizeSelected(const QString &size)
{
    if (!m_signalsAllowed) {
        return;
    }

    m_signalsAllowed = false;

    qreal currentSize = 0.0;
    if (size.isEmpty()) {
        currentSize = QLocale::system().toDouble(m_ui->sizeListWidget->currentItem()->text());
    } else {
        currentSize = QLocale::system().toDouble(size);
    }

    // Reset the customized size slot in the list if not needed.
    if (m_customSizeRow >= 0 && m_selectedFont.pointSizeF() != currentSize) {
        m_ui->sizeListWidget->item(m_customSizeRow)->setText(m_standardSizeAtCustom);
        m_customSizeRow = -1;
    }

    m_ui->sizeSpinBox->setValue(currentSize);
    m_selectedFont.setPointSizeF(currentSize);
    Q_EMIT q->fontSelected(m_selectedFont);

    if (!size.isEmpty()) {
        m_selectedSize = currentSize;
    }

    m_signalsAllowed = true;
}

void KFontChooserPrivate::slotSizeValue(double dval)
{
    if (!m_signalsAllowed) {
        return;
    }
    m_signalsAllowed = false;

    // We compare with qreal, so convert for platforms where qreal != double.
    qreal val = qreal(dval);

    // Reset current size slot in list if it was customized.
    if (m_customSizeRow >= 0 && m_ui->sizeListWidget->currentRow() == m_customSizeRow) {
        m_ui->sizeListWidget->item(m_customSizeRow)->setText(m_standardSizeAtCustom);
        m_customSizeRow = -1;
    }

    bool canCustomize = true;

    const QString family = m_qtFamilies[m_ui->familyListWidget->currentItem()->text()];
    const QString style = m_qtStyles[m_ui->styleListWidget->currentItem()->text()];

    // For Qt-bad-sizes workaround: skip this block unconditionally
    if (!QFontDatabase::isSmoothlyScalable(family, style)) {
        // Bitmap font, allow only discrete sizes.
        // Determine the nearest in the direction of change.
        canCustomize = false;
        int nrows = m_ui->sizeListWidget->count();
        int row = m_ui->sizeListWidget->currentRow();
        int nrow;
        if (val - m_selectedFont.pointSizeF() > 0) {
            for (nrow = row + 1; nrow < nrows; ++nrow) {
                if (QLocale::system().toDouble(m_ui->sizeListWidget->item(nrow)->text()) >= val) {
                    break;
                }
            }
        } else {
            for (nrow = row - 1; nrow >= 0; --nrow) {
                if (QLocale::system().toDouble(m_ui->sizeListWidget->item(nrow)->text()) <= val) {
                    break;
                }
            }
        }
        // Make sure the new row is not out of bounds.
        nrow = nrow < 0 ? 0 : nrow >= nrows ? nrows - 1 : nrow;
        // Get the size from the new row and set the spinbox to that size.
        val = QLocale::system().toDouble(m_ui->sizeListWidget->item(nrow)->text());
        m_ui->sizeSpinBox->setValue(val);
    }

    // Set the current size in the size listbox.
    int row = nearestSizeRow(val, canCustomize);
    m_ui->sizeListWidget->setCurrentRow(row);

    m_selectedSize = val;
    m_selectedFont.setPointSizeF(val);
    Q_EMIT q->fontSelected(m_selectedFont);

    m_signalsAllowed = true;
}

void KFontChooserPrivate::displaySample(const QFont &font)
{
    m_ui->sampleTextEdit->setFont(font);
}

int KFontChooserPrivate::nearestSizeRow(qreal val, bool customize)
{
    qreal diff = 1000;
    int row = 0;
    for (int r = 0; r < m_ui->sizeListWidget->count(); ++r) {
        qreal cval = QLocale::system().toDouble(m_ui->sizeListWidget->item(r)->text());
        if (qAbs(cval - val) < diff) {
            diff = qAbs(cval - val);
            row = r;
        }
    }
    // For Qt-bad-sizes workaround: ignore value of customize, use true
    if (customize && diff > 0) {
        m_customSizeRow = row;
        m_standardSizeAtCustom = m_ui->sizeListWidget->item(row)->text();
        m_ui->sizeListWidget->item(row)->setText(formatFontSize(val));
    }
    return row;
}

qreal KFontChooserPrivate::fillSizeList(const QList<qreal> &sizes_)
{
    if (m_ui->sizeListWidget->isHidden()) {
        qCWarning(KWidgetsAddonsLog) << "Trying to fill the font size listwidget, but the widget is hidden.";
        return 0.0;
    }

    QList<qreal> sizes = sizes_;
    bool canCustomize = false;
    if (sizes.isEmpty()) {
        static const int c[] = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 22, 24, 26, 28, 32, 48, 64, 72, 80, 96, 128, 0};
        for (int i = 0; c[i]; ++i) {
            sizes.append(c[i]);
        }
        // Since sizes were not supplied, this is a vector font,
        // and size slot customization is allowed.
        canCustomize = true;
    }

    // Insert sizes into the listbox.
    m_ui->sizeListWidget->clear();
    std::sort(sizes.begin(), sizes.end());
    for (qreal size : std::as_const(sizes)) {
        m_ui->sizeListWidget->addItem(formatFontSize(size));
    }

    // Return the nearest to selected size.
    // If the font is vector, the nearest size is always same as selected,
    // thus size slot customization is allowed.
    // If the font is bitmap, the nearest size need not be same as selected,
    // thus size slot customization is not allowed.
    m_customSizeRow = -1;
    int row = nearestSizeRow(m_selectedSize, canCustomize);
    return QLocale::system().toDouble(m_ui->sizeListWidget->item(row)->text());
}

qreal KFontChooserPrivate::setupSizeListBox(const QString &family, const QString &style)
{
    QList<qreal> sizes;
    const bool smoothlyScalable = QFontDatabase::isSmoothlyScalable(family, style);
    if (!smoothlyScalable) {
        const QList<int> smoothSizes = QFontDatabase::smoothSizes(family, style);
        for (int size : smoothSizes) {
            sizes.append(size);
        }
    }

    // Fill the listbox (uses default list of sizes if the given is empty).
    // Collect the best fitting size to selected size, to use if not smooth.
    qreal bestFitSize = fillSizeList(sizes);

    // Set the best fit size as current in the listbox if available.
    const QList<QListWidgetItem *> selectedSizeList = m_ui->sizeListWidget->findItems(formatFontSize(bestFitSize), Qt::MatchExactly);
    if (!selectedSizeList.isEmpty()) {
        m_ui->sizeListWidget->setCurrentItem(selectedSizeList.first());
    }

    return bestFitSize;
}

void KFontChooserPrivate::setupDisplay()
{
    qreal size = m_selectedFont.pointSizeF();
    if (size == -1) {
        size = QFontInfo(m_selectedFont).pointSizeF();
    }

    int numEntries;
    int i;

    // Get the styleID here before familyListWidget->setCurrentRow() is called
    // as it may change the font style
    const QString styleID = styleIdentifier(m_selectedFont);

    QString family = m_selectedFont.family().toLower();
    // Direct family match.
    numEntries = m_ui->familyListWidget->count();
    for (i = 0; i < numEntries; ++i) {
        if (family == m_qtFamilies[m_ui->familyListWidget->item(i)->text()].toLower()) {
            m_ui->familyListWidget->setCurrentRow(i);
            break;
        }
    }

    // 1st family fallback.
    if (i == numEntries) {
        const int bracketPos = family.indexOf(QLatin1Char('['));
        if (bracketPos != -1) {
            family = QStringView(family).left(bracketPos).trimmed().toString();
            for (i = 0; i < numEntries; ++i) {
                if (family == m_qtFamilies[m_ui->familyListWidget->item(i)->text()].toLower()) {
                    m_ui->familyListWidget->setCurrentRow(i);
                    break;
                }
            }
        }
    }

    // 2nd family fallback.
    if (i == numEntries) {
        QString fallback = family + QLatin1String(" [");
        for (i = 0; i < numEntries; ++i) {
            if (m_qtFamilies[m_ui->familyListWidget->item(i)->text()].toLower().startsWith(fallback)) {
                m_ui->familyListWidget->setCurrentRow(i);
                break;
            }
        }
    }

    // 3rd family fallback.
    if (i == numEntries) {
        for (i = 0; i < numEntries; ++i) {
            if (m_qtFamilies[m_ui->familyListWidget->item(i)->text()].toLower().startsWith(family)) {
                m_ui->familyListWidget->setCurrentRow(i);
                break;
            }
        }
    }

    // Family fallback in case nothing matched. Otherwise, diff doesn't work
    if (i == numEntries) {
        m_ui->familyListWidget->setCurrentRow(0);
    }

    // By setting the current item in the family box, the available
    // styles and sizes for that family have been collected.
    // Try now to set the current items in the style and size boxes.

    // Set current style in the listbox.
    numEntries = m_ui->styleListWidget->count();
    for (i = 0; i < numEntries; ++i) {
        if (styleID == m_styleIDs[m_ui->styleListWidget->item(i)->text()]) {
            m_ui->styleListWidget->setCurrentRow(i);
            break;
        }
    }
    if (i == numEntries) {
        // Style not found, fallback.
        m_ui->styleListWidget->setCurrentRow(0);
    }

    // Set current size in the listbox.
    // If smoothly scalable, allow customizing one of the standard size slots,
    // otherwise just select the nearest available size.
    const QString currentFamily = m_qtFamilies[m_ui->familyListWidget->currentItem()->text()];
    const QString currentStyle = m_qtStyles[m_ui->styleListWidget->currentItem()->text()];
    const bool canCustomize = QFontDatabase::isSmoothlyScalable(currentFamily, currentStyle);
    m_ui->sizeListWidget->setCurrentRow(nearestSizeRow(size, canCustomize));

    // Set current size in the spinbox.
    m_ui->sizeSpinBox->setValue(QLocale::system().toDouble(m_ui->sizeListWidget->currentItem()->text()));
}

// static
QStringList KFontChooser::createFontList(uint fontListCriteria)
{
    QStringList lstSys(QFontDatabase::families());

    // if we have criteria; then check fonts before adding
    if (fontListCriteria) {
        QStringList lstFonts;
        for (const QString &family : std::as_const(lstSys)) {
            if ((fontListCriteria & FixedWidthFonts) > 0 && !QFontDatabase::isFixedPitch(family)) {
                continue;
            }
            if (((fontListCriteria & (SmoothScalableFonts | ScalableFonts)) == ScalableFonts) && !QFontDatabase::isBitmapScalable(family)) {
                continue;
            }
            if ((fontListCriteria & SmoothScalableFonts) > 0 && !QFontDatabase::isSmoothlyScalable(family)) {
                continue;
            }
            lstFonts.append(family);
        }

        if ((fontListCriteria & FixedWidthFonts) > 0) {
            // Fallback.. if there are no fixed fonts found, it's probably a
            // bug in the font server or Qt.  In this case, just use 'fixed'
            if (lstFonts.isEmpty()) {
                lstFonts.append(QStringLiteral("fixed"));
            }
        }

        lstSys = lstFonts;
    }

    lstSys.sort();

    return lstSys;
}

void KFontChooser::setFontListItems(const QStringList &fontList)
{
    d->setFamilyBoxItems(fontList);
}

void KFontChooserPrivate::setFamilyBoxItems(const QStringList &fonts)
{
    m_signalsAllowed = false;

    m_ui->familyListWidget->clear();

    m_qtFamilies = translateFontNameList(!fonts.isEmpty() ? fonts : KFontChooser::createFontList(m_usingFixed ? KFontChooser::FixedWidthFonts : 0));

    QStringList list;
    list.reserve(m_qtFamilies.size());

    // Generic font names
    const QStringList genericTranslatedNames{
        translateFontName(QStringLiteral("Sans Serif")),
        translateFontName(QStringLiteral("Serif")),
        translateFontName(QStringLiteral("Monospace")),
    };

    // Add generic family names to the top of the list
    for (const QString &s : genericTranslatedNames) {
        auto nIt = m_qtFamilies.find(s);
        if (nIt != m_qtFamilies.cend()) {
            list.push_back(s);
        }
    }

    for (auto it = m_qtFamilies.cbegin(); it != m_qtFamilies.cend(); ++it) {
        const QString &name = it->first;
        if (genericTranslatedNames.contains(name)) {
            continue;
        }

        list.push_back(name);
    }

    m_ui->familyListWidget->addItems(list);
    m_ui->familyListWidget->setMinimumWidth(minimumListWidth(m_ui->familyListWidget));

    m_signalsAllowed = true;
}

void KFontChooser::setMinVisibleItems(int visibleItems)
{
    for (auto *widget : {d->m_ui->familyListWidget, d->m_ui->styleListWidget, d->m_ui->sizeListWidget}) {
        widget->setMinimumHeight(minimumListHeight(widget, visibleItems));
    }
}

// Human-readable style identifiers returned by QFontDatabase::styleString()
// do not always survive round trip of QFont serialization/deserialization,
// causing wrong style in the style box to be highlighted when
// the chooser dialog is opened. This will cause the style to be changed
// when the dialog is closed and the user did not touch the style box.
// Hence, construct custom style identifiers sufficient for the purpose.
QString KFontChooserPrivate::styleIdentifier(const QFont &font)
{
    const int weight = font.weight();
    QString styleName = font.styleName();
    // If the styleName property is empty and the weight is QFont::Normal, that
    // could mean it's a "Regular"-like style with the styleName part stripped
    // so that subsequent calls to setBold(true) can work properly (i.e. selecting
    // the "Bold" style provided by the font itself) without resorting to font
    // "emboldening" which looks ugly.
    // See also KConfigGroupGui::writeEntryGui().
    if (styleName.isEmpty() && weight == QFont::Normal) {
        const QStringList styles = QFontDatabase::styles(font.family());
        for (const QString &style : styles) {
            if (isDefaultFontStyleName(style)) {
                styleName = style;
                break;
            } else {
                // nothing more we can do
            }
        }
    }

    const QChar comma(QLatin1Char(','));
    return QString::number(weight) + comma //
        + QString::number((int)font.style()) + comma //
        + QString::number(font.stretch()) + comma //
        + styleName;
}

#include "moc_kfontchooser.cpp"
