// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/secrets/network_secret_agent.h"

#include <chrono>

#include "base/logging.h"
#include "lingmo/networkmanager/client.h"
#include "lingmo/networkmanager/handler.h"
#include "lingmo/networkmanager/variants.h"
#include "shell/secrets/secret_store.h"

namespace Lingmo::Shell {

using base::dbus::ObjectPath;
using base::dbus::Variant;
using base::dbus::VariantMap;
using networkmanager::Settings;
namespace variants = networkmanager::variants;

static const base::LogCategory lingmoSecrets("lingmo.shell.secrets");

static const std::string kAgentPath = "/org/freedesktop/NetworkManager/SecretAgent";
static const std::string kAgentInterface = "org.freedesktop.NetworkManager.SecretAgent";
static const std::string kAgentManagerPath = "/org/freedesktop/NetworkManager/AgentManager";
static const std::string kAgentManagerInterface = "org.freedesktop.NetworkManager.AgentManager";
static const std::string kAgentIdentifier = "org.lingmo.network";

static const std::string kPromptService = "org.lingmo.SecretAgent";
static const std::string kPromptPath = "/SecretAgent";
static const std::string kPromptInterface = "org.lingmo.SecretAgent";

static const std::string kConnectionSetting = "connection";
static const std::string kWirelessSecurity = "802-11-wireless-security";
static const std::string kIeee8021x = "802-1x";

static const std::string kNoSecrets = "org.freedesktop.NetworkManager.SecretAgent.NoSecrets";
static const std::string kUserCanceled = "org.freedesktop.NetworkManager.SecretAgent.UserCanceled";
static const std::string kAgentCanceled = "org.freedesktop.NetworkManager.SecretAgent.AgentCanceled";
static const std::string kPermissionDenied = "org.freedesktop.NetworkManager.SecretAgent.PermissionDenied";

// NMSecretAgentGetSecretsFlags.
static constexpr uint32_t kAllowInteraction = 0x1;
static constexpr uint32_t kRequestNew = 0x2;
static constexpr uint32_t kNoErrors = 0x40000000;
// NMSecretAgentCapabilities: VPN hints.
static constexpr uint32_t kVpnHints = 0x1;

// NetworkManager keeps one flags property for all four WEP keys.
static std::string flagsKey(const std::string &key)
{
    if (key.starts_with("wep-key") && key.size() == 8)
        return "wep-key-flags";
    return key + "-flags";
}

static bool savedByAgent(uint32_t flags)
{
    return (flags & networkmanager::SecretAgentOwned) && !(flags & networkmanager::SecretNotSaved);
}

static uint32_t stringFlags(const std::map<std::string, std::string> &data, const std::string &key)
{
    const auto it = data.find(key + "-flags");
    return it == data.end() ? 0 : uint32_t(std::strtoul(it->second.c_str(), nullptr, 10));
}

static const VariantMap &settingOf(const Settings &connection, const std::string &name)
{
    static const VariantMap none;
    const auto it = connection.find(name);
    return it == connection.end() ? none : it->second;
}

NetworkSecretAgent::NetworkSecretAgent(networkmanager::Client *client, base::dbus::Connection *session)
    : m_client(client)
    , m_session(session)
    , m_store(std::make_unique<SecretStore>(session))
{
    m_registerRetry.setSingleShot(true);
    if (!m_client)
        return;
    exportAgent();

    // Registered with every NetworkManager that starts; one that quit took its requests along.
    m_readySubscription = m_client->readyChanged.connect([this] { readyChanged(); });
    m_changedSubscription = m_client->changed.connect([this] { dismissStalePrompts(); });
    if (m_client->ready())
        registerAgent();
}

NetworkSecretAgent::~NetworkSecretAgent()
{
    if (!m_client)
        return;
    for (const Request &request : m_lookups)
        fail(request, kAgentCanceled, "The agent is going away");
    for (const Request &request : m_requests) {
        if (request.accessPointPath.empty())
            fail(request, kAgentCanceled, "The agent is going away");
    }
    if (m_registered && !m_client->owner().empty()) {
        base::dbus::Message message = m_client->bus()->newMethodCall(m_client->owner(), kAgentManagerPath,
                                                                     kAgentManagerInterface, "Unregister");
        m_client->bus()->send(message);
    }
}

void NetworkSecretAgent::exportAgent()
{
    base::dbus::Connection *bus = m_client->bus();
    if (!bus)
        return;
    m_agentObject = std::make_unique<base::dbus::Object>(bus, kAgentPath, kAgentInterface);
    m_agentObject->addMethod("GetSecrets", "a{sa{sv}}osasu", "a{sa{sv}}",
                             [this](base::dbus::MethodCall &call) { getSecrets(call); },
                             {{"connection", "connectionPath", "settingName", "hints", "flags"}, {"result"}});
    m_agentObject->addMethod("CancelGetSecrets", "os", "",
                             [this](base::dbus::MethodCall &call) {
                                 if (!fromNetworkManager(call.message())) {
                                     call.replyError(kPermissionDenied, "Only NetworkManager may ask for secrets");
                                     return;
                                 }
                                 ObjectPath path;
                                 std::string setting;
                                 call.read(path, setting);
                                 cancelGetSecrets(path.path, setting);
                                 call.reply();
                             },
                             {{"connectionPath", "settingName"}, {}});
    m_agentObject->addMethod("SaveSecrets", "a{sa{sv}}o", "",
                             [this](base::dbus::MethodCall &call) { saveSecrets(call); },
                             {{"connection", "connectionPath"}, {}});
    m_agentObject->addMethod("DeleteSecrets", "a{sa{sv}}o", "",
                             [this](base::dbus::MethodCall &call) { deleteSecrets(call); },
                             {{"connection", "connectionPath"}, {}});
    if (!m_agentObject->publish()) {
        LM_WARNING(lingmoSecrets) << "Cannot export the NetworkManager secret agent";
        m_agentObject.reset();
    }
}

bool NetworkSecretAgent::registerService()
{
    if (!m_session)
        return false;
    m_promptObject = std::make_unique<base::dbus::Object>(m_session, kPromptPath, kPromptInterface);
    m_promptObject->addMethod("ConnectWireless",
                              [this](const std::string &devicePath, const std::string &accessPointPath,
                                     const std::string &name, bool wep) {
                                  connectWireless(devicePath, accessPointPath, name, wep);
                              },
                              {{"devicePath", "accessPointPath", "name", "wep"}, {}});
    if (!m_promptObject->publish()
        || !m_session->requestName(kPromptService, base::dbus::Connection::DoNotQueue)) {
        LM_WARNING(lingmoSecrets) << "Cannot offer" << kPromptService;
        return false;
    }
    return true;
}

void NetworkSecretAgent::readyChanged()
{
    m_registered = false;
    ++m_registration;
    m_registerRetry.stop();
    m_registerCall.reset();
    if (m_client->ready()) {
        registerAgent();
        return;
    }

    const bool prompting = !m_requests.empty();
    m_lookups.clear();
    m_requests.clear();
    if (prompting)
        requestChanged.emit();
}

void NetworkSecretAgent::registerAgent(int attempt)
{
    base::dbus::Connection *bus = m_client->bus();
    if (!bus || m_client->owner().empty())
        return;
    const uint64_t registration = m_registration;
    base::dbus::Message message = bus->newMethodCall(m_client->owner(), kAgentManagerPath, kAgentManagerInterface,
                                                     "RegisterWithCapabilities");
    message.append(kAgentIdentifier, kVpnHints);
    m_registerCall = bus->callAsync(message, [this, registration, attempt](const base::dbus::Reply &reply) {
        if (registration != m_registration)
            return;
        if (!reply.isError()) {
            m_registered = true;
            return;
        }
        // Right after it appears, NetworkManager may not export its agent manager yet.
        if (attempt < 6) {
            m_registerRetry.onTimeout = [this, registration, attempt] {
                if (registration == m_registration)
                    registerAgent(attempt + 1);
            };
            m_registerRetry.start(std::chrono::milliseconds(100 << attempt));
            return;
        }
        LM_WARNING(lingmoSecrets) << "Cannot register the NetworkManager secret agent:" << reply.error().message;
    });
}

bool NetworkSecretAgent::fromNetworkManager(const base::dbus::Message &message) const
{
    // The bus policy already lets only root talk to agents; this keeps it to NetworkManager.
    return m_client->owner().empty() || message.sender() == m_client->owner();
}

std::string NetworkSecretAgent::connectionName() const
{
    return m_requests.empty() ? std::string() : m_requests.front().connectionName;
}

NetworkSecretAgent::Kind NetworkSecretAgent::kind() const
{
    return m_requests.empty() ? Kind::WpaPassword : m_requests.front().kind;
}

std::string NetworkSecretAgent::identity() const
{
    return m_requests.empty() ? std::string() : m_requests.front().identity;
}

bool NetworkSecretAgent::retry() const
{
    return !m_requests.empty() && m_requests.front().retry;
}

std::vector<NetworkSecretAgent::Field> NetworkSecretAgent::fields() const
{
    std::vector<Field> fields;
    if (m_requests.empty())
        return fields;
    const Request &request = m_requests.front();
    for (const VpnSecretField &field : request.fields) {
        const auto value = request.values.find(field.key);
        fields.push_back({field.key, field.label, field.secret,
                          value == request.values.end() ? std::string() : value->second});
    }
    return fields;
}

void NetworkSecretAgent::retranslate()
{
    for (Request &request : m_requests) {
        if (request.kind == Kind::VpnSecrets)
            request.fields = vpnSecretFields(request.vpnServiceType, request.vpnData, request.hints);
    }
}

std::string NetworkSecretAgent::message() const
{
    return m_requests.empty() ? std::string() : m_requests.front().serverMessage;
}

void NetworkSecretAgent::getSecrets(base::dbus::MethodCall &call)
{
    if (!fromNetworkManager(call.message())) {
        call.replyError(kPermissionDenied, "Only NetworkManager may ask for secrets");
        return;
    }
    Settings connection;
    ObjectPath connectionPath;
    std::string settingName;
    std::vector<std::string> hints;
    uint32_t flags = 0;
    if (!call.read(connection, connectionPath, settingName, hints, flags)) {
        call.replyError("org.freedesktop.DBus.Error.InvalidArgs", "Unexpected arguments");
        return;
    }

    Request request;
    request.id = ++m_nextId;
    request.call = call;
    request.connectionPath = connectionPath.path;
    request.settingName = settingName;
    const VariantMap &connectionSetting = settingOf(connection, kConnectionSetting);
    request.connectionName = variants::toString(variants::value(connectionSetting, "id"));
    request.uuid = variants::toString(variants::value(connectionSetting, "uuid"));
    request.interactive = flags & (kAllowInteraction | kRequestNew);
    request.retry = flags & kRequestNew;
    request.noErrors = flags & kNoErrors;

    const VariantMap &setting = settingOf(connection, settingName);
    if (settingName == "vpn") {
        request.kind = Kind::VpnSecrets;
        request.vpnData = variants::toStringMap(variants::value(setting, "data"));
        request.vpnServiceType = variants::toString(variants::value(setting, "service-type"));
        request.hints = hints;
        request.fields = vpnSecretFields(request.vpnServiceType, request.vpnData, hints);
        request.serverMessage = vpnHintMessage(hints);
        request.values = variants::toStringMap(variants::value(setting, "secrets"));
    } else if (settingName == kWirelessSecurity) {
        const std::string keyMgmt = variants::toString(variants::value(setting, "key-mgmt"));
        if (keyMgmt == "wpa-psk" || keyMgmt == "sae" || keyMgmt == "wpa-none") {
            request.kind = Kind::WpaPassword;
            request.key = "psk";
        } else if (keyMgmt == "none") {
            request.kind = Kind::WepKey;
            request.key = "wep-key" + std::to_string(variants::toUInt(variants::value(setting, "wep-tx-keyidx")));
        } else if (keyMgmt == "ieee8021x" && variants::toString(variants::value(setting, "auth-alg")) == "leap") {
            request.kind = Kind::LeapPassword;
            request.key = "leap-password";
            request.identity = variants::toString(variants::value(setting, "leap-username"));
        }
    } else if (settingName == kIeee8021x) {
        const std::vector<std::string> eap = variants::toStringList(variants::value(setting, "eap"));
        request.identity = variants::toString(variants::value(setting, "identity"));
        if (eap == std::vector<std::string> {"tls"}) {
            request.kind = Kind::PrivateKeyPassword;
            request.key = "private-key-password";
        } else if (!request.identity.empty()) {
            // The identity is not a secret, so a reply cannot supply a missing one.
            request.kind = Kind::EapPassword;
            request.key = "password";
        }
    }
    request.secretFlags = variants::toUInt(variants::value(setting, flagsKey(request.key)));

    // A second request for the same secret replaces the first, and its prompt.
    const auto same = [&](const Request &other) {
        return other.connectionPath == request.connectionPath && other.settingName == settingName;
    };
    for (size_t i = m_lookups.size(); i-- > 0;) {
        if (!same(m_lookups[i]))
            continue;
        fail(m_lookups[i], kAgentCanceled, "A newer request replaced it");
        m_lookups.erase(m_lookups.begin() + long(i));
    }
    for (size_t i = m_requests.size(); i-- > 0;) {
        if (!same(m_requests[i]))
            continue;
        fail(m_requests[i], kAgentCanceled, "A newer request replaced it");
        if (request.interactive && (!request.key.empty() || !request.fields.empty())) {
            m_requests[i] = request;
            if (i == 0)
                requestChanged.emit();
            return;
        }
        m_requests.erase(m_requests.begin() + long(i));
        if (i == 0)
            requestChanged.emit();
    }

    if (request.kind == Kind::VpnSecrets) {
        getVpnSecrets(std::move(request));
        return;
    }

    // RequestNew means what is stored was just rejected, and a secret that is
    // never saved has nothing stored.
    if (request.retry || (request.secretFlags & networkmanager::SecretNotSaved) || request.uuid.empty()) {
        ask(request);
        return;
    }

    m_lookups.push_back(request);
    m_store->load(request.uuid, settingName, [this, id = request.id](const SecretStore::Secrets &secrets) {
        // Cancelled or replaced meanwhile: nothing to answer.
        const auto it = std::find_if(m_lookups.begin(), m_lookups.end(),
                                     [id](const Request &request) { return request.id == id; });
        if (it == m_lookups.end())
            return;
        const Request request = *it;
        m_lookups.erase(it);
        const bool complete = !secrets.empty() && (request.key.empty() || secrets.count(request.key));
        if (complete || (!secrets.empty() && !request.interactive)) {
            VariantMap values;
            for (const auto &[key, value] : secrets)
                values[key] = Variant(value);
            reply(request, {{request.settingName, values}});
        } else {
            ask(request);
        }
    });
}

void NetworkSecretAgent::getVpnSecrets(Request request)
{
    // Nothing to ask for, a plugin that signs in by itself for instance: what
    // NetworkManager has is the answer.
    if (request.fields.empty()) {
        replyVpn(request, request.values);
        return;
    }
    if (request.retry || request.uuid.empty()) {
        ask(request);
        return;
    }

    m_lookups.push_back(request);
    m_store->load(request.uuid, "vpn", [this, id = request.id](const SecretStore::Secrets &stored) {
        const auto it = std::find_if(m_lookups.begin(), m_lookups.end(),
                                     [id](const Request &request) { return request.id == id; });
        if (it == m_lookups.end())
            return;
        Request request = *it;
        m_lookups.erase(it);
        for (const auto &[key, value] : stored) {
            if (request.values[key].empty())
                request.values[key] = value;
        }

        // A one-time code and other secrets that are never saved are asked for each time.
        bool complete = true;
        for (const VpnSecretField &field : request.fields) {
            const uint32_t flags = stringFlags(request.vpnData, field.key);
            complete &= !(flags & networkmanager::SecretNotSaved) && !request.values[field.key].empty();
        }

        // Without interaction, an answer with no secrets has NetworkManager ask again with it.
        if (complete || !request.interactive)
            replyVpn(request, request.values);
        else
            ask(request);
    });
}

void NetworkSecretAgent::replyVpn(const Request &request, const std::map<std::string, std::string> &values)
{
    std::map<std::string, std::string> secrets;
    for (const auto &[key, value] : values) {
        if (!value.empty())
            secrets[key] = value;
    }
    reply(request, {{"vpn", {{"secrets", Variant(secrets)}}}});
}

void NetworkSecretAgent::cancelGetSecrets(const std::string &connectionPath, const std::string &settingName)
{
    // NetworkManager may cancel a request it already has the answer to.
    const auto matches = [&](const Request &request) {
        return request.connectionPath == connectionPath && request.settingName == settingName;
    };
    for (auto it = m_lookups.begin(); it != m_lookups.end(); ++it) {
        if (!matches(*it))
            continue;
        fail(*it, kAgentCanceled, "Canceled by NetworkManager");
        m_lookups.erase(it);
        return;
    }
    for (size_t i = 0; i < m_requests.size(); ++i) {
        if (!matches(m_requests[i]))
            continue;
        fail(m_requests[i], kAgentCanceled, "Canceled by NetworkManager");
        m_requests.erase(m_requests.begin() + long(i));
        if (i == 0)
            requestChanged.emit();
        return;
    }
}

void NetworkSecretAgent::saveSecrets(base::dbus::MethodCall &call)
{
    if (!fromNetworkManager(call.message())) {
        call.replyError(kPermissionDenied, "Only NetworkManager may ask for secrets");
        return;
    }
    Settings connection;
    ObjectPath path;
    call.read(connection, path);
    const VariantMap &connectionSetting = settingOf(connection, kConnectionSetting);
    const std::string uuid = variants::toString(variants::value(connectionSetting, "uuid"));
    const std::string name = variants::toString(variants::value(connectionSetting, "id"));
    if (uuid.empty()) {
        call.reply();
        return;
    }

    // System-owned secrets are NetworkManager's to store; only agent-owned ones come here.
    std::vector<SecretStore::Entry> entries;
    for (const auto &[settingName, setting] : connection) {
        if (settingName == "vpn") {
            const auto data = variants::toStringMap(variants::value(setting, "data"));
            const auto secrets = variants::toStringMap(variants::value(setting, "secrets"));
            for (const auto &[key, value] : secrets) {
                if (savedByAgent(stringFlags(data, key)))
                    entries.push_back({settingName, key, value});
            }
            continue;
        }
        for (const auto &[key, value] : setting) {
            const auto flags = setting.find(flagsKey(key));
            if (flags == setting.end() || value.signature() != "s" || !savedByAgent(variants::toUInt(flags->second)))
                continue;
            entries.push_back({settingName, key, value.value<std::string>()});
        }
    }

    base::dbus::MethodCall pending = call;
    m_store->replace(uuid, name, entries, [pending]() mutable { pending.reply(); });
}

void NetworkSecretAgent::deleteSecrets(base::dbus::MethodCall &call)
{
    if (!fromNetworkManager(call.message())) {
        call.replyError(kPermissionDenied, "Only NetworkManager may ask for secrets");
        return;
    }
    Settings connection;
    ObjectPath path;
    call.read(connection, path);
    const std::string uuid = variants::toString(variants::value(settingOf(connection, kConnectionSetting), "uuid"));
    if (uuid.empty()) {
        call.reply();
        return;
    }
    base::dbus::MethodCall pending = call;
    m_store->remove(uuid, [pending]() mutable { pending.reply(); });
}

void NetworkSecretAgent::connectWireless(const std::string &devicePath, const std::string &accessPointPath,
                                         const std::string &name, bool wep)
{
    if (!m_requests.empty() && m_requests.front().accessPointPath == accessPointPath)
        return;

    // Another network was picked: the prompt switches to it, and whatever
    // Wi-Fi password was being asked for is given up, as connecting to this
    // one would.
    for (size_t i = m_requests.size(); i-- > 0;) {
        const Request &request = m_requests[i];
        if (request.accessPointPath.empty()) {
            if (request.settingName != kWirelessSecurity && request.settingName != kIeee8021x)
                continue;
            fail(request, kUserCanceled, "Another network was chosen");
        }
        m_requests.erase(m_requests.begin() + long(i));
    }

    Request request;
    request.id = ++m_nextId;
    request.connectionName = name;
    request.kind = wep ? Kind::WepKey : Kind::WpaPassword;
    request.interactive = true;
    request.devicePath = devicePath;
    request.accessPointPath = accessPointPath;
    if (m_client)
        request.deviceActive = m_client->activeConnection(devicePath);
    m_requests.push_front(request);
    requestChanged.emit();
}

void NetworkSecretAgent::respond(const std::string &secret)
{
    if (m_requests.empty())
        return;
    const Request request = m_requests.front();

    if (!request.accessPointPath.empty()) {
        if (!m_handler)
            m_handler = std::make_unique<networkmanager::Handler>();
        m_handler->addAndActivateConnection(request.devicePath, request.accessPointPath, secret);
        finish();
        return;
    }

    reply(request, {{request.settingName, {{request.key, Variant(secret)}}}});

    // NetworkManager hands typed secrets back to no agent: storing them is the agent's job.
    if (!request.uuid.empty() && savedByAgent(request.secretFlags))
        m_store->save(request.uuid, request.connectionName, request.settingName, request.key, secret);

    finish();
}

void NetworkSecretAgent::respondFields(const std::map<std::string, std::string> &values)
{
    if (m_requests.empty() || m_requests.front().kind != Kind::VpnSecrets)
        return;

    Request request = m_requests.front();
    for (const auto &[key, value] : values)
        request.values[key] = value;
    replyVpn(request, request.values);

    // As for Wi-Fi, typed secrets the agent keeps are the agent's to store.
    for (const VpnSecretField &field : request.fields) {
        const auto value = values.find(field.key);
        if (value == values.end() || value->second.empty() || request.uuid.empty())
            continue;
        if (savedByAgent(stringFlags(request.vpnData, field.key)))
            m_store->save(request.uuid, request.connectionName, "vpn", field.key, value->second);
    }

    finish();
}

void NetworkSecretAgent::cancel()
{
    if (m_requests.empty())
        return;
    if (m_requests.front().accessPointPath.empty())
        fail(m_requests.front(), kUserCanceled, "Canceled by the user");
    finish();
}

void NetworkSecretAgent::reply(const Request &request, const Settings &secrets)
{
    if (request.call) {
        base::dbus::MethodCall call = *request.call;
        call.reply(secrets);
    }
}

void NetworkSecretAgent::fail(const Request &request, const std::string &error, const std::string &text)
{
    if (request.call) {
        base::dbus::MethodCall call = *request.call;
        call.replyError(error, text);
    }
}

void NetworkSecretAgent::ask(const Request &request)
{
    if (!request.interactive
        || (request.key.empty() && request.fields.empty() && request.accessPointPath.empty())) {
        // Secrets read through Settings.Connection.GetSecrets expect an answer, not an error.
        if (request.noErrors)
            reply(request, {{request.settingName, {}}});
        else
            fail(request, kNoSecrets, "No " + request.settingName + " secrets to offer");
        return;
    }

    m_requests.push_back(request);
    if (m_requests.size() == 1)
        requestChanged.emit();
}

void NetworkSecretAgent::finish()
{
    m_requests.pop_front();
    requestChanged.emit();
}

void NetworkSecretAgent::dismissStalePrompts()
{
    if (!m_client->ready())
        return;

    // A prompt for a network that went out of range, a device that went away,
    // or a device that started connecting elsewhere is out of date.
    bool headChanged = false;
    for (size_t i = m_requests.size(); i-- > 0;) {
        const Request &request = m_requests[i];
        if (request.accessPointPath.empty())
            continue;
        if (m_client->has(request.accessPointPath, networkmanager::accessPointInterface)
            && m_client->has(request.devicePath, networkmanager::deviceInterface)) {
            const std::string active = m_client->activeConnection(request.devicePath);
            if (active.empty() || active == request.deviceActive)
                continue;
        }
        m_requests.erase(m_requests.begin() + long(i));
        headChanged |= i == 0;
    }
    if (headChanged)
        requestChanged.emit();
}

} // namespace Lingmo::Shell
