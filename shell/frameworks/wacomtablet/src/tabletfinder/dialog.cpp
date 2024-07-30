/*
 * This file is part of the KDE wacomtablet project. For copyright
 * information and license terms see the AUTHORS and COPYING files
 * in the top-level directory of this distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dialog.h"
#include "ui_dialog.h"

#include "hwbuttondialog.h"
#include "tabletdatabase.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <QAbstractButton>
#include <QMap>
#include <QMapIterator>
#include <QProcess>

using namespace Wacom;

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::Dialog)
    , m_hwbDialog(0)
{
    m_ui->setupUi(this);

    connect(m_ui->refreshButton, SIGNAL(clicked()), SLOT(refreshTabletList()));
    connect(m_ui->listTablets, SIGNAL(currentIndexChanged(int)), SLOT(changeTabletSelection(int)));

    connect(m_ui->hasStatusLEDsLeft, SIGNAL(toggled(bool)), SLOT(onHasStatusLEDsLeftChanged(bool)));
    connect(m_ui->hasStatusLEDsRight, SIGNAL(toggled(bool)), SLOT(onhasStatusLEDsRightChanged(bool)));
    connect(m_ui->hasTouchRing, SIGNAL(toggled(bool)), SLOT(onhasTouchRingChanged(bool)));
    connect(m_ui->hasTouchStripLeft, SIGNAL(toggled(bool)), SLOT(onhasTouchStripLeftChanged(bool)));
    connect(m_ui->hasTouchStripRight, SIGNAL(toggled(bool)), SLOT(onhasTouchStripRightChanged(bool)));
    connect(m_ui->hasWheel, SIGNAL(toggled(bool)), SLOT(onhasWheelChanged(bool)));

    connect(m_ui->expressKeyNumbers, SIGNAL(valueChanged(int)), SLOT(onExpressKeyNumbersChanged(int)));

    connect(m_ui->buttonBox, SIGNAL(clicked(QAbstractButton *)), SLOT(buttonBoxClicked(QAbstractButton *)));

    connect(m_ui->mapButtons, SIGNAL(clicked()), SLOT(onMapButtons()));

    connect(m_ui->comboTouchSensor, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &Dialog::onPairedIdChanged);
    connect(m_ui->radioNormalTablet, &QRadioButton::toggled, this, &Dialog::onNormalTabletSet);
    connect(m_ui->radioParentTablet, &QRadioButton::toggled, this, &Dialog::onParentTabletSet);
    connect(m_ui->radioTouchSensor, &QRadioButton::toggled, this, &Dialog::onTouchSensorSet);

    m_ui->mapButtons->setEnabled(false);

    refreshTabletList();
}

Dialog::~Dialog()
{
    delete m_ui;
}

void Dialog::refreshTabletList()
{
    m_ui->listTablets->blockSignals(true);
    m_ui->comboTouchSensor->blockSignals(true);

    m_ui->listTablets->clear();
    m_ui->comboTouchSensor->clear();

    m_tabletList.clear();

    // fetch all connected tablets
    QLatin1String program("xsetwacom");
    QStringList arguments;
    arguments << QLatin1String("--list") << QLatin1String("devices");

    QProcess listDevices;
    listDevices.start(program, arguments);
    listDevices.waitForFinished();

    QString listDevicesString = QString::fromLatin1(listDevices.readAll());
    QStringList listDevicesStringList = listDevicesString.split(QLatin1String("\n"));

    // for each tablet device fetch the device id
    QMap<QString, QStringList> tabletList;
    foreach (const QString &s, listDevicesStringList) {
        QString device = s.split(QLatin1String("\t")).first();
        device = device.trimmed();

        if (device.isEmpty()) {
            continue;
        }

        QProcess checkTabletID;
        arguments.clear();
        arguments << QLatin1String("--get") << device << QLatin1String("TabletID");
        checkTabletID.start(program, arguments);
        checkTabletID.waitForFinished();

        QString checkTabletIDString = QString::fromLatin1(checkTabletID.readAll());
        checkTabletIDString.remove(QLatin1String("\n"));
        QString tabletID = checkTabletIDString.trimmed();

        QStringList tl = tabletList.value(tabletID);
        tl << device;
        tabletList.insert(tabletID, tl);
    }

    // for each device id figure out the common base name
    // for most this is the name without "pad", "stylus", "touch" etc
    // for some like the Wacom Pen and Touch we remove "Finger" as well
    QMapIterator<QString, QStringList> i(tabletList);
    while (i.hasNext()) {
        i.next();

        QString name;
        QStringList deviceList = i.value();
        QString firstDevice = deviceList.takeFirst();
        bool end = false;

        for (int j = 0; j < firstDevice.length(); j++) {
            QChar c = firstDevice.at(j);

            foreach (const QString &s, deviceList) {
                if (s.at(j) != c) {
                    end = true;
                    break;
                }
            }

            if (end) {
                break;
            }

            name.append(c);
        }

        // create the Tablet entry
        Tablet t;
        t.serialID = i.key().toInt();
        t.company = QLatin1String("Wacom");
        t.name = name.trimmed();
        t.devices = i.value();

        QString hexNumber = QString::number(t.serialID, 16);
        while (hexNumber.length() < 4) {
            hexNumber.prepend(QLatin1String("0"));
        }

        TabletInformation ti;
        if (TabletDatabase::instance().lookupTablet(hexNumber.toUpper(), ti)) {
            t.buttonNumber = ti.getInt(TabletInfo::NumPadButtons);
            int leds = ti.getInt(TabletInfo::StatusLEDs);
            t.hasStatusLEDsLeft = leds > 4;
            t.hasStatusLEDsRight = leds > 0;
            t.hasTouchRing = ti.getBool(TabletInfo::HasTouchRing);
            t.hasTouchStripLeft = ti.getBool(TabletInfo::HasLeftTouchStrip);
            t.hasTouchStripRight = ti.getBool(TabletInfo::HasRightTouchStrip);
            t.hasWheel = ti.getBool(TabletInfo::HasWheel);

            t.company = ti.get(TabletInfo::CompanyName);
            t.name = ti.get(TabletInfo::TabletName);

            t.model = ti.get(TabletInfo::TabletModel);
            t.layout = ti.get(TabletInfo::ButtonLayout);

            // set default button map
            for (int i = 1; i <= t.buttonNumber; i++) {
                t.hwMapping << i;
            }

            const auto buttonMap = ti.getButtonMap();
            for (const auto &key : buttonMap.keys()) {
                const auto keyIndex = key.toInt() - 1;
                if (keyIndex < t.buttonNumber) {
                    t.hwMapping[keyIndex] = buttonMap.value(key).toInt();
                }
            }

            auto pairedID = ti.get(TabletInfo::TouchSensorId);
            if (pairedID.isEmpty()) {
                t.hasPairedID = false;
            } else {
                t.hasPairedID = true;
                t.pairedID = pairedID;
            }
            t.isTouchSensor = ti.getBool(TabletInfo::IsTouchSensor);
        }

        m_tabletList.append(t);

        m_ui->listTablets->addItem(t.name);
        m_ui->comboTouchSensor->addItem(t.name);
    }

    m_ui->listTablets->setCurrentIndex(0);
    m_ui->listTablets->blockSignals(false);
    m_ui->comboTouchSensor->setCurrentIndex(0);
    m_ui->comboTouchSensor->blockSignals(false);

    changeTabletSelection(0);
}

bool Dialog::validIndex()
{
    return !m_tabletList.isEmpty() && m_tabletList.size() > m_ui->listTablets->currentIndex();
}

void Dialog::changeTabletSelection(int index)
{
    if (!validIndex()) {
        return;
    }

    Tablet t = m_tabletList.at(index);

    QString hexNumber = QString::number(t.serialID, 16);

    while (hexNumber.length() < 4) {
        hexNumber.prepend(QLatin1String("0"));
    }

    m_ui->listSerialId->setText(QString::fromLatin1("%1 [%2]").arg(t.serialID).arg(hexNumber.toUpper()));

    m_ui->listDevices->setText(t.devices.join(QLatin1String("\n")));
    m_ui->hasStatusLEDsLeft->setChecked(t.hasStatusLEDsLeft);
    m_ui->hasStatusLEDsRight->setChecked(t.hasStatusLEDsRight);
    m_ui->hasTouchRing->setChecked(t.hasTouchRing);
    m_ui->hasTouchStripLeft->setChecked(t.hasTouchStripLeft);
    m_ui->hasTouchStripRight->setChecked(t.hasTouchStripRight);
    m_ui->hasWheel->setChecked(t.hasWheel);
    m_ui->expressKeyNumbers->setValue(t.buttonNumber);
    m_ui->radioNormalTablet->setChecked(!t.isTouchSensor && !t.hasPairedID);
    m_ui->radioTouchSensor->setChecked(t.isTouchSensor);
    m_ui->radioParentTablet->setChecked(t.hasPairedID);
    m_ui->comboTouchSensor->setEnabled(t.hasPairedID);

    int tabletIndex = 0;
    for (const auto &tablet : m_tabletList) {
        if (QString::fromLatin1("%1").arg(tablet.serialID, 4, 16, QChar::fromLatin1('0')) == t.pairedID) {
            m_ui->comboTouchSensor->setCurrentIndex(tabletIndex);
            break;
        }
        ++tabletIndex;
    }

    showHWButtonMap();
}

void Dialog::onHasStatusLEDsLeftChanged(bool toggled)
{
    if (!validIndex()) {
        return;
    }

    Tablet t = m_tabletList.at(m_ui->listTablets->currentIndex());
    t.hasStatusLEDsLeft = toggled;
    m_tabletList.replace(m_ui->listTablets->currentIndex(), t);
}

void Dialog::onhasStatusLEDsRightChanged(bool toggled)
{
    if (!validIndex()) {
        return;
    }

    Tablet t = m_tabletList.at(m_ui->listTablets->currentIndex());
    t.hasStatusLEDsRight = toggled;
    m_tabletList.replace(m_ui->listTablets->currentIndex(), t);
}

void Dialog::onhasTouchRingChanged(bool toggled)
{
    if (!validIndex()) {
        return;
    }

    Tablet t = m_tabletList.at(m_ui->listTablets->currentIndex());
    t.hasTouchRing = toggled;
    m_tabletList.replace(m_ui->listTablets->currentIndex(), t);
}

void Dialog::onhasTouchStripLeftChanged(bool toggled)
{
    if (!validIndex()) {
        return;
    }

    Tablet t = m_tabletList.at(m_ui->listTablets->currentIndex());
    t.hasTouchStripLeft = toggled;
    m_tabletList.replace(m_ui->listTablets->currentIndex(), t);
}

void Dialog::onhasTouchStripRightChanged(bool toggled)
{
    if (!validIndex()) {
        return;
    }

    Tablet t = m_tabletList.at(m_ui->listTablets->currentIndex());
    t.hasTouchStripRight = toggled;
    m_tabletList.replace(m_ui->listTablets->currentIndex(), t);
}

void Dialog::onhasWheelChanged(bool toggled)
{
    if (!validIndex()) {
        return;
    }

    Tablet t = m_tabletList.at(m_ui->listTablets->currentIndex());
    t.hasWheel = toggled;
    m_tabletList.replace(m_ui->listTablets->currentIndex(), t);
}

void Dialog::onExpressKeyNumbersChanged(int buttons)
{
    if (!validIndex()) {
        return;
    }

    Tablet t = m_tabletList.at(m_ui->listTablets->currentIndex());
    t.buttonNumber = buttons;
    m_tabletList.replace(m_ui->listTablets->currentIndex(), t);

    m_ui->mapButtons->setEnabled(m_ui->expressKeyNumbers->value() > 0);

    showHWButtonMap();
}

void Dialog::buttonBoxClicked(QAbstractButton *button)
{
    QDialogButtonBox::StandardButton stdButton = m_ui->buttonBox->standardButton(button);
    switch (stdButton) {
    case QDialogButtonBox::Close:
        qApp->quit();
        break;

    case QDialogButtonBox::Save: {
        if (!validIndex()) {
            return;
        }

        Tablet t = m_tabletList.at(m_ui->listTablets->currentIndex());
        saveTabletInfo(t);
        break;
    }
    case QDialogButtonBox::SaveAll: {
        foreach (const Tablet &t, m_tabletList) {
            saveTabletInfo(t);
        }
        break;
    }
    default:
        return;
    }
}

void Dialog::onPairedIdChanged(int index)
{
    Tablet t = m_tabletList.at(m_ui->listTablets->currentIndex());
    t.pairedID = QString::fromLatin1("%1").arg(m_tabletList.at(index).serialID, 4, 16, QChar::fromLatin1('0'));
    m_tabletList.replace(m_ui->listTablets->currentIndex(), t);
}

void Dialog::onNormalTabletSet(bool enabled)
{
    if (enabled) {
        const int index = m_ui->listTablets->currentIndex();
        if (index != -1) {
            Tablet t = m_tabletList.at(index);
            t.isTouchSensor = false;
            t.hasPairedID = false;
            m_tabletList.replace(index, t);
            m_ui->comboTouchSensor->setEnabled(false);
        }
    }
}

void Dialog::onParentTabletSet(bool enabled)
{
    const int index = m_ui->listTablets->currentIndex();
    if (enabled && index != -1) {
        Tablet t = m_tabletList.at(index);
        t.isTouchSensor = false;
        t.hasPairedID = true;
        m_tabletList.replace(index, t);
        m_ui->comboTouchSensor->setEnabled(true);
    } else {
        m_ui->comboTouchSensor->setEnabled(false);
    }
}

void Dialog::onTouchSensorSet(bool enabled)
{
    const int index = m_ui->listTablets->currentIndex();
    if (enabled && index != -1) {
        Tablet t = m_tabletList.at(index);
        t.isTouchSensor = true;
        t.hasPairedID = false;
        m_tabletList.replace(index, t);
        m_ui->comboTouchSensor->setEnabled(false);
    }
}

void Dialog::onMapButtons()
{
    m_hwbDialog = new HWButtonDialog(m_ui->expressKeyNumbers->value(), this);

    int ret = m_hwbDialog->exec();

    Tablet t = m_tabletList.at(m_ui->listTablets->currentIndex());
    if (ret == QDialog::Accepted) {
        t.hwMapping = m_hwbDialog->buttonMap();
    } else {
        t.hwMapping.clear();
    }

    m_tabletList.replace(m_ui->listTablets->currentIndex(), t);
    showHWButtonMap();
}

void Dialog::showHWButtonMap()
{
    Tablet t = m_tabletList.at(m_ui->listTablets->currentIndex());

    QString text;
    for (int i = 0; i < t.hwMapping.size(); ++i) {
        text.append(i18n("Button %1 maps to Button %2", (i + 1), t.hwMapping.at(i)));
        text.append(QLatin1String("\n"));
    }

    m_ui->buttonMappingLabel->setText(text);
}

void Dialog::saveTabletInfo(const Tablet &t)
{
    KConfig config(QLatin1String("tabletdblocalrc"));

    QString hexNumber = QString::number(t.serialID, 16);

    while (hexNumber.length() < 4) {
        hexNumber.prepend(QLatin1String("0"));
    }
    KConfigGroup tabletGroup(&config, hexNumber.toUpper());
    tabletGroup.deleteGroup();
    tabletGroup.config()->sync();

    tabletGroup.writeEntry("model", t.model);
    tabletGroup.writeEntry("layout", t.layout);
    tabletGroup.writeEntry("name", t.name);

    int leds = 0;
    if (t.hasStatusLEDsLeft && t.hasStatusLEDsRight) {
        leds = 8;
    } else if (t.hasStatusLEDsLeft || t.hasStatusLEDsRight) {
        leds = 4;
    }

    tabletGroup.writeEntry("statusleds", leds);
    tabletGroup.writeEntry("wheel", t.hasWheel);
    tabletGroup.writeEntry("touchring", t.hasTouchRing);
    tabletGroup.writeEntry("touchstripl", t.hasTouchStripLeft);
    tabletGroup.writeEntry("touchstripr", t.hasTouchStripRight);

    tabletGroup.writeEntry("padbuttons", t.buttonNumber);
    for (int i = 0; i < t.hwMapping.size(); i++) {
        tabletGroup.writeEntry(QString::fromLatin1("hwbutton%1").arg(i + 1), t.hwMapping.at(i));
    }

    if (t.isTouchSensor) {
        tabletGroup.writeEntry("istouchsensor", t.isTouchSensor);
    }
    if (t.hasPairedID) {
        tabletGroup.writeEntry("touchsensorid", t.pairedID);
    }

    tabletGroup.config()->sync();
}

#include "moc_dialog.cpp"
