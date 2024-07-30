/*
    SPDX-FileCopyrightText: 2011-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "variableeditor.h"
#include "variableitem.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFontComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QSpinBox>
#include <QToolButton>
#include <QVariant>

#include <KColorCombo>
#include <KHelpClient>
#include <KLocalizedString>
#include <sonnet/dictionarycombobox.h>

class KateHelpButton : public QToolButton
{
public:
    enum IconState { IconColored = 0, IconHidden };

public:
    explicit KateHelpButton(QWidget *parent)
        : QToolButton(parent)
    {
        setAutoRaise(true);
        setIconState(IconColored);
        setToolTip(i18n("Kate Handbook."));

        connect(this, &KateHelpButton::clicked, this, &KateHelpButton::invokeHelp);
    }

public:
    void setSection(const QString &section)
    {
        m_section = section;
    }

    void setIconState(IconState state)
    {
        if (state == IconHidden) {
            setIcon(QIcon());
        } else {
            setIcon(QIcon::fromTheme(QStringLiteral("help-contents")));
        }

        update();
    }
    void invokeHelp()
    {
        KHelpClient::invokeHelp(m_section, QStringLiteral("kate"));
    }

private:
    QString m_section;
};

// BEGIN VariableEditor
VariableEditor::VariableEditor(VariableItem *item, QWidget *parent)
    : QWidget(parent)
    , m_item(item)
{
    setAttribute(Qt::WA_Hover);

    setAutoFillBackground(true);
    QGridLayout *l = new QGridLayout(this);
    l->setContentsMargins(10, 10, 10, 10);

    m_checkBox = new QCheckBox(this);
    m_variable = new QLabel(item->variable(), this);
    m_variable->setFocusPolicy(Qt::ClickFocus);
    m_variable->setFocusProxy(m_checkBox);
    m_btnHelp = new KateHelpButton(this);
    m_btnHelp->setIconState(KateHelpButton::IconHidden);
    m_btnHelp->setEnabled(false);
    m_btnHelp->setSection(QLatin1String("variable-") + item->variable());

    m_helpText = new QLabel(item->helpText(), this);
    m_helpText->setWordWrap(true);

    l->addWidget(m_checkBox, 0, 0, Qt::AlignLeft);
    l->addWidget(m_variable, 0, 1, Qt::AlignLeft);
    l->addWidget(m_btnHelp, 0, 3, Qt::AlignRight);
    l->addWidget(m_helpText, 1, 1, 1, 3);

    l->setColumnStretch(0, 0);
    l->setColumnStretch(1, 1);
    l->setColumnStretch(2, 1);
    l->setColumnStretch(3, 0);

    connect(m_checkBox, &QCheckBox::toggled, this, &VariableEditor::itemEnabled);
    m_checkBox->setChecked(item->isActive());

    connect(m_checkBox, &QCheckBox::toggled, this, &VariableEditor::valueChanged);
    setMouseTracking(true);
}

void VariableEditor::enterEvent(QEnterEvent *event)
{
    QWidget::enterEvent(event);
    m_btnHelp->setIconState(KateHelpButton::IconColored);
    m_btnHelp->setEnabled(true);

    update();
}

void VariableEditor::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    m_btnHelp->setIconState(KateHelpButton::IconHidden);
    m_btnHelp->setEnabled(false);

    update();
}

void VariableEditor::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    // draw highlighting rect like in plasma
    if (underMouse()) {
        QPainter painter(this);

        painter.setRenderHint(QPainter::Antialiasing);

        QColor cornerColor = palette().color(QPalette::Highlight);
        cornerColor.setAlphaF(0.2);

        QColor midColor = palette().color(QPalette::Highlight);
        midColor.setAlphaF(0.5);

        QRect highlightRect = rect().adjusted(2, 2, -2, -2);

        QPen outlinePen;
        outlinePen.setWidth(2);

        QLinearGradient gradient(highlightRect.topLeft(), highlightRect.topRight());
        gradient.setColorAt(0, cornerColor);
        gradient.setColorAt(0.3, midColor);
        gradient.setColorAt(1, cornerColor);
        outlinePen.setBrush(gradient);
        painter.setPen(outlinePen);

        const int radius = 5;
        painter.drawRoundedRect(highlightRect, radius, radius);
    }
}

void VariableEditor::itemEnabled(bool enabled)
{
    if (enabled) {
        m_variable->setText(QLatin1String("<b>") + m_item->variable() + QLatin1String("</b>"));
    } else {
        m_variable->setText(m_item->variable());
    }
    m_item->setActive(enabled);
}

void VariableEditor::activateItem()
{
    m_checkBox->setChecked(true);
}

VariableItem *VariableEditor::item() const
{
    return m_item;
}
// END VariableEditor

// BEGIN VariableUintEditor
VariableIntEditor::VariableIntEditor(VariableIntItem *item, QWidget *parent)
    : VariableEditor(item, parent)
{
    QGridLayout *l = (QGridLayout *)layout();

    m_spinBox = new QSpinBox(this);
    m_spinBox->setValue(item->value());
    m_spinBox->setMinimum(item->minValue());
    m_spinBox->setMaximum(item->maxValue());

    l->addWidget(m_spinBox, 0, 2, Qt::AlignLeft);

    connect(m_spinBox, &QSpinBox::valueChanged, this, &VariableIntEditor::valueChanged);
    connect(m_spinBox, &QSpinBox::valueChanged, this, &VariableIntEditor::activateItem);
    connect(m_spinBox, &QSpinBox::valueChanged, this, &VariableIntEditor::setItemValue);
}

void VariableIntEditor::setItemValue(int newValue)
{
    static_cast<VariableIntItem *>(item())->setValue(newValue);
}
// END VariableUintEditor

// BEGIN VariableBoolEditor
VariableBoolEditor::VariableBoolEditor(VariableBoolItem *item, QWidget *parent)
    : VariableEditor(item, parent)
{
    QGridLayout *l = (QGridLayout *)layout();

    m_comboBox = new QComboBox(this);
    m_comboBox->addItem(i18n("true"));
    m_comboBox->addItem(i18n("false"));
    m_comboBox->setCurrentIndex(item->value() ? 0 : 1);
    l->addWidget(m_comboBox, 0, 2, Qt::AlignLeft);

    connect(m_comboBox, &QComboBox::currentIndexChanged, this, &VariableBoolEditor::valueChanged);
    connect(m_comboBox, &QComboBox::currentIndexChanged, this, &VariableBoolEditor::activateItem);
    connect(m_comboBox, &QComboBox::currentIndexChanged, this, &VariableBoolEditor::setItemValue);
}

void VariableBoolEditor::setItemValue(int enabled)
{
    static_cast<VariableBoolItem *>(item())->setValue(enabled == 0);
}
// END VariableBoolEditor

// BEGIN VariableStringListEditor
VariableStringListEditor::VariableStringListEditor(VariableStringListItem *item, QWidget *parent)
    : VariableEditor(item, parent)
{
    QGridLayout *l = (QGridLayout *)layout();

    m_comboBox = new QComboBox(this);
    m_comboBox->addItems(item->stringList());
    int index = 0;
    for (int i = 0; i < item->stringList().size(); ++i) {
        if (item->stringList().at(i) == item->value()) {
            index = i;
            break;
        }
    }
    m_comboBox->setCurrentIndex(index);
    l->addWidget(m_comboBox, 0, 2, Qt::AlignLeft);

    connect(m_comboBox, &QComboBox::currentIndexChanged, this, &VariableStringListEditor::valueChanged);
    connect(m_comboBox, &QComboBox::currentIndexChanged, this, &VariableStringListEditor::activateItem);
    connect(m_comboBox, &QComboBox::currentTextChanged, this, &VariableStringListEditor::setItemValue);
}

void VariableStringListEditor::setItemValue(const QString &newValue)
{
    static_cast<VariableStringListItem *>(item())->setValue(newValue);
}
// END VariableStringListEditor

// BEGIN VariableColorEditor
VariableColorEditor::VariableColorEditor(VariableColorItem *item, QWidget *parent)
    : VariableEditor(item, parent)
{
    QGridLayout *l = (QGridLayout *)layout();

    m_comboBox = new KColorCombo(this);
    m_comboBox->setColor(item->value());
    l->addWidget(m_comboBox, 0, 2, Qt::AlignLeft);

    connect(m_comboBox, &KColorCombo::activated, this, &VariableColorEditor::valueChanged);
    connect(m_comboBox, &KColorCombo::activated, this, &VariableColorEditor::activateItem);
    connect(m_comboBox, &KColorCombo::activated, this, &VariableColorEditor::setItemValue);
}

void VariableColorEditor::setItemValue(const QColor &newValue)
{
    static_cast<VariableColorItem *>(item())->setValue(newValue);
}
// END VariableColorEditor

// BEGIN VariableFontEditor
VariableFontEditor::VariableFontEditor(VariableFontItem *item, QWidget *parent)
    : VariableEditor(item, parent)
{
    QGridLayout *l = (QGridLayout *)layout();

    m_comboBox = new QFontComboBox(this);
    m_comboBox->setCurrentFont(item->value());
    l->addWidget(m_comboBox, 0, 2, Qt::AlignLeft);

    connect(m_comboBox, &QFontComboBox::currentFontChanged, this, &VariableFontEditor::valueChanged);
    connect(m_comboBox, &QFontComboBox::currentFontChanged, this, &VariableFontEditor::activateItem);
    connect(m_comboBox, &QFontComboBox::currentFontChanged, this, &VariableFontEditor::setItemValue);
}

void VariableFontEditor::setItemValue(const QFont &newValue)
{
    static_cast<VariableFontItem *>(item())->setValue(newValue);
}
// END VariableFontEditor

// BEGIN VariableStringEditor
VariableStringEditor::VariableStringEditor(VariableStringItem *item, QWidget *parent)
    : VariableEditor(item, parent)
{
    QGridLayout *l = (QGridLayout *)layout();

    m_lineEdit = new QLineEdit(this);
    m_lineEdit->setText(item->value());
    l->addWidget(m_lineEdit, 0, 2, Qt::AlignLeft);

    connect(m_lineEdit, &QLineEdit::textChanged, this, &VariableStringEditor::valueChanged);
    connect(m_lineEdit, &QLineEdit::textChanged, this, &VariableStringEditor::activateItem);
    connect(m_lineEdit, &QLineEdit::textChanged, this, &VariableStringEditor::setItemValue);
}

void VariableStringEditor::setItemValue(const QString &newValue)
{
    static_cast<VariableStringItem *>(item())->setValue(newValue);
}
// END VariableStringEditor

// BEGIN VariableSpellCheckEditor
VariableSpellCheckEditor::VariableSpellCheckEditor(VariableSpellCheckItem *item, QWidget *parent)
    : VariableEditor(item, parent)
{
    QGridLayout *l = (QGridLayout *)layout();

    m_dictionaryCombo = new Sonnet::DictionaryComboBox(this);
    m_dictionaryCombo->setCurrentByDictionary(item->value());
    l->addWidget(m_dictionaryCombo, 0, 2, Qt::AlignLeft);

    connect(m_dictionaryCombo, &Sonnet::DictionaryComboBox::dictionaryNameChanged, this, &VariableSpellCheckEditor::valueChanged);
    connect(m_dictionaryCombo, &Sonnet::DictionaryComboBox::dictionaryNameChanged, this, &VariableSpellCheckEditor::activateItem);
    connect(m_dictionaryCombo, &Sonnet::DictionaryComboBox::dictionaryChanged, this, &VariableSpellCheckEditor::setItemValue);
}

void VariableSpellCheckEditor::setItemValue(const QString &newValue)
{
    static_cast<VariableSpellCheckItem *>(item())->setValue(newValue);
}

// END VariableSpellCheckEditor

// BEGIN VariableRemoveSpacesEditor
VariableRemoveSpacesEditor::VariableRemoveSpacesEditor(VariableRemoveSpacesItem *item, QWidget *parent)
    : VariableEditor(item, parent)
{
    QGridLayout *l = (QGridLayout *)layout();

    m_comboBox = new QComboBox(this);
    m_comboBox->addItem(i18nc("value for variable remove-trailing-spaces", "none"));
    m_comboBox->addItem(i18nc("value for variable remove-trailing-spaces", "modified"));
    m_comboBox->addItem(i18nc("value for variable remove-trailing-spaces", "all"));
    m_comboBox->setCurrentIndex(item->value());
    l->addWidget(m_comboBox, 0, 2, Qt::AlignLeft);

    connect(m_comboBox, &QComboBox::currentIndexChanged, this, &VariableRemoveSpacesEditor::valueChanged);
    connect(m_comboBox, &QComboBox::currentIndexChanged, this, &VariableRemoveSpacesEditor::activateItem);
    connect(m_comboBox, &QComboBox::currentIndexChanged, this, &VariableRemoveSpacesEditor::setItemValue);
}

void VariableRemoveSpacesEditor::setItemValue(int enabled)
{
    static_cast<VariableRemoveSpacesItem *>(item())->setValue(enabled == 0);
}
// END VariableRemoveSpacesEditor

#include "moc_variableeditor.cpp"
