#pragma once

#include <QObject>
#include <memory>
#include <kscreen/getconfigoperation.h>

#include "confighandler.h"
#include "outputmodel.h"

class ConfigHandler;
class OutputModel;
class Screen : public QObject
{
    Q_OBJECT
    Q_PROPERTY(OutputModel *outputModel READ outputModel NOTIFY outputModelChanged)

public:
    explicit Screen(QObject *parent = nullptr);

    OutputModel *outputModel() const;

    void load();
    Q_INVOKABLE void save();

private:
    void configReady(KScreen::ConfigOperation *op);

Q_SIGNALS:
    void outputModelChanged();

private:
    std::unique_ptr<ConfigHandler> m_config;
};