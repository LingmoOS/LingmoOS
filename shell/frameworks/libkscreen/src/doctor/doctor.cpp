/*
 *  SPDX-FileCopyrightText: 2014-2016 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "doctor.h"
#include "mode.h"
#include <dpms.h>

#include <QCollator>
#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QRect>
#include <QScreen>
#include <QStandardPaths>

#include <utility>

#include "../backendmanager_p.h"
#include "../config.h"
#include "../configoperation.h"
#include "../getconfigoperation.h"
#include "../log.h"
#include "../output.h"
#include "../setconfigoperation.h"

Q_LOGGING_CATEGORY(KSCREEN_DOCTOR, "kscreen.doctor")

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

Doctor::Doctor(QObject *parent)
    : QObject(parent)
    , m_config(nullptr)
    , m_changed(false)
    , m_dpmsClient(nullptr)
{
}

Doctor::~Doctor()
{
}

void Doctor::start(QCommandLineParser *parser)
{
    m_parser = parser;
    if (m_parser->isSet(QStringLiteral("info"))) {
        showBackends();
    }
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

        m_dpmsClient = new Dpms(this);
        auto screens = qGuiApp->screens();
        if (m_parser->isSet(QStringLiteral("dpms-excluded"))) {
            const auto excludedConnectors = m_parser->values(QStringLiteral("dpms-excluded"));
            auto it = std::remove_if(screens.begin(), screens.end(), [&excludedConnectors](QScreen *screen) {
                return excludedConnectors.contains(screen->name());
            });
            screens.erase(it, screens.end());
        }

        connect(m_dpmsClient, &Dpms::hasPendingChangesChanged, qGuiApp, [](bool hasChanges) {
            if (!hasChanges) {
                // We need to hit the event loop, otherwise .quit() hangs
                QTimer::singleShot(0, qApp->quit);
            }
        });

        const QString dpmsArg = m_parser->value(QStringLiteral("dpms"));
        if (dpmsArg == QLatin1String("show")) {
        } else {
            if (!m_dpmsClient->isSupported()) {
                cerr << "DPMS not supported in this system";
                qGuiApp->quit();
                return;
            }

            if (dpmsArg == QLatin1String("off")) {
                m_dpmsClient->switchMode(KScreen::Dpms::Off, screens);
            } else if (dpmsArg == QLatin1String("on")) {
                m_dpmsClient->switchMode(KScreen::Dpms::On, screens);
            } else {
                cerr << "--dpms argument not understood (" << dpmsArg << ")";
            }
        }
        return;
    }

    if (m_parser->isSet(QStringLiteral("log"))) {
        const QString logmsg = m_parser->value(QStringLiteral("log"));
        if (!Log::instance()->enabled()) {
            qCWarning(KSCREEN_DOCTOR) << "Logging is disabled, unset KSCREEN_LOGGING in your environment.";
        } else {
            Log::log(logmsg);
        }
    }
    // We need to kick the event loop, otherwise .quit() hangs
    QTimer::singleShot(0, qApp->quit);
}

void Doctor::showBackends() const
{
    cout << "Environment: " << Qt::endl;
    auto env_kscreen_backend = qEnvironmentVariable("KSCREEN_BACKEND", QStringLiteral("[not set]"));
    cout << "  * KSCREEN_BACKEND           : " << env_kscreen_backend << Qt::endl;
    auto env_kscreen_backend_inprocess = qEnvironmentVariable("KSCREEN_BACKEND_INPROCESS", QStringLiteral("[not set]"));
    cout << "  * KSCREEN_BACKEND_INPROCESS : " << env_kscreen_backend_inprocess << Qt::endl;
    auto env_kscreen_logging = qEnvironmentVariable("KSCREEN_LOGGING", QStringLiteral("[not set]"));
    cout << "  * KSCREEN_LOGGING           : " << env_kscreen_logging << Qt::endl;

    cout << "Logging to                : " << (Log::instance()->enabled() ? Log::instance()->logFile() : QStringLiteral("[logging disabled]")) << Qt::endl;
    const auto backends = BackendManager::instance()->listBackends();
    auto preferred = BackendManager::instance()->preferredBackend();
    cout << "Preferred KScreen backend : " << green << preferred.fileName() << cr << Qt::endl;
    cout << "Available KScreen backends:" << Qt::endl;
    for (const QFileInfo &f : backends) {
        auto c = blue;
        if (preferred == f) {
            c = green;
        }
        cout << "  * " << c << f.fileName() << cr << ": " << f.absoluteFilePath() << Qt::endl;
    }
    cout << Qt::endl;
}

void Doctor::setOptionList(const QStringList &outputArgs)
{
    m_outputArgs = outputArgs;
}

OutputPtr Doctor::findOutput(const QString &query)
{
    // try as an output name or ID
    for (const auto &output : m_config->outputs()) {
        if (output->name() == query) {
            return output;
        }
    }
    bool ok;
    int id = query.toInt(&ok);
    if (!ok) {
        cerr << "Output with name " << query << " not found." << Qt::endl;
        return OutputPtr();
    }

    if (m_config->outputs().contains(id)) {
        return m_config->outputs()[id];
    } else {
        cerr << "Output with id " << id << " not found." << Qt::endl;
        return OutputPtr();
    }
}

void Doctor::parseOutputArgs()
{
    // qCDebug(KSCREEN_DOCTOR) << "POSARGS" << m_positionalArgs;
    for (const QString &op : std::as_const(m_outputArgs)) {
        auto ops = op.split(QLatin1Char('.'));
        if (ops.count() > 2) {
            bool ok;
            if (ops[0] == QLatin1String("output")) {
                OutputPtr output = findOutput(ops[1]);
                if (!output) {
                    qApp->exit(3);
                    return;
                }
                int output_id = output->id();

                const QString subcmd = ops.length() > 2 ? ops[2] : QString();

                if (ops.count() == 3 && subcmd == QLatin1String("primary")) {
                    setPrimary(output);
                } else if (ops.count() == 4 && subcmd == QLatin1String("priority")) {
                    uint32_t priority = ops[3].toUInt(&ok);
                    if (!ok || priority > 100) {
                        qCWarning(KSCREEN_DOCTOR) << "Wrong input: allowed values for priority are from 1 to 100";
                        qApp->exit(5);
                        return;
                    }
                    setPriority(output, priority);
                } else if (ops.count() == 3 && subcmd == QLatin1String("enable")) {
                    setEnabled(output, true);
                } else if (ops.count() == 3 && subcmd == QLatin1String("disable")) {
                    setEnabled(output, false);
                } else if (ops.count() == 4 && subcmd == QLatin1String("mode")) {
                    QString mode_id = ops[3];
                    // set mode
                    if (!setMode(output, mode_id)) {
                        qApp->exit(9);
                        return;
                    }
                    qCDebug(KSCREEN_DOCTOR) << "Output" << output_id << "set mode" << mode_id;

                } else if (ops.count() == 4 && subcmd == QLatin1String("position")) {
                    QStringList _pos = ops[3].split(QLatin1Char(','));
                    if (_pos.count() != 2) {
                        qCWarning(KSCREEN_DOCTOR) << "Invalid position:" << ops[3];
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
                    setPosition(output, QPoint(x, y));
                } else if ((ops.count() == 4 || ops.count() == 5) && subcmd == QLatin1String("scale")) {
                    // be lenient about . vs. comma as separator
                    qreal scale = ops[3].replace(QLatin1Char(','), QLatin1Char('.')).toDouble(&ok);
                    if (ops.count() == 5) {
                        const QString dbl = ops[3] + QStringLiteral(".") + ops[4];
                        scale = dbl.toDouble(&ok);
                    };
                    // set scale
                    if (!ok || qFuzzyCompare(scale, 0.0)) {
                        qCWarning(KSCREEN_DOCTOR) << "Could not set scale " << scale << " to output " << output_id;
                        qApp->exit(9);
                        return;
                    }
                    setScale(output, scale);
                } else if ((ops.count() == 4) && (subcmd == QLatin1String("orientation") || subcmd == QStringLiteral("rotation"))) {
                    const QString _rotation = ops[3].toLower();
                    bool ok = false;
                    const QHash<QString, KScreen::Output::Rotation> rotationMap({{QStringLiteral("none"), KScreen::Output::None},
                                                                                 {QStringLiteral("normal"), KScreen::Output::None},
                                                                                 {QStringLiteral("left"), KScreen::Output::Left},
                                                                                 {QStringLiteral("right"), KScreen::Output::Right},
                                                                                 {QStringLiteral("inverted"), KScreen::Output::Inverted},
                                                                                 {QStringLiteral("flipped"), KScreen::Output::Flipped},
                                                                                 {QStringLiteral("flipped90"), KScreen::Output::Flipped90},
                                                                                 {QStringLiteral("flipped180"), KScreen::Output::Flipped180},
                                                                                 {QStringLiteral("flipped270"), KScreen::Output::Flipped270}});
                    KScreen::Output::Rotation rot = KScreen::Output::None;
                    // set orientation
                    if (rotationMap.contains(_rotation)) {
                        ok = true;
                        rot = rotationMap[_rotation];
                    }
                    if (!ok) {
                        qCWarning(KSCREEN_DOCTOR) << "Could not set orientation " << _rotation << " to output " << output_id;
                        qApp->exit(9);
                        return;
                    }
                    setRotation(output, rot);
                } else if (ops.count() == 4 && subcmd == QLatin1String("overscan")) {
                    const uint32_t overscan = ops[3].toInt();
                    if (overscan > 100) {
                        qCWarning(KSCREEN_DOCTOR) << "Wrong input: allowed values for overscan are from 0 to 100";
                        qApp->exit(9);
                        return;
                    }
                    setOverscan(output, overscan);
                } else if (ops.count() == 4 && subcmd == QLatin1String("vrrpolicy")) {
                    const QString _policy = ops[3].toLower();
                    KScreen::Output::VrrPolicy policy;
                    if (_policy == QStringLiteral("never")) {
                        policy = KScreen::Output::VrrPolicy::Never;
                    } else if (_policy == QStringLiteral("always")) {
                        policy = KScreen::Output::VrrPolicy::Always;
                    } else if (_policy == QStringLiteral("automatic")) {
                        policy = KScreen::Output::VrrPolicy::Automatic;
                    } else {
                        qCWarning(KSCREEN_DOCTOR) << "Wrong input: Only allowed values are \"never\", \"always\" and \"automatic\"";
                        qApp->exit(9);
                        return;
                    }
                    setVrrPolicy(output, policy);
                } else if (ops.count() == 4 && subcmd == QLatin1String("rgbrange")) {
                    const QString _range = ops[3].toLower();
                    KScreen::Output::RgbRange range;
                    if (_range == QStringLiteral("automatic")) {
                        range = KScreen::Output::RgbRange::Automatic;
                    } else if (_range == QStringLiteral("full")) {
                        range = KScreen::Output::RgbRange::Full;
                    } else if (_range == QStringLiteral("limited")) {
                        range = KScreen::Output::RgbRange::Limited;
                    } else {
                        qCWarning(KSCREEN_DOCTOR) << "Wrong input: Only allowed values for rgbrange are \"automatic\", \"full\" and \"limited\"";
                        qApp->exit(9);
                        return;
                    }
                    setRgbRange(output, range);
                } else if (ops.count() == 4 && subcmd == "hdr") {
                    const QString _enable = ops[3].toLower();
                    if (_enable == "enable") {
                        setHdrEnabled(output, true);
                    } else if (_enable == "disable") {
                        setHdrEnabled(output, false);
                    } else {
                        qCWarning(KSCREEN_DOCTOR) << "Wrong input: Only allowed values for hdr are \"enable\" and \"disable\"";
                        qApp->exit(9);
                        return;
                    }
                } else if (ops.count() == 4 && subcmd == "sdr-brightness") {
                    const uint32_t brightness = ops[3].toInt();
                    if (brightness < 100 || brightness > 10000) {
                        qCWarning(KSCREEN_DOCTOR) << "Wrong input: Allowed range for sdr-brightness is 100 to 10000";
                        qApp->exit(9);
                        return;
                    }
                    setSdrBrightness(output, brightness);
                } else if (ops.count() == 4 && subcmd == "wcg") {
                    const QString _enable = ops[3].toLower();
                    if (_enable == "enable") {
                        setWcgEnabled(output, true);
                    } else if (_enable == "disable") {
                        setWcgEnabled(output, false);
                    } else {
                        qCWarning(KSCREEN_DOCTOR) << "Wrong input: Only allowed values for wcg are \"enable\" and \"disable\"";
                        qApp->exit(9);
                        return;
                    }
                } else if (ops.count() >= 4 && subcmd == "iccprofile") {
                    QString profilePath = ops[3];
                    for (uint32_t i = 4; i < ops.size(); i++) {
                        profilePath += "." + ops[i];
                    }
                    output->setIccProfilePath(profilePath);
                    if (profilePath.isEmpty()) {
                        output->setColorProfileSource(Output::ColorProfileSource::sRGB);
                    } else {
                        output->setColorProfileSource(Output::ColorProfileSource::ICC);
                    }
                    m_changed = true;
                } else if (ops.count() >= 4 && subcmd == "sdrGamut") {
                    const uint32_t wideness = ops[3].toUInt();
                    if (wideness > 100) {
                        qCWarning(KSCREEN_DOCTOR) << "Wrong input: Allowed range for sdr wideness is 0 to 100";
                        qApp->exit(9);
                        return;
                    }
                    output->setSdrGamutWideness(wideness / 100.0);
                    m_changed = true;
                } else if (ops.count() >= 4 && subcmd == "maxBrightnessOverride") {
                    if (ops[3] == "disable") {
                        output->setMaxPeakBrightnessOverride(std::nullopt);
                    } else if (const uint32_t nits = ops[3].toUInt(); nits != 0) {
                        output->setMaxPeakBrightnessOverride(nits);
                    } else {
                        qCWarning(KSCREEN_DOCTOR) << "Wrong input: max brightness must be bigger than 0";
                        qApp->exit(9);
                        return;
                    }
                    m_changed = true;
                } else if (ops.count() >= 4 && subcmd == "maxAverageBrightnessOverride") {
                    if (ops[3] == "disable") {
                        output->setMaxPeakBrightnessOverride(std::nullopt);
                    } else if (const uint32_t nits = ops[3].toUInt(); nits != 0) {
                        output->setMaxAverageBrightnessOverride(nits);
                    } else {
                        qCWarning(KSCREEN_DOCTOR) << "Wrong input: max average brightness must be bigger than 0";
                        qApp->exit(9);
                        return;
                    }
                    m_changed = true;
                } else if (ops.count() >= 4 && subcmd == "minBrightnessOverride") {
                    bool ok = false;
                    if (ops[3] == "disable") {
                        output->setMinBrightnessOverride(std::nullopt);
                    } else if (const uint32_t nits10k = ops[3].toUInt(&ok); ok) {
                        output->setMinBrightnessOverride(nits10k / 10'000.0);
                    } else {
                        qCWarning(KSCREEN_DOCTOR) << "Wrong input: couldn't parse" << ops[3];
                        qApp->exit(9);
                        return;
                    }
                    m_changed = true;
                } else if (ops.count() >= 4 && subcmd == "colorProfileSource") {
                    if (ops[3] == "sRGB") {
                        output->setColorProfileSource(Output::ColorProfileSource::sRGB);
                    } else if (ops[3] == "ICC") {
                        output->setColorProfileSource(Output::ColorProfileSource::ICC);
                    } else if (ops[3] == "EDID") {
                        output->setColorProfileSource(Output::ColorProfileSource::EDID);
                    } else {
                        qCWarning(KSCREEN_DOCTOR) << "Wrong input: only allowed values for colorProfileSource are \"sRGB\", \"ICC\" and \"EDID\"";
                        qApp->exit(9);
                        return;
                    }
                    m_changed = true;
                } else if (ops.count() >= 4 && subcmd == "brightness") {
                    const uint32_t brightness = ops[3].toUInt();
                    if (brightness > 100) {
                        qCWarning(KSCREEN_DOCTOR) << "Wrong input: Allowed range for brightness is 0 to 100";
                        qApp->exit(9);
                        return;
                    }
                    output->setBrightness(brightness / 100.0);
                    m_changed = true;
                } else {
                    cerr << "Unable to parse arguments: " << op << Qt::endl;
                    qApp->exit(2);
                    return;
                }
            }
        }
    }
}

void Doctor::configReceived(KScreen::ConfigOperation *op)
{
    m_config = op->config();

    if (!m_config) {
        qCWarning(KSCREEN_DOCTOR) << "Invalid config.";
        return;
    }

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

void Doctor::showOutputs() const
{
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
        const auto endl = '\n';
        cout << green << "Output: " << cr << output->id() << " " << output->name() << endl;
        cout << "\t" << (output->isEnabled() ? green + QStringLiteral("enabled") : red + QStringLiteral("disabled")) << cr << endl;
        cout << "\t" << (output->isConnected() ? green + QStringLiteral("connected") : red + QStringLiteral("disconnected")) << cr << endl;
        cout << "\t" << (output->isEnabled() ? green : red) + QStringLiteral("priority ") << output->priority() << cr << endl;
        auto _type = typeString[output->type()];
        cout << "\t" << yellow << (_type.isEmpty() ? QStringLiteral("UnmappedOutputType") : _type) << cr << endl;
        cout << "\t" << blue << "Modes: " << cr;

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
            cout << " " << mode->id() << ":" << name << " ";
        }
        cout << endl;
        const auto g = output->geometry();
        cout << yellow << "\tGeometry: " << cr << g.x() << "," << g.y() << " " << g.width() << "x" << g.height() << endl;
        cout << yellow << "\tScale: " << cr << output->scale() << endl;
        cout << yellow << "\tRotation: " << cr << output->rotation() << endl;
        cout << yellow << "\tOverscan: " << cr << output->overscan() << endl;
        cout << yellow << "\tVrr: ";
        if (output->capabilities() & Output::Capability::Vrr) {
            switch (output->vrrPolicy()) {
            case Output::VrrPolicy::Never:
                cout << cr << "Never" << endl;
                break;
            case Output::VrrPolicy::Automatic:
                cout << cr << "Automatic" << endl;
                break;
            case Output::VrrPolicy::Always:
                cout << cr << "Always" << endl;
            }
        } else {
            cout << cr << "incapable" << endl;
        }
        cout << yellow << "\tRgbRange: ";
        if (output->capabilities() & Output::Capability::RgbRange) {
            switch (output->rgbRange()) {
            case Output::RgbRange::Automatic:
                cout << cr << "Automatic" << endl;
                break;
            case Output::RgbRange::Full:
                cout << cr << "Full" << endl;
                break;
            case Output::RgbRange::Limited:
                cout << cr << "Limited" << endl;
            }
        } else {
            cout << cr << "unknown" << endl;
        }
        cout << yellow << "\tHDR: ";
        if (output->capabilities() & Output::Capability::HighDynamicRange) {
            if (output->isHdrEnabled()) {
                cout << cr << "enabled" << endl;
                cout << yellow << "\t\tSDR brightness: " << cr << output->sdrBrightness() << " nits" << endl;
                cout << yellow << "\t\tSDR gamut wideness: " << cr << std::round(output->sdrGamutWideness() * 100) << "%" << endl;
                cout << yellow << "\t\tPeak brightness: " << cr << output->maxPeakBrightness() << " nits";
                if (const auto used = output->maxPeakBrightnessOverride()) {
                    cout << yellow << ", overridden with: " << cr << *used << " nits";
                }
                cout << endl;
                cout << yellow << "\t\tMax average brightness: " << cr << output->maxAverageBrightness() << " nits";
                if (const auto used = output->maxAverageBrightnessOverride()) {
                    cout << yellow << ", overridden with: " << cr << *used << " nits";
                }
                cout << endl;
                cout << yellow << "\t\tMin brightness: " << cr << output->minBrightness() << " nits";
                if (const auto used = output->minBrightnessOverride()) {
                    cout << yellow << ", overridden with: " << cr << (*used) / 10'000.0 << " nits";
                }
                cout << endl;
            } else {
                cout << cr << "disabled" << endl;
            }
        } else {
            cout << cr << "incapable" << endl;
        }
        cout << yellow << "\tWide Color Gamut: ";
        if (output->capabilities() & Output::Capability::WideColorGamut) {
            if (output->isWcgEnabled()) {
                cout << cr << "enabled" << endl;
            } else {
                cout << cr << "disabled" << endl;
            }
        } else {
            cout << cr << "incapable" << endl;
        }
        cout << yellow << "\tICC profile: ";
        if (output->capabilities() & Output::Capability::IccProfile) {
            if (!output->iccProfilePath().isEmpty()) {
                cout << cr << output->iccProfilePath() << endl;
            } else {
                cout << cr << "none" << endl;
            }
        } else {
            cout << cr << "incapable" << endl;
        }
        cout << yellow << "\tColor profile source: ";
        if (output->capabilities() & Output::Capability::IccProfile) {
            cout << cr;
            switch (output->colorProfileSource()) {
            case Output::ColorProfileSource::sRGB:
                cout << "sRGB";
                break;
            case Output::ColorProfileSource::ICC:
                cout << "ICC";
                break;
            case Output::ColorProfileSource::EDID:
                cout << "EDID";
                break;
            }
            cout << endl;
        } else {
            cout << cr << "incapable" << endl;
        }
        cout << yellow << "\tBrightness control: ";
        if (output->capabilities() & Output::Capability::BrightnessControl) {
            cout << cr << "supported, set to " << std::round(output->brightness() * 100) << "%" << endl;
        } else {
            cout << cr << "unsupported" << endl;
        }
    }
}

void Doctor::showJson() const
{
    QJsonDocument doc(KScreen::ConfigSerializer::serializeConfig(m_config));
    cout << doc.toJson(QJsonDocument::Indented);
}

void Doctor::setEnabled(OutputPtr output, bool enable)
{
    cout << (enable ? "Enabling " : "Disabling ") << "output " << output->id() << Qt::endl;
    output->setEnabled(enable);
    m_changed = true;
}

void Doctor::setPosition(OutputPtr output, const QPoint &pos)
{
    qCDebug(KSCREEN_DOCTOR) << "Set output position" << pos;
    output->setPos(pos);
    m_changed = true;
}

KScreen::ModePtr Doctor::findMode(OutputPtr output, const QString &query)
{
    for (const KScreen::ModePtr &mode : output->modes()) {
        auto name = QStringLiteral("%1x%2@%3")
                        .arg(QString::number(mode->size().width()), QString::number(mode->size().height()), QString::number(qRound(mode->refreshRate())));
        if (mode->id() == query || name == query) {
            qCDebug(KSCREEN_DOCTOR) << "Taddaaa! Found mode" << mode->id() << name;
            return mode;
        }
    }
    cout << "Output mode " << query << " not found." << Qt::endl;
    return ModePtr();
}

bool Doctor::setMode(OutputPtr output, const QString &query)
{
    // find mode
    const KScreen::ModePtr mode = findMode(output, query);
    if (!mode) {
        return false;
    }
    output->setCurrentModeId(mode->id());
    m_changed = true;
    return true;
}

void Doctor::setScale(OutputPtr output, qreal scale)
{
    output->setScale(scale);
    m_changed = true;
}

void Doctor::setRotation(OutputPtr output, KScreen::Output::Rotation rot)
{
    output->setRotation(rot);
    m_changed = true;
}

void Doctor::setOverscan(OutputPtr output, uint32_t overscan)
{
    output->setOverscan(overscan);
    m_changed = true;
}

void Doctor::setVrrPolicy(OutputPtr output, KScreen::Output::VrrPolicy policy)
{
    output->setVrrPolicy(policy);
    m_changed = true;
}

void Doctor::setRgbRange(OutputPtr output, KScreen::Output::RgbRange rgbRange)
{
    output->setRgbRange(rgbRange);
    m_changed = true;
}

void KScreen::Doctor::setPrimary(OutputPtr output)
{
    setPriority(output, 1);
}

void KScreen::Doctor::setPriority(OutputPtr output, uint32_t priority)
{
    m_config->setOutputPriority(output, priority);
    m_changed = true;
}

void Doctor::setHdrEnabled(OutputPtr output, bool enable)
{
    output->setHdrEnabled(enable);
    m_changed = true;
}

void Doctor::setSdrBrightness(OutputPtr output, uint32_t brightness)
{
    output->setSdrBrightness(brightness);
    m_changed = true;
}

void Doctor::setWcgEnabled(OutputPtr output, bool enable)
{
    output->setWcgEnabled(enable);
    m_changed = true;
}

void Doctor::applyConfig()
{
    if (!m_changed) {
        return;
    }
    auto setop = new SetConfigOperation(m_config, this);
    setop->exec();
    qCDebug(KSCREEN_DOCTOR) << "setop exec returned" << m_config;
    qApp->exit(0);
}

#include "moc_doctor.cpp"
