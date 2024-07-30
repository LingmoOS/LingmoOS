/*
    SPDX-FileCopyrightText: 2002-2010 Anders Lund <anders@alweb.dk>

    Rewritten based on code of:
    SPDX-FileCopyrightText: 2002 Michael Goffioul <kdeprint@swing.be>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_PRINT_CONFIG_WIDGETS_H
#define KATE_PRINT_CONFIG_WIDGETS_H

#include <QWidget>

class KColorButton;
class KFontRequester;

class QCheckBox;
class QLabel;
class QLineEdit;
class QComboBox;
class QSpinBox;
class QGroupBox;

namespace KatePrinter
{
// BEGIN Text settings
/*
 *  Text settings page:
 *  - Print Line Numbers
 *    () Smart () Yes () No
 */
class KatePrintTextSettings : public QWidget
{
public:
    explicit KatePrintTextSettings(QWidget *parent = nullptr);
    ~KatePrintTextSettings() override;

    bool printLineNumbers();
    bool printGuide();
    bool dontPrintFoldedCode() const;

private:
    void readSettings();
    void writeSettings();

    QCheckBox *cbLineNumbers;
    QCheckBox *cbGuide;
    QCheckBox *cbFolding;
};
// END Text Settings

// BEGIN Header/Footer
/*
 *  Header & Footer page:
 *  - enable header/footer
 *  - header/footer props
 *    o formats
 *    o colors
 */

class KatePrintHeaderFooter : public QWidget
{
public:
    explicit KatePrintHeaderFooter(QWidget *parent = nullptr);
    ~KatePrintHeaderFooter() override;

    QFont font();

    bool useHeader();
    QStringList headerFormat();
    QColor headerForeground();
    QColor headerBackground();
    bool useHeaderBackground();

    bool useFooter();
    QStringList footerFormat();
    QColor footerForeground();
    QColor footerBackground();
    bool useFooterBackground();

public:
    void showContextMenu(const QPoint &pos);

private:
    void readSettings();
    void writeSettings();

    QCheckBox *cbEnableHeader, *cbEnableFooter;
    KFontRequester *lFontPreview;
    QGroupBox *gbHeader, *gbFooter;
    QLineEdit *leHeaderLeft, *leHeaderCenter, *leHeaderRight;
    KColorButton *kcbtnHeaderFg, *kcbtnHeaderBg;
    QCheckBox *cbHeaderEnableBgColor;
    QLineEdit *leFooterLeft, *leFooterCenter, *leFooterRight;
    KColorButton *kcbtnFooterFg, *kcbtnFooterBg;
    QCheckBox *cbFooterEnableBgColor;
};

// END Header/Footer

// BEGIN Layout
/*
 *  Layout page:
 *  - Color scheme
 *  - Use Box
 *  - Box properties
 *    o Width
 *    o Margin
 *    o Color
 */
class KatePrintLayout : public QWidget
{
public:
    explicit KatePrintLayout(QWidget *parent = nullptr);
    ~KatePrintLayout() override;

    QString colorScheme();
    QFont textFont();
    bool useBackground();
    bool useBox();
    int boxWidth();
    int boxMargin();
    QColor boxColor();

private:
    void readSettings();
    void writeSettings();

    QComboBox *cmbSchema;
    KFontRequester *lFontPreview;
    QCheckBox *cbEnableBox;
    QCheckBox *cbDrawBackground;
    QGroupBox *gbBoxProps;
    QSpinBox *sbBoxWidth;
    QSpinBox *sbBoxMargin;
    KColorButton *kcbtnBoxColor;
};
// END Layout

}

#endif
