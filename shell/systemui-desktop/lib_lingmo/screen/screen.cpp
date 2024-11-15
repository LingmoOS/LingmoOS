#include "screen.h"
#include "outputmodel.h"

#include <kscreen/setconfigoperation.h>

#include <QQmlExtensionPlugin>
#include <QQmlEngine>

Screen::Screen(QObject *parent)
    : QObject(parent)
{
    qmlRegisterType<OutputModel>();
    load();
}

void Screen::load()
{
    // Don't pull away the outputModel under QML's feet
    // signal its disappearance first before deleting and replacing it.
    // We take the m_config pointer so outputModel() will return null,
    // gracefully cleaning up the QML side and only then we will delete it.
    auto *oldConfig = m_config.release();
    if (oldConfig) {
        emit outputModelChanged();
        delete oldConfig;
    }

    m_config.reset(new ConfigHandler(this));
    connect(m_config.get(), &ConfigHandler::outputModelChanged, this, &Screen::outputModelChanged);

    connect(new KScreen::GetConfigOperation(), &KScreen::GetConfigOperation::finished, this, &Screen::configReady);
}

void Screen::save()
{
    if (!m_config)
        return;

    auto config = m_config->config();
    bool atLeastOneEnabledOutput = false;

    for (const KScreen::OutputPtr &output : config->outputs()) {
        KScreen::ModePtr mode = output->currentMode();
        atLeastOneEnabledOutput |= output->isEnabled();
    }

    m_config->writeControl();

    auto *op = new KScreen::SetConfigOperation(config);
    op->exec();
}

OutputModel *Screen::outputModel() const
{
    if (!m_config) {
        return nullptr;
    }

    return m_config->outputModel();
}

void Screen::configReady(KScreen::ConfigOperation *op)
{
    if (op->hasError()) {
        m_config.reset();
        return;
    }

    KScreen::ConfigPtr config = qobject_cast<KScreen::GetConfigOperation *>(op)->config();
    // const bool autoRotationSupported = config->supportedFeatures() & (KScreen::Config::Feature::AutoRotation | KScreen::Config::Feature::TabletMode);

    m_config->setConfig(config);
}
