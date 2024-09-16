// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONSOLE_H
#define CONSOLE_H

#include <QCommandLineParser>
#include <QObject>

#include <kscreen/config.h>
#include <kscreen/output.h>

namespace KScreen
{
class ConfigOperation;
class DpmsClient;

class Console : public QObject
{
    Q_OBJECT

public:
    explicit Console(QObject *parent = nullptr);
    ~Console() override;

    void setOptionList(const QStringList &positionalArgs);
    void start(QCommandLineParser *m_parser);
    void configReceived(KScreen::ConfigOperation *op);

    void showDpms();

    void showOutputs() const;
    void showJson() const;
    int outputCount() const;
    void setDpms(const QString &dpmsArg);

    bool setEnabled(int id, bool enabled);
    bool setPosition(int id, const QPoint &pos);
    bool setMode(int id, const QString &mode_id);
    bool setScale(int id, qreal scale);
    bool setRotation(int id, KScreen::Output::Rotation rot);
    bool setOverscan(int id, uint32_t overscan);
    bool setVrrPolicy(int id, KScreen::Output::VrrPolicy policy);
    bool setRgbRange(int id, KScreen::Output::RgbRange rgbRange);
    bool setPrimary(int id);

Q_SIGNALS:
    void outputsChanged();
    void started();
    void configChanged();

private:
    // static QString modeString(KWayland::Server::OutputDeviceInterface* outputdevice, int mid);
    void applyConfig();
    void parseOutputArgs();
    KScreen::ConfigPtr m_config;
    QCommandLineParser *m_parser;
    bool m_changed;
    QStringList m_outputArgs;
    DpmsClient *m_dpmsClient;
};

} // namespace

#endif // CONSOLE_H
