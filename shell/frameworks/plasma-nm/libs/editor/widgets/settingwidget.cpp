/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "settingwidget.h"
#include "bssidcombobox.h"
#include "hwaddrcombobox.h"
#include "passwordfield.h"
#include "ssidcombobox.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTableView>

#include <KUrlRequester>

SettingWidget::SettingWidget(const NetworkManager::Setting::Ptr &setting, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
    , m_type(setting->name())
{
}

SettingWidget::SettingWidget(const NetworkManager::Setting::Ptr &setting, const QStringList &hints, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
    , m_hints(hints)
    , m_type(setting->name())
{
}

SettingWidget::~SettingWidget() = default;

void SettingWidget::loadConfig(const NetworkManager::Setting::Ptr &setting)
{
    Q_UNUSED(setting);
}

void SettingWidget::loadSecrets(const NetworkManager::Setting::Ptr &setting)
{
    Q_UNUSED(setting);
}

void SettingWidget::watchChangedSetting()
{
    // Attempt to connect to all widgets representing various configurations
    // to notify about setting change

    /************ Qt Widgets ************/

    // Connect all QLineEdit widgets
    const QList<QLineEdit *> lineEdits = findChildren<QLineEdit *>();
    for (QLineEdit *lineedit : lineEdits) {
        connect(lineedit, &QLineEdit::textChanged, this, &SettingWidget::settingChanged);
    }

    // Connect all QComboBox widgets
    const QList<QComboBox *> comboboxes = findChildren<QComboBox *>();
    for (QComboBox *combobox : comboboxes) {
        connect(combobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingWidget::settingChanged);
        connect(combobox, &QComboBox::currentTextChanged, this, &SettingWidget::settingChanged);
    }

    // Connect all QCheckBox widgets
    const QList<QCheckBox *> checkboxes = findChildren<QCheckBox *>();
    for (QCheckBox *checkbox : checkboxes) {
        connect(checkbox, &QCheckBox::stateChanged, this, &SettingWidget::settingChanged);
    }

    // Connect all QPushButton widgets
    const QList<QPushButton *> pushbuttons = findChildren<QPushButton *>();
    for (QPushButton *pushbutton : pushbuttons) {
        connect(pushbutton, &QPushButton::clicked, this, &SettingWidget::settingChanged);
    }

    // Connect all QSpinBox widgets
    const QList<QSpinBox *> spinboxes = findChildren<QSpinBox *>();
    for (QSpinBox *spinbox : spinboxes) {
        connect(spinbox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingWidget::settingChanged);
    }

    // Connect all KUrlRequester widgets
    const QList<KUrlRequester *> urlrequesters = findChildren<KUrlRequester *>();
    for (KUrlRequester *urlrequester : urlrequesters) {
        connect(urlrequester, &KUrlRequester::textChanged, this, &SettingWidget::settingChanged);
        connect(urlrequester, &KUrlRequester::urlSelected, this, &SettingWidget::settingChanged);
    }

    // Connect all QTableView widgets
    const QList<QTableView *> tableviews = findChildren<QTableView *>();
    for (QTableView *tableview : tableviews) {
        connect(tableview, &QTableView::clicked, this, &SettingWidget::settingChanged);
    }

    // Connect all QGroupBox widgets
    const QList<QGroupBox *> groupBoxes = findChildren<QGroupBox *>();
    for (QGroupBox *box : groupBoxes) {
        connect(box, &QGroupBox::toggled, this, &SettingWidget::settingChanged);
    }

    /********** OUR CUSTOM WIDGETS **********/
    // Connect all PasswordField widgets
    const QList<PasswordField *> passwordfields = findChildren<PasswordField *>();
    for (PasswordField *passwordfield : passwordfields) {
        connect(passwordfield, &PasswordField::textChanged, this, &SettingWidget::settingChanged);
        connect(passwordfield, &PasswordField::passwordOptionChanged, this, &SettingWidget::settingChanged);
    }

    // Connect all HwAddrComboBox widgets
    const QList<HwAddrComboBox *> hwAddrcomboboxes = findChildren<HwAddrComboBox *>();
    for (HwAddrComboBox *combobox : hwAddrcomboboxes) {
        connect(combobox, &HwAddrComboBox::hwAddressChanged, this, &SettingWidget::settingChanged);
    }

    // Connect all SssidComboBox widgets
    const QList<SsidComboBox *> ssidcomboboxes = findChildren<SsidComboBox *>();
    for (SsidComboBox *combobox : ssidcomboboxes) {
        connect(combobox, &SsidComboBox::ssidChanged, this, &SettingWidget::settingChanged);
    }

    // Connect all BssidComboBox widgets
    const QList<BssidComboBox *> bssidcomboboxes = findChildren<BssidComboBox *>();
    for (BssidComboBox *combobox : bssidcomboboxes) {
        connect(combobox, &BssidComboBox::bssidChanged, this, &SettingWidget::settingChanged);
    }
}

QString SettingWidget::type() const
{
    return m_type;
}

void SettingWidget::slotWidgetChanged()
{
    Q_EMIT validChanged(isValid());
}

#include "moc_settingwidget.cpp"
