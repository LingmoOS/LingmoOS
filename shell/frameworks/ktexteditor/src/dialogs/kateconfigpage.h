/*
    SPDX-FileCopyrightText: 2019 KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_CONFIG_PAGE_H
#define KATE_CONFIG_PAGE_H

#include <ktexteditor/configpage.h>

class KFontRequester;
class KUrlRequester;
class QAbstractButton;
class QAbstractSlider;
class QSpinBox;
class QComboBox;
class QGroupBox;
class QLineEdit;
class QDoubleSpinBox;

class KateConfigPage : public KTextEditor::ConfigPage
{
public:
    explicit KateConfigPage(QWidget *parent = nullptr, const char *name = nullptr);
    ~KateConfigPage() override;
    virtual void reload() = 0;

public:
    bool hasChanged()
    {
        return m_changed;
    }

    void observeChanges(KateConfigPage *page) const;
    void observeChanges(KUrlRequester *requester) const;
    void observeChanges(QAbstractButton *button) const;
    void observeChanges(QAbstractSlider *slider) const;
    void observeChanges(QSpinBox *spinBox) const;
    void observeChanges(QDoubleSpinBox *spinBox) const;
    void observeChanges(QComboBox *comboBox) const;
    void observeChanges(QGroupBox *groupBox) const;
    void observeChanges(QLineEdit *lineEdit) const;
    void observeChanges(KFontRequester *chooser) const;

protected:
    void slotChanged();

private:
    void somethingHasChanged();

protected:
    bool m_changed = false;
};

#endif
