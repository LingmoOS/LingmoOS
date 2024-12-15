// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "console.h"
#include "dpmsclient.h"

#include <QCollator>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QRect>
#include <QStandardPaths>
#include <QTimer>

#include <kscreen/config.h>
#include <kscreen/configoperation.h>
#include <kscreen/edid.h>
#include <kscreen/getconfigoperation.h>
#include <kscreen/output.h>
#include <kscreen/setconfigoperation.h>

static QTextStream cout(stdout);
static QTextStream cerr(stderr);

const static QString green = QStringLiteral("\033[01;32m");
const static QString red = QStringLiteral("\033[01;31m");
const static QString yellow = QStringLiteral("\033[01;33m");
const static QString blue = QStringLiteral("\033[01;34m");
const static QString bold = QStringLiteral("\033[01;39m");
const static QString cr = QStringLiteral("\033[0;0m");

namespace KScreen
{
namespace ConfigSerializer
{
// Exported private symbol in configserializer_p.h in KScreen
extern QJsonObject serializeConfig(const KScreen::ConfigPtr &config);
}
}

using namespace KScreen;

Console::Console(QObject *parent)
    : QObject(parent)
    , m_config(nullptr)
    , m_changed(false)
    , m_dpmsClient(nullptr)
{
}

Console::~Console()
{
}

void Console::start(QCommandLineParser *parser)
{
    m_parser = parser;
    if (parser->isSet(QStringLiteral("json")) || parser->isSet(QStringLiteral("outputs")) || !m_outputArgs.isEmpty()) {
        KScreen::GetConfigOperation *op = new KScreen::GetConfigOperation();
        connect(op, &KScreen::GetConfigOperation::finished, this, [this](KScreen::ConfigOperation *op) {
            configReceived(op);
        });
        return;
    }

    if (m_parser->isSet(QStringLiteral("dpms"))) {
        if (!QGuiApplication::platformName().startsWith(QLatin1String("wayland"))) {
            cerr << "DPMS is only supported on Wayland." << Qt::endl;
            // We need to kick the event loop, otherwise .quit() hangs
            QTimer::singleShot(0, qApp->quit);
            return;
        }

        m_dpmsClient = new DpmsClient(this);
        if (m_parser->isSet(QStringLiteral("dpms-excluded"))) {
            const auto excludedConnectors = m_parser->values(QStringLiteral("dpms-excluded"));
            m_dpmsClient->setExcludedOutputNames(excludedConnectors);
        }

        connect(m_dpmsClient, &DpmsClient::finished, qApp, &QCoreApplication::quit);

        const QString dpmsArg = m_parser->value(QStringLiteral("dpms"));
        if (dpmsArg == QLatin1String("show")) {
            showDpms();
        } else {
            setDpms(dpmsArg);
        }
        return;
    }

    // We need to kick the event loop, otherwise .quit() hangs
    QTimer::singleShot(0, qApp->quit);
}

void KScreen::Console::setDpms(const QString &dpmsArg)
{
    qDebug() << "SetDpms: " << dpmsArg;
    connect(m_dpmsClient, &DpmsClient::ready, this, [this, dpmsArg]() {
        cout << "DPMS.ready()";
        if (dpmsArg == QLatin1String("off")) {
            m_dpmsClient->off();
        } else if (dpmsArg == QLatin1String("on")) {
            m_dpmsClient->on();
        } else {
            cout << "--dpms argument not understood (" << dpmsArg << ")";
        }
    });

    m_dpmsClient->connect();
}

void Console::showDpms()
{
    m_dpmsClient = new DpmsClient(this);

    connect(m_dpmsClient, &DpmsClient::ready, this, []() {
        cout << "DPMS.ready()";
    });

    m_dpmsClient->connect();
}

void Console::setOptionList(const QStringList &outputArgs)
{
    m_outputArgs = outputArgs;
}

void Console::parseOutputArgs()
{
    // qDebug() << "POSARGS" << m_positionalArgs;
    for (const QString &op : qAsConst(m_outputArgs)) {
        auto ops = op.split(QLatin1Char('.'));
        if (ops.count() > 2) {
            bool ok;
            int output_id = -1;
            if (ops[0] == QLatin1String("output")) {
                for (const auto &output : m_config->outputs()) {
                    if (output->name() == ops[1]) {
                        output_id = output->id();
                    }
                }
                if (output_id == -1) {
                    output_id = ops[1].toInt(&ok);
                    if (!ok) {
                        cerr << "Unable to parse output id: " << ops[1] << Qt::endl;
                        qApp->exit(3);
                        return;
                    }
                }
                if (ops.count() == 3 && ops[2] == QLatin1String("primary")) {
                    if (!setPrimary(output_id)) {
                        qApp->exit(1);
                        return;
                    };
                } else if (ops.count() == 3 && ops[2] == QLatin1String("enable")) {
                    if (!setEnabled(output_id, true)) {
                        qApp->exit(1);
                        return;
                    };
                } else if (ops.count() == 3 && ops[2] == QLatin1String("disable")) {
                    if (!setEnabled(output_id, false)) {
                        qApp->exit(1);
                        return;
                    };
                } else if (ops.count() == 4 && ops[2] == QLatin1String("mode")) {
                    QString mode_id = ops[3];
                    // set mode
                    if (!setMode(output_id, mode_id)) {
                        qApp->exit(9);
                        return;
                    }
                    qDebug() << "Output" << output_id << "set mode" << mode_id;

                } else if (ops.count() == 4 && ops[2] == QLatin1String("position")) {
                    QStringList _pos = ops[3].split(QLatin1Char(','));
                    if (_pos.count() != 2) {
                        qWarning() << "Invalid position:" << ops[3];
                        qApp->exit(5);
                        return;
                    }
                    int x = _pos[0].toInt(&ok);
                    int y = _pos[1].toInt(&ok);
                    if (!ok) {
                        cerr << "Unable to parse position: " << ops[3] << Qt::endl;
                        qApp->exit(5);
                        return;
                    }

                    QPoint p(x, y);
                    qDebug() << "Output position" << p;
                    if (!setPosition(output_id, p)) {
                        qApp->exit(1);
                        return;
                    }
                } else if ((ops.count() == 4 || ops.count() == 5) && ops[2] == QLatin1String("scale")) {
                    // be lenient about . vs. comma as separator
                    qreal scale = ops[3].replace(QLatin1Char(','), QLatin1Char('.')).toDouble(&ok);
                    if (ops.count() == 5) {
                        const QString dbl = ops[3] + QStringLiteral(".") + ops[4];
                        scale = dbl.toDouble(&ok);
                    };
                    // set scale
                    if (!ok || qFuzzyCompare(scale, 0.0) || !setScale(output_id, scale)) {
                        qDebug() << "Could not set scale " << scale << " to output " << output_id;
                        qApp->exit(9);
                        return;
                    }
                } else if ((ops.count() == 4) && (ops[2] == QLatin1String("orientation") || ops[2] == QStringLiteral("rotation"))) {
                    const QString _rotation = ops[3].toLower();
                    bool ok = false;
                    const QHash<QString, KScreen::Output::Rotation> rotationMap({{QStringLiteral("none"), KScreen::Output::None},
                                                                                 {QStringLiteral("normal"), KScreen::Output::None},
                                                                                 {QStringLiteral("left"), KScreen::Output::Left},
                                                                                 {QStringLiteral("right"), KScreen::Output::Right},
                                                                                 {QStringLiteral("inverted"), KScreen::Output::Inverted}});
                    KScreen::Output::Rotation rot = KScreen::Output::None;
                    // set orientation
                    if (rotationMap.contains(_rotation)) {
                        ok = true;
                        rot = rotationMap[_rotation];
                    }
                    if (!ok || !setRotation(output_id, rot)) {
                        qDebug() << "Could not set orientation " << _rotation << " to output " << output_id;
                        qApp->exit(9);
                        return;
                    }
                } else if (ops.count() == 4 && ops[2] == QLatin1String("overscan")) {
                    const uint32_t overscan = ops[3].toInt();
                    if (overscan > 100) {
                        qWarning() << "Wrong input: allowed values for overscan are from 0 to 100";
                        qApp->exit(9);
                        return;
                    }
                    if (!setOverscan(output_id, overscan)) {
                        qDebug() << "Could not set overscan " << overscan << " to output " << output_id;
                        qApp->exit(9);
                        return;
                    }
                } else if (ops.count() == 4 && ops[2] == QLatin1String("vrrpolicy")) {
                    const QString _policy = ops[3].toLower();
                    KScreen::Output::VrrPolicy policy;
                    if (_policy == QStringLiteral("never")) {
                        policy = KScreen::Output::VrrPolicy::Never;
                    } else if (_policy == QStringLiteral("always")) {
                        policy = KScreen::Output::VrrPolicy::Always;
                    } else if (_policy == QStringLiteral("automatic")) {
                        policy = KScreen::Output::VrrPolicy::Automatic;
                    } else {
                        qDebug() << "Wrong input: Only allowed values are \"never\", \"always\" and \"automatic\"";
                        qApp->exit(9);
                        return;
                    }
                    if (!setVrrPolicy(output_id, policy)) {
                        qDebug() << "Could not set vrr policy " << policy << " to output " << output_id;
                        qApp->exit(9);
                        return;
                    }
                } else if (ops.count() == 4 && ops[2] == QLatin1String("rgbrange")) {
                    const QString _range = ops[3].toLower();
                    KScreen::Output::RgbRange range;
                    if (_range == QStringLiteral("automatic")) {
                        range = KScreen::Output::RgbRange::Automatic;
                    } else if (_range == QStringLiteral("full")) {
                        range = KScreen::Output::RgbRange::Full;
                    } else if (_range == QStringLiteral("limited")) {
                        range = KScreen::Output::RgbRange::Limited;
                    } else {
                        qDebug() << "Wrong input: Only allowed values for rgbrange are \"automatic\", \"full\" and \"limited\"";
                        qApp->exit(9);
                        return;
                    }
                    if (!setRgbRange(output_id, range)) {
                        qDebug() << "Could not set rgb range " << range << " to output " << output_id;
                        qApp->exit(9);
                        return;
                    }
                } else {
                    cerr << "Unable to parse arguments: " << op << Qt::endl;
                    qApp->exit(2);
                    return;
                }
            }
        }
    }
}

void Console::configReceived(KScreen::ConfigOperation *op)
{
    m_config = op->config();

    if (m_parser->isSet(QStringLiteral("json"))) {
        showJson();
        qApp->quit();
    }
    if (m_parser->isSet(QStringLiteral("outputs"))) {
        showOutputs();
        qApp->quit();
    }

    parseOutputArgs();

    if (m_changed) {
        applyConfig();
        m_changed = false;
    }
}

int Console::outputCount() const
{
    if (!m_config) {
        qWarning() << "Invalid config.";
        return 0;
    }
    return m_config->outputs().count();
}

void Console::showOutputs() const
{
    if (!m_config) {
        qWarning() << "Invalid config.";
        return;
    }

    QHash<KScreen::Output::Type, QString> typeString;
    typeString[KScreen::Output::Unknown] = QStringLiteral("Unknown");
    typeString[KScreen::Output::VGA] = QStringLiteral("VGA");
    typeString[KScreen::Output::DVI] = QStringLiteral("DVI");
    typeString[KScreen::Output::DVII] = QStringLiteral("DVII");
    typeString[KScreen::Output::DVIA] = QStringLiteral("DVIA");
    typeString[KScreen::Output::DVID] = QStringLiteral("DVID");
    typeString[KScreen::Output::HDMI] = QStringLiteral("HDMI");
    typeString[KScreen::Output::Panel] = QStringLiteral("Panel");
    typeString[KScreen::Output::TV] = QStringLiteral("TV");
    typeString[KScreen::Output::TVComposite] = QStringLiteral("TVComposite");
    typeString[KScreen::Output::TVSVideo] = QStringLiteral("TVSVideo");
    typeString[KScreen::Output::TVComponent] = QStringLiteral("TVComponent");
    typeString[KScreen::Output::TVSCART] = QStringLiteral("TVSCART");
    typeString[KScreen::Output::TVC4] = QStringLiteral("TVC4");
    typeString[KScreen::Output::DisplayPort] = QStringLiteral("DisplayPort");

    QCollator collator;
    collator.setNumericMode(true);

    for (const auto &output : m_config->outputs()) {
        cout << green << "Output: " << cr << output->id() << " " << output->name();
        cout << " " << (output->isEnabled() ? green + QStringLiteral("enabled") : red + QStringLiteral("disabled"));
        cout << " " << (output->isConnected() ? green + QStringLiteral("connected") : red + QStringLiteral("disconnected"));
        cout << " " << (output->isPrimary() ? green + QStringLiteral("primary") : QString());
        auto _type = typeString[output->type()];
        cout << " " << yellow << (_type.isEmpty() ? QStringLiteral("UnmappedOutputType") : _type);
        cout << blue << " Modes: " << cr;

        const auto modes = output->modes();
        auto modeKeys = modes.keys();
        std::sort(modeKeys.begin(), modeKeys.end(), collator);

        for (const auto &key : modeKeys) {
            auto mode = *modes.find(key);

            auto name = QStringLiteral("%1x%2@%3")
                            .arg(QString::number(mode->size().width()), QString::number(mode->size().height()), QString::number(qRound(mode->refreshRate())));
            if (mode == output->currentMode()) {
                name = green + name + QLatin1Char('*') + cr;
            }
            if (mode == output->preferredMode()) {
                name = name + QLatin1Char('!');
            }
            cout << mode->id() << ":" << name << " ";
        }
        const auto g = output->geometry();
        cout << yellow << "Geometry: " << cr << g.x() << "," << g.y() << " " << g.width() << "x" << g.height() << " ";
        cout << yellow << "Scale: " << cr << output->scale() << " ";
        cout << yellow << "Rotation: " << cr << output->rotation() << " ";
        cout << yellow << "Overscan: " << cr << output->overscan() << " ";
        cout << yellow << "Vrr: ";
        if (output->capabilities() & Output::Capability::Vrr) {
            switch (output->vrrPolicy()) {
            case Output::VrrPolicy::Never:
                cout << cr << "Never ";
                break;
            case Output::VrrPolicy::Automatic:
                cout << cr << "Automatic ";
                break;
            case Output::VrrPolicy::Always:
                cout << cr << "Always ";
            }
        } else {
            cout << cr << "incapable ";
        }
        cout << yellow << "RgbRange: ";
        if (output->capabilities() & Output::Capability::RgbRange) {
            switch (output->rgbRange()) {
            case Output::RgbRange::Automatic:
                cout << cr << "Automatic ";
                break;
            case Output::RgbRange::Full:
                cout << cr << "Full ";
                break;
            case Output::RgbRange::Limited:
                cout << cr << "Limited ";
            }
        } else {
            cout << cr << "unknown ";
        }
        if (output->isPrimary()) {
            cout << blue << "primary";
        }
        cout << cr << Qt::endl;
    }
}

void Console::showJson() const
{
    QJsonDocument doc(KScreen::ConfigSerializer::serializeConfig(m_config));
    cout << doc.toJson(QJsonDocument::Indented);
}

bool Console::setEnabled(int id, bool enabled = true)
{
    if (!m_config) {
        qWarning() << "Invalid config.";
        return false;
    }

    for (const auto &output : m_config->outputs()) {
        if (output->id() == id) {
            cout << (enabled ? "Enabling " : "Disabling ") << "output " << id << Qt::endl;
            output->setEnabled(enabled);
            m_changed = true;
            return true;
        }
    }
    cerr << "Output with id " << id << " not found." << Qt::endl;
    qApp->exit(8);
    return false;
}

bool Console::setPosition(int id, const QPoint &pos)
{
    if (!m_config) {
        qWarning() << "Invalid config.";
        return false;
    }

    for (const auto &output : m_config->outputs()) {
        if (output->id() == id) {
            qDebug() << "Set output position" << pos;
            output->setPos(pos);
            m_changed = true;
            return true;
        }
    }
    cout << "Output with id " << id << " not found." << Qt::endl;
    return false;
}

bool Console::setMode(int id, const QString &mode_id)
{
    if (!m_config) {
        qWarning() << "Invalid config.";
        return false;
    }

    for (const auto &output : m_config->outputs()) {
        if (output->id() == id) {
            // find mode
            for (const KScreen::ModePtr &mode : output->modes()) {
                auto name =
                    QStringLiteral("%1x%2@%3")
                        .arg(QString::number(mode->size().width()), QString::number(mode->size().height()), QString::number(qRound(mode->refreshRate())));
                if (mode->id() == mode_id || name == mode_id) {
                    qDebug() << "Taddaaa! Found mode" << mode->id() << name;
                    output->setCurrentModeId(mode->id());
                    m_changed = true;
                    return true;
                }
            }
        }
    }
    cout << "Output mode " << mode_id << " not found." << Qt::endl;
    return false;
}

bool Console::setScale(int id, qreal scale)
{
    if (!m_config) {
        qWarning() << "Invalid config.";
        return false;
    }

    for (const auto &output : m_config->outputs()) {
        if (output->id() == id) {
            output->setScale(scale);
            m_changed = true;
            return true;
        }
    }
    cout << "Output scale " << id << " invalid." << Qt::endl;
    return false;
}

bool Console::setRotation(int id, KScreen::Output::Rotation rot)
{
    if (!m_config) {
        qWarning() << "Invalid config.";
        return false;
    }

    for (const auto &output : m_config->outputs()) {
        if (output->id() == id) {
            output->setRotation(rot);
            m_changed = true;
            return true;
        }
    }
    cout << "Output rotation " << id << " invalid." << Qt::endl;
    return false;
}

bool Console::setOverscan(int id, uint32_t overscan)
{
    if (!m_config) {
        qWarning() << "Invalid config.";
        return false;
    }

    for (const auto &output : m_config->outputs()) {
        if (output->id() == id) {
            output->setOverscan(overscan);
            m_changed = true;
            return true;
        }
    }
    cout << "Output overscan " << id << " invalid." << Qt::endl;
    return false;
}

bool Console::setVrrPolicy(int id, KScreen::Output::VrrPolicy policy)
{
    if (!m_config) {
        qWarning() << "Invalid config.";
        return false;
    }

    for (const auto &output : m_config->outputs()) {
        if (output->id() == id) {
            output->setVrrPolicy(policy);
            m_changed = true;
            return true;
        }
    }
    cout << "Output VrrPolicy " << id << " invalid." << Qt::endl;
    return false;
}

bool Console::setRgbRange(int id, KScreen::Output::RgbRange rgbRange)
{
    if (!m_config) {
        qWarning() << "Invalid config.";
        return false;
    }

    for (const auto &output : m_config->outputs()) {
        if (output->id() == id) {
            output->setRgbRange(rgbRange);
            m_changed = true;
            return true;
        }
    }
    cout << "Output RgbRange " << id << " invalid." << Qt::endl;
    return false;
}

bool KScreen::Console::setPrimary(int id)
{
    if (!m_config) {
        qWarning() << "Invalid config.";
        return false;
    }

    bool found = false;
    for (const auto &output : m_config->outputs()) {
        if (output->id() == id) {
            output->setPrimary(true);
            m_changed = true;
            found = true;
        }
    }
    if (found) {
        for (const auto &output : m_config->outputs()) {
            if (output->id() != id) {
                output->setPrimary(false);
                m_changed = true;
            }
        }
    }
    return found;
}

void Console::applyConfig()
{
    if (!m_changed) {
        return;
    }
    auto setop = new SetConfigOperation(m_config, this);
    setop->exec();
    qDebug() << "setop exec returned" << m_config;
    qApp->exit(0);
}
