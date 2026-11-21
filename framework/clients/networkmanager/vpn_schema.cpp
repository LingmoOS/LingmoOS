// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/networkmanager/vpn_schema.h"
#include "lingmo/networkmanager/text.h"
#include "lingmo/networkmanager/types.h"
#include "framework_i18n.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>

using Lingmo::framework::tr;
using Lingmo::i18n::format;

namespace Lingmo::Networkmanager::vpn::schema {

namespace {

// Builds one kind's table; the helpers keep each field to a line.
struct Table
{
    std::vector<Field> fields;
    std::string section;
    bool advanced = false;

    Field& add(const std::string& key, const std::string& label, Type type)
    {
        Field field;
        field.key = key;
        field.label = label;
        field.type = type;
        field.advanced = advanced;
        field.section = section;
        fields.push_back(field);
        return fields.back();
    }

    Field& text(const std::string& key, const std::string& label) { return add(key, label, Type::Text); }
    Field& file(const std::string& key, const std::string& label) { return add(key, label, Type::File); }
    Field& flag(const std::string& key, const std::string& label) { return add(key, label, Type::Bool); }

    Field& password(const std::string& key, const std::string& label, bool allowNotRequired = false)
    {
        Field& field = add(key, label, Type::Password);
        field.allowNotRequired = allowNotRequired;
        return field;
    }

    Field& number(const std::string& key, const std::string& label, int minimum, int maximum,
                  const std::string& defaultValue = {})
    {
        Field& field = add(key, label, Type::Number);
        field.minimum = minimum;
        field.maximum = maximum;
        field.defaultValue = defaultValue;
        field.placeholder = defaultValue;
        return field;
    }

    Field& choice(const std::string& key, const std::string& label, const std::vector<Choice>& choices,
                  const std::string& defaultValue = {})
    {
        Field& field = add(key, label, Type::Choice);
        field.choices = choices;
        field.defaultValue = defaultValue;
        return field;
    }

    // The PPP options the pppd-based plugins share.
    void ppp(bool authentication = true)
    {
        section = tr("Authentication methods");
        if (authentication) {
            flag("refuse-pap", "PAP").inverted = true;
            flag("refuse-chap", "CHAP").inverted = true;
            flag("refuse-mschap", "MSCHAP").inverted = true;
            flag("refuse-mschapv2", "MSCHAPv2").inverted = true;
            flag("refuse-eap", "EAP").inverted = true;
        }
        section = tr("Encryption and compression");
        choice("require-mppe", tr("Point-to-point encryption"),
               {{"", tr("Not required")},
                {"require-mppe", tr("Any")},
                {"require-mppe-128", tr("128-bit")},
                {"require-mppe-40", tr("40-bit")}})
            .exclusiveKeys = true;
        flag("mppe-stateful", tr("Stateful encryption"));
        flag("nobsdcomp", tr("BSD compression")).inverted = true;
        flag("nodeflate", tr("Deflate compression")).inverted = true;
        flag("no-vj-comp", tr("TCP header compression")).inverted = true;
        section = tr("Link");
        Field& echo = flag("lcp-echo-interval", tr("Send PPP echo packets"));
        echo.trueValue = "30";
        echo.alsoSet["lcp-echo-failure"] = "5";
    }
};

} // namespace

static std::vector<Choice> tlsVersions()
{
    return {{"", tr("Default")},
            {"1.0", "1.0"},
            {"1.1", "1.1"},
            {"1.2", "1.2"},
            {"1.3", "1.3"}};
}

static std::vector<Choice> dhGroups(const std::string& defaultLabel, const std::vector<std::string>& extra = {})
{
    std::vector<Choice> choices{{"", defaultLabel}};
    for (const std::string& value : extra)
        choices.push_back(Choice{value, value});
    for (const char* group : {"dh1", "dh2", "dh5", "dh14", "dh15", "dh16", "dh17", "dh18"})
        choices.push_back(Choice{group, format(tr("DH group {number}"), {{"number", group + 2}})});
    return choices;
}

static Table openvpn()
{
    Table t;
    t.text("remote", tr("Server")).required = true;
    // TRANSLATORS: keep "host[:port[:proto]]" as it is; it is the syntax the field takes.
    t.fields.back().placeholder = tr("host[:port[:proto]], several separated by commas");
    t.choice("connection-type", tr("Sign in with"),
             {{"password", tr("User name and password")},
              {"tls", tr("Certificates")},
              {"password-tls", tr("User name, password and certificates")},
              {"static-key", tr("Static key")}},
             "tls");
    const std::string withPassword = "connection-type=password|password-tls";
    const std::string withCertificates = "connection-type=tls|password-tls";
    t.text("username", tr("User name")).showWhen = withPassword;
    t.password("password", tr("Password"), true).showWhen = withPassword;
    t.file("ca", tr("CA certificate")).showWhen = "connection-type=tls|password|password-tls";
    t.file("cert", tr("User certificate")).showWhen = withCertificates;
    t.file("key", tr("Private key")).showWhen = withCertificates;
    t.password("cert-pass", tr("Private key password"), true).showWhen = withCertificates;
    t.file("static-key", tr("Static key")).showWhen = "connection-type=static-key";
    t.choice("static-key-direction", tr("Key direction"),
             {{"", tr("None")}, {"0", "0"}, {"1", "1"}})
            .showWhen = "connection-type=static-key";
    t.text("remote-ip", tr("Remote IP address")).showWhen = "connection-type=static-key";
    t.text("local-ip", tr("Local IP address")).showWhen = "connection-type=static-key";

    t.advanced = true;
    t.section = tr("General");
    t.number("port", tr("Port"), 1, 65535, "1194");
    t.flag("proto-tcp", tr("Use TCP"));
    t.choice("dev-type", tr("Device type"),
             {{"", tr("Automatic")}, {"tun", "TUN"}, {"tap", "TAP"}});
    t.text("dev", tr("Device name"));
    t.number("tunnel-mtu", tr("Tunnel MTU"), 1, 65535, "1500");
    t.number("fragment-size", tr("UDP fragment size"), 1, 65535, "1300");
    t.flag("mssfix", tr("Restrict TCP maximum segment size"));
    t.choice("mtu-disc", tr("Path MTU discovery"),
             {{"", tr("Default")}, {"no", tr("Never")},
              {"maybe", tr("When possible")}, {"yes", tr("Always")}});
    t.number("reneg-seconds", tr("Renegotiate every (seconds)"), 0, 604800, "0");
    t.choice("compress", tr("Compression"),
             {{"", tr("None")}, {"lzo", "LZO"},
              {"lz4", "LZ4"}, {"lz4-v2", "LZ4 v2"},
              {"yes", tr("Automatic")}});
    t.choice("allow-compression", tr("Allow compression"),
             {{"", tr("Default")}, {"asym", tr("Incoming only")},
              {"yes", tr("Both ways")}, {"no", tr("No")}});
    t.flag("remote-random", tr("Pick servers at random"));
    t.flag("remote-random-hostname", tr("Randomise server host names"));
    t.flag("allow-pull-fqdn", tr("Allow pulled host names"));
    t.flag("tun-ipv6", tr("IPv6 inside the tunnel"));
    t.flag("float", tr("Accept the server from any address"));
    t.flag("push-peer-info", tr("Send client details to the server"));
    t.number("ping", tr("Ping every (seconds)"), 1, 65535, "30");
    t.number("ping-exit", tr("Exit without a ping after (seconds)"), 1, 65535, "30");
    t.number("ping-restart", tr("Restart without a ping after (seconds)"), 1, 65535, "30");
    t.number("connect-timeout", tr("Connection timeout (seconds)"), 1, 65535, "120");
    t.number("max-routes", tr("Maximum routes"), 1, 65535, "100");

    t.section = tr("Security");
    t.text("cipher", tr("Cipher")).placeholder = tr("Default");
    t.text("data-ciphers", tr("Data ciphers")).placeholder = tr("Default");
    t.text("data-ciphers-fallback", tr("Data cipher fallback")).placeholder = tr("Default");
    t.flag("ncp-disable", tr("Disable cipher negotiation"));
    std::vector<Choice> hmac{{"", tr("Default")}, {"none", tr("None")}};
    for (const char* name : {"MD5", "SHA1", "SHA224", "SHA256", "SHA384", "SHA512", "RIPEMD160", "RSA-MD4"})
        hmac.push_back(Choice{name, name});
    t.choice("auth", tr("HMAC authentication"), hmac);
    t.number("keysize", tr("Key size (bits)"), 1, 65535, "128");
    t.file("crl-verify-file", tr("Revocation list file"));
    t.file("crl-verify-dir", tr("Revocation list directory"));

    t.section = "TLS";
    t.text("verify-x509-name", tr("Server certificate name"));
    // TRANSLATORS: keep the "name:", "subject:" and "name-prefix:" prefixes as they are.
    t.fields.back().placeholder = tr("name:vpn.example.com, subject:… or name-prefix:…");
    t.choice("remote-cert-tls", tr("Server certificate usage"),
             {{"", tr("Not checked")}, {"server", tr("Server")}, {"client", tr("Client")}});
    t.choice("ns-cert-type", tr("Netscape certificate type"),
             {{"", tr("Not checked")}, {"server", tr("Server")}, {"client", tr("Client")}});
    t.file("ta", tr("TLS authentication key"));
    t.choice("ta-dir", tr("TLS key direction"),
             {{"", tr("None")}, {"0", "0"}, {"1", "1"}});
    t.file("tls-crypt", tr("TLS encryption key"));
    t.file("tls-crypt-v2", tr("TLS encryption key (v2)"));
    t.file("extra-certs", tr("Extra certificates"));
    t.text("tls-cipher", tr("TLS cipher")).placeholder = tr("Default");
    t.choice("tls-version-min", tr("Minimum TLS version"), tlsVersions());
    t.flag("tls-version-min-or-highest", tr("Or the highest the server supports"));
    t.choice("tls-version-max", tr("Maximum TLS version"), tlsVersions());

    t.section = tr("Proxy");
    t.choice("proxy-type", tr("Proxy"),
             {{"", tr("None")}, {"http", "HTTP"}, {"socks", "SOCKS"}});
    const std::string withProxy = "proxy-type=http|socks";
    t.text("proxy-server", tr("Proxy server")).showWhen = withProxy;
    t.number("proxy-port", tr("Proxy port"), 1, 65535).showWhen = withProxy;
    t.flag("proxy-retry", tr("Retry indefinitely")).showWhen = withProxy;
    t.text("http-proxy-username", tr("Proxy user name")).showWhen = "proxy-type=http";
    t.password("http-proxy-password", tr("Proxy password"), true).showWhen = "proxy-type=http";
    return t;
}

static Table vpnc()
{
    Table t;
    t.text("IPSec gateway", tr("Server")).required = true;
    t.text("IPSec ID", tr("Group name")).required = true;
    t.text("Xauth username", tr("User name"));
    Field& user = t.password("Xauth password", tr("Password"), true);
    user.legacyTypeKey = "xauth-password-type";
    Field& group = t.password("IPSec secret", tr("Group password"), true);
    group.legacyTypeKey = "ipsec-secret-type";
    t.flag("IKE Authmode", tr("Hybrid authentication")).trueValue = "hybrid";
    t.file("CA-File", tr("CA certificate")).showWhen = "IKE Authmode=hybrid";

    t.advanced = true;
    t.section = tr("General");
    t.text("Domain", tr("Domain"));
    t.choice("Vendor", tr("Vendor"),
             {{"", "Cisco"}, {"netscreen", "Netscreen"},
              {"fortigate", "Fortigate"}});
    t.text("Application Version", tr("Application version"));
    t.number("Interface MTU", tr("Interface MTU"), 1200, 12000);
    t.section = tr("Security");
    t.flag("Enable Single DES", tr("Weak encryption (single DES)"));
    t.flag("Enable no encryption", tr("No encryption"));
    t.flag("Enable weak authentication", tr("Weak authentication"));
    t.flag("Enable weak encryption", tr("Weak encryption"));
    t.choice("NAT Traversal Mode", tr("NAT traversal"),
             {{"", tr("NAT-T when detected")}, {"force-natt", tr("NAT-T always")},
              {"cisco-udp", "Cisco UDP"}, {"none", tr("Off")}});
    t.choice("IKE DH Group", tr("IKE DH group"), dhGroups(format(tr("DH group {number}"), {{"number", "2"}})));
    t.choice("Perfect Forward Secrecy", tr("Perfect forward secrecy"),
             dhGroups(tr("As the server says"), {"nopfs"}));
    t.number("Local Port", tr("Local port"), 0, 65535, "0");
    t.number("DPD idle timeout (our side)", tr("Dead peer detection (seconds, 0 is off)"), 0, 86400, "90");
    return t;
}

static Table l2tp()
{
    Table t;
    t.text("gateway", tr("Server")).required = true;
    t.choice("user-auth-type", tr("Sign in with"),
             {{"", tr("User name and password")}, {"tls", tr("Certificates")}});
    const std::string withPassword = "user-auth-type=";
    const std::string withCertificates = "user-auth-type=tls";
    t.text("user", tr("User name")).showWhen = withPassword;
    t.password("password", tr("Password"), true).showWhen = withPassword;
    t.text("domain", tr("NT domain")).showWhen = withPassword;
    t.file("user-ca", tr("CA certificate")).showWhen = withCertificates;
    t.file("user-cert", tr("User certificate")).showWhen = withCertificates;
    t.file("user-key", tr("Private key")).showWhen = withCertificates;
    t.password("user-certpass", tr("Private key password"), true).showWhen = withCertificates;

    t.advanced = true;
    t.section = "IPsec";
    t.flag("ipsec-enabled", tr("IPsec tunnel to the server"));
    const std::string withIpsec = "ipsec-enabled=yes";
    t.text("ipsec-remote-id", tr("Server identity")).showWhen = withIpsec;
    t.choice("machine-auth-type", tr("Machine authentication"),
             {{"", tr("Pre-shared key")}, {"tls", tr("Certificates")}})
            .showWhen = withIpsec;
    t.password("ipsec-psk", tr("Pre-shared key"), true).showWhen = "machine-auth-type=";
    t.file("machine-ca", tr("Machine CA certificate")).showWhen = "machine-auth-type=tls";
    t.file("machine-cert", tr("Machine certificate")).showWhen = "machine-auth-type=tls";
    t.file("machine-key", tr("Machine private key")).showWhen = "machine-auth-type=tls";
    t.password("machine-certpass", tr("Machine key password"), true).showWhen = "machine-auth-type=tls";
    t.text("ipsec-ike", tr("Phase 1 algorithms")).showWhen = withIpsec;
    t.text("ipsec-esp", tr("Phase 2 algorithms")).showWhen = withIpsec;
    t.number("ipsec-ikelifetime", tr("Phase 1 lifetime (seconds)"), 0, 1000000).showWhen = withIpsec;
    t.number("ipsec-salifetime", tr("Phase 2 lifetime (seconds)"), 0, 1000000).showWhen = withIpsec;
    t.flag("ipsec-forceencaps", tr("Force UDP encapsulation")).showWhen = withIpsec;
    t.flag("ipsec-ipcomp", tr("IP compression")).showWhen = withIpsec;
    t.flag("ipsec-ikev2", tr("Use IKEv2")).showWhen = withIpsec;
    t.flag("ipsec-pfs", tr("Disable perfect forward secrecy")).showWhen = withIpsec;
    t.fields.back().trueValue = "no";
    t.ppp();
    t.section = tr("Link");
    t.number("mtu", "MTU", 575, 1500, "1400");
    t.number("mru", "MRU", 575, 1500, "1400");
    t.number("mrru", tr("Multilink MRRU"), 1500, 4500);
    t.flag("ephemeral-port", tr("Use a random source port"));
    return t;
}

static Table pptp()
{
    Table t;
    t.text("gateway", tr("Server")).required = true;
    t.text("user", tr("User name"));
    t.password("password", tr("Password"), true);
    t.text("domain", tr("NT domain"));
    t.advanced = true;
    t.ppp();
    t.number("unit", tr("PPP unit number"), 0, 65535);
    return t;
}

static Table sstp()
{
    Table t;
    t.text("gateway", tr("Server")).required = true;
    t.fields.back().placeholder = "host[:port]";
    t.choice("connection-type", tr("Sign in with"),
             {{"", tr("User name and password")}, {"tls", tr("Certificates")}});
    const std::string withPassword = "connection-type=";
    const std::string withCertificates = "connection-type=tls";
    t.text("user", tr("User name")).showWhen = withPassword;
    t.password("password", tr("Password"), true).showWhen = withPassword;
    t.text("domain", tr("NT domain")).showWhen = withPassword;
    t.file("tls-ca-cert", tr("CA certificate")).showWhen = withCertificates;
    t.file("tls-user-cert", tr("User certificate")).showWhen = withCertificates;
    t.file("tls-user-key", tr("Private key")).showWhen = withCertificates;
    t.password("tls-user-key-secret", tr("Private key password"), true).showWhen = withCertificates;

    t.advanced = true;
    t.section = tr("Certificates");
    t.file("ca-cert", tr("Server CA certificate"));
    t.file("crl-file", tr("Revocation list"));
    t.flag("ignore-cert-warn", tr("Ignore certificate warnings"));
    t.flag("tls-ext", tr("TLS hostname extension")).falseValue = "no";
    t.fields.back().defaultValue = "yes";
    t.section = "TLS";
    t.text("tls-identity", tr("Identity"));
    t.choice("tls-verify-method", tr("Verify the server name by"),
             {{"", tr("Default")}, {"none", tr("Nothing")},
              {"subject", tr("Subject")}, {"name", tr("Name")},
              {"suffix", tr("Name suffix")}});
    t.text("tls-remotename", tr("Expected server name"));
    t.flag("tls-verify-key-usage", tr("Check key usage"));
    t.choice("tls-max-version", tr("Maximum TLS version"), tlsVersions());
    t.ppp();
    for (Field& field : t.fields) {
        if (field.key == "refuse-pap" || field.key == "refuse-chap")
            field.defaultValue = "yes";
    }
    t.section = tr("Link");
    t.number("unit", tr("PPP unit number"), 0, 65535);
    t.number("mtu", "MTU", 1, 65535);
    t.section = tr("Proxy");
    t.text("proxy-server", tr("Proxy server"));
    t.number("proxy-port", tr("Proxy port"), 1, 65535);
    t.text("proxy-user", tr("Proxy user name"));
    t.password("proxy-password", tr("Proxy password"), true);
    return t;
}

static Table strongswan()
{
    Table t;
    t.text("address", tr("Server")).required = true;
    t.file("certificate", tr("Server CA certificate"));
    t.text("remote-identity", tr("Server identity"));
    t.choice("method", tr("Sign in with"),
             {{"eap", tr("User name and password (EAP)")},
              {"psk", tr("Pre-shared key")},
              {"cert", tr("Certificate")},
              {"eap-tls", tr("Certificate (EAP-TLS)")},
              {"key", tr("Certificate and private key")},
              {"agent", tr("Certificate and SSH agent")},
              {"smartcard", tr("Smart card")}},
             "eap");
    t.text("user", tr("User name")).showWhen = "method=eap|psk";
    t.choice("cert-source", tr("Certificate from"),
             {{"", tr("A file")}, {"agent", tr("The SSH agent")},
              {"smartcard", tr("A smart card")}})
            .showWhen = "method=cert|eap-tls";
    t.file("usercert", tr("User certificate")).showWhen = "method=cert|eap-tls|key|agent";
    t.file("userkey", tr("Private key")).showWhen = "method=cert|eap-tls|key";
    t.password("password", tr("Password"), true).showWhen = "method=eap|psk|cert|eap-tls|key|smartcard";
    t.text("local-identity", tr("Client identity"));
    t.advanced = true;
    t.section = tr("Options");
    t.number("server-port", tr("Server port"), 1, 65535, "500");
    t.flag("virtual", tr("Request an inner IP address"));
    t.flag("encap", tr("Enforce UDP encapsulation"));
    t.flag("ipcomp", tr("IP compression"));
    t.flag("proposal", tr("Custom algorithm proposals"));
    t.text("ike", tr("IKE proposal")).showWhen = "proposal=yes";
    t.text("esp", tr("ESP proposal")).showWhen = "proposal=yes";
    t.text("local-ts", tr("Local traffic selectors"));
    t.text("remote-ts", tr("Remote traffic selectors"));
    return t;
}

static Table libreswan()
{
    Table t;
    t.text("right", tr("Server")).required = true;
    t.choice("ikev2", tr("Sign in with"),
             {{"", tr("Group password and user name (IKEv1)")},
              {"insist", tr("Certificate (IKEv2)")}});
    const std::string xauth = "ikev2=";
    const std::string certificate = "ikev2=insist";
    t.text("leftid", tr("Group name")).showWhen = xauth;
    Field& group = t.password("pskvalue", tr("Group password"), true);
    group.showWhen = xauth;
    group.legacyTypeKey = "pskinputmodes";
    t.text("leftxauthusername", tr("User name")).showWhen = xauth;
    Field& user = t.password("xauthpassword", tr("Password"), true);
    user.showWhen = xauth;
    user.legacyTypeKey = "xauthpasswordinputmodes";
    t.text("leftcert", tr("Certificate name")).showWhen = certificate;
    t.text("rightid", tr("Server identity"));
    t.advanced = true;
    t.section = tr("Security");
    t.text("ike", tr("Phase 1 algorithms"));
    t.text("esp", tr("Phase 2 algorithms"));
    t.text("ikelifetime", tr("Phase 1 lifetime"));
    t.text("salifetime", tr("Phase 2 lifetime"));
    t.flag("rekey", tr("Disable rekeying")).trueValue = "no";
    t.flag("pfs", tr("Disable perfect forward secrecy")).trueValue = "no";
    t.choice("authby", tr("Authenticate by"),
             {{"", tr("Default")}, {"secret", tr("Pre-shared key")},
              {"rsasig", tr("RSA signature")}, {"null", tr("None")}});
    t.section = tr("Networks");
    t.text("Domain", tr("Domain"));
    t.text("leftsubnet", tr("Local subnet"));
    t.text("rightsubnet", tr("Remote subnet"));
    t.flag("narrowing", tr("Allow narrowing"));
    t.flag("mobike", tr("Mobility (MOBIKE)"));
    t.flag("leftmodecfgclient", tr("Don't ask the server for an address")).trueValue = "no";
    t.choice("fragmentation", tr("IKE fragmentation"),
             {{"", tr("Default")}, {"no", tr("Off")}, {"force", tr("Forced")}});
    t.text("ipsec-interface", tr("IPsec interface"));
    t.section = tr("Dead peer detection");
    t.number("dpddelay", tr("Delay (seconds)"), 0, 86400);
    t.number("dpdtimeout", tr("Timeout (seconds)"), 0, 86400);
    t.choice("dpdaction", tr("Action"),
             {{"", tr("Default")}, {"hold", tr("Hold")},
              {"clear", tr("Clear")}, {"restart", tr("Restart")}});
    t.section = tr("Certificates");
    t.text("rightcert", tr("Server certificate"));
    t.text("rightca", tr("Server CA"));
    t.choice("leftsendcert", tr("Send the certificate"),
             {{"", tr("Default")}, {"always", tr("Always")},
              {"ifasked", tr("If asked")}, {"never", tr("Never")}});
    t.flag("require-id-on-certificate", tr("Accept certificates without the identity")).trueValue = "no";
    return t;
}

static Table fortisslvpn()
{
    Table t;
    t.text("gateway", tr("Server")).required = true;
    t.text("user", tr("User name"));
    t.password("password", tr("Password"), true);
    t.file("ca", tr("CA certificate"));
    t.file("cert", tr("User certificate"));
    t.file("key", tr("Private key"));
    t.advanced = true;
    t.section = tr("Options");
    t.text("trusted-cert", tr("Trusted certificate fingerprint"));
    t.text("realm", tr("Realm"));
    Field& otp = t.flag("otp-flags", tr("Ask for a one-time password"));
    otp.trueValue = std::to_string(SecretNotSaved);
    Field& tfa = t.flag("2fa-flags", tr("Two-factor authentication"));
    tfa.trueValue = std::to_string(SecretAgentOwned);
    return t;
}

static Table ssh()
{
    Table t;
    t.text("remote", tr("Server")).required = true;
    t.choice("auth-type", tr("Sign in with"),
             {{"", tr("The SSH agent")}, {"password", tr("Password")},
              {"key", tr("Key file")}});
    t.password("password", tr("Password"), true).showWhen = "auth-type=password";
    t.file("key-file", tr("Key file")).showWhen = "auth-type=key";
    t.text("remote-ip", tr("Remote IP address")).required = true;
    t.text("local-ip", tr("Local IP address")).required = true;
    t.text("netmask", tr("Netmask")).required = true;
    t.flag("ip-6", tr("IPv6 addresses too"));
    t.text("remote-ip-6", tr("Remote IPv6 address")).showWhen = "ip-6=yes";
    t.text("local-ip-6", tr("Local IPv6 address")).showWhen = "ip-6=yes";
    t.text("netmask-6", tr("IPv6 prefix")).showWhen = "ip-6=yes";
    t.advanced = true;
    t.section = tr("Options");
    t.number("port", tr("Port"), 1, 65535, "22");
    t.number("tunnel-mtu", tr("Tunnel MTU"), 1, 65535, "1500");
    t.text("remote-username", tr("Remote user name")).placeholder = "root";
    t.number("remote-dev", tr("Remote device number"), 0, 255, "100");
    t.flag("tap-dev", tr("TAP device"));
    t.flag("no-default-route", tr("No default route"));
    t.text("extra-opts", tr("Extra SSH options"));
    return t;
}

static Table iodine()
{
    Table t;
    t.text("topdomain", tr("Tunnel domain")).required = true;
    t.text("nameserver", tr("Name server"));
    t.password("password", tr("Password"));
    t.advanced = true;
    t.section = tr("Options");
    t.number("fragsize", tr("Fragment size"), 0, 65535);
    return t;
}

static Table openconnect()
{
    Table t;
    t.choice("protocol", tr("Protocol"),
             {{"", tr("Cisco AnyConnect or openconnect")},
              {"nc", "Juniper Network Connect"},
              {"gp", "Palo Alto GlobalProtect"},
              {"pulse", "Pulse Connect Secure"},
              {"f5", "F5 BIG-IP"},
              {"fortinet", "Fortinet SSL VPN"},
              {"array", "Array Networks"}});
    t.text("gateway", tr("Server")).required = true;
    t.file("cacert", tr("CA certificate"));
    t.file("usercert", tr("User certificate"));
    t.file("userkey", tr("Private key"));
    t.advanced = true;
    t.section = tr("Options");
    t.text("proxy", tr("Proxy"));
    // TRANSLATORS: keep "http://" and "socks://" as they are.
    t.fields.back().placeholder = tr("http://… or socks://…");
    t.text("useragent", tr("User agent"));
    t.choice("reported_os", tr("Reported operating system"),
             {{"", tr("Default")}, {"linux", "Linux"},
              {"linux-64", tr("Linux 64-bit")}, {"win", "Windows"},
              {"mac-intel", "Mac"}, {"android", "Android"},
              {"apple-ios", "iOS"}});
    t.flag("disable_udp", tr("Disable UDP (DTLS)"));
    t.flag("prevent_invalid_cert", tr("Refuse invalid server certificates"));
    t.flag("pem_passphrase_fsid", tr("Private key passphrase from the file system ID"));
    t.section = tr("Trojan");
    t.flag("enable_csd_trojan", tr("Run the server's compliance script"));
    t.file("csd_wrapper", tr("Wrapper script")).showWhen = "enable_csd_trojan=yes";
    t.section = tr("Machine certificate");
    t.file("mcacert", tr("Machine certificate"));
    t.file("mcakey", tr("Machine private key"));
    t.section = tr("Software token");
    t.choice("stoken_source", tr("Token"),
             {{"", tr("None")}, {"stokenrc", tr("RSA SecurID from ~/.stokenrc")},
              {"manual", "RSA SecurID"}, {"totp", "TOTP"},
              {"hotp", "HOTP"}, {"yubioath", "Yubikey OATH"}});
    t.password("stoken_string", tr("Token secret")).showWhen = "stoken_source=manual|totp|hotp|yubioath";
    return t;
}

static const std::map<std::string, Table>& tables()
{
    static const std::map<std::string, Table> tables{
        {"openvpn", openvpn()},
        {"openconnect", openconnect()},
        {"vpnc", vpnc()},
        {"l2tp", l2tp()},
        {"pptp", pptp()},
        {"sstp", sstp()},
        {"strongswan", strongswan()},
        {"libreswan", libreswan()},
        {"fortisslvpn", fortisslvpn()},
        {"ssh", ssh()},
        {"iodine", iodine()},
    };
    return tables;
}

static bool toBool(const std::string& value)
{
    // Text is true unless empty, "0" or "false" in any case.
    return !value.empty() && value != "0" && text::lower(value) != "false";
}

static std::string legacyType(uint32_t flags)
{
    if (flags & SecretNotRequired)
        return "unused";
    if (flags & SecretNotSaved)
        return "ask";
    return "save";
}

bool shown(const Field& field, const std::vector<Field>& fields, const Values& values)
{
    if (field.showWhen.empty())
        return true;
    const size_t equals = field.showWhen.find('=');
    const std::string key = field.showWhen.substr(0, equals);
    const std::string accepted = equals == std::string::npos ? std::string() : field.showWhen.substr(equals + 1);
    std::string value;
    for (const Field& other : fields) {
        if (other.key != key)
            continue;
        const auto raw = values.find(key);
        if (other.type == Type::Bool)
            value = (raw != values.end() && toBool(raw->second)) != other.inverted ? other.trueValue : other.falseValue;
        else if (raw == values.end())
            value = other.defaultValue;
        else
            value = raw->second;
        break;
    }
    // "key=" accepts the empty value, which split() would drop.
    size_t start = 0;
    while (true) {
        const size_t bar = accepted.find('|', start);
        if (accepted.substr(start, bar == std::string::npos ? std::string::npos : bar - start) == value)
            return true;
        if (bar == std::string::npos)
            return false;
        start = bar + 1;
    }
}

std::vector<std::string> kinds()
{
    return {"openvpn", "openconnect", "vpnc", "l2tp", "pptp", "sstp", "strongswan", "libreswan",
            "fortisslvpn", "ssh", "iodine"};
}

std::vector<Field> fields(const std::string& kind)
{
    const auto found = tables().find(kind);
    return found == tables().end() ? std::vector<Field>() : found->second.fields;
}

std::vector<std::string> importPatterns(const std::string& kind)
{
    if (kind == "openvpn")
        return {"*.ovpn", "*.conf"};
    if (kind == "vpnc")
        return {"*.pcf"};
    if (kind == "l2tp" || kind == "openconnect" || kind == "libreswan")
        return {"*.conf"};
    return {};
}

bool exportable(const std::string& kind)
{
    return !importPatterns(kind).empty();
}

std::vector<FormField> formFields(const std::string& kind, const std::map<std::string, std::string>& data)
{
    const auto valueOf = [&data](const std::string& key) {
        const auto found = data.find(key);
        return found == data.end() ? std::string() : found->second;
    };

    std::vector<FormField> list;
    for (const Field& field : fields(kind)) {
        FormField item;
        static_cast<Field&>(item) = field;
        const std::string stored = valueOf(field.key);
        switch (field.type) {
        case Type::Bool: {
            bool on;
            if (field.exclusiveKeys)
                on = false;
            else if (stored.empty())
                on = field.defaultValue == field.trueValue;
            else
                on = stored == field.trueValue;
            item.checked = on != field.inverted;
            break;
        }
        case Type::Choice: {
            std::string value = stored.empty() ? field.defaultValue : stored;
            if (field.exclusiveKeys) {
                value.clear();
                for (const Choice& choice : field.choices) {
                    if (!choice.value.empty() && valueOf(choice.value) == "yes")
                        value = choice.value;
                }
            }
            const bool known = std::any_of(field.choices.begin(), field.choices.end(),
                                           [&value](const Choice& choice) { return choice.value == value; });
            // A value from another editor stays as it is, shown as itself.
            if (!known)
                item.choices.push_back({value, value});
            item.value = value;
            break;
        }
        case Type::Password: {
            const std::string flags = valueOf(field.key + "-flags");
            item.storage = flags.empty() ? uint32_t(SecretAgentOwned) : uint32_t(std::strtoul(flags.c_str(), nullptr, 10));
            break;
        }
        default:
            item.value = stored;
            break;
        }
        list.push_back(std::move(item));
    }
    return list;
}

static std::string missingMessage(const Field& field)
{
    const std::string& key = field.key;
    if (key == "remote" || key == "gateway" || key == "IPSec gateway" || key == "address" || key == "right")
        return tr("Enter the server.");
    if (key == "IPSec ID")
        return tr("Enter the group name.");
    if (key == "remote-ip")
        return tr("Enter the remote IP address.");
    if (key == "local-ip")
        return tr("Enter the local IP address.");
    if (key == "netmask")
        return tr("Enter the netmask.");
    if (key == "topdomain")
        return tr("Enter the tunnel domain.");
    return format(tr("Fill in “{field}”."), {{"field", field.label}});
}

bool apply(const std::string& kind, const Values& values, const std::map<std::string, uint32_t>& storage,
           const std::map<std::string, std::string>& secrets, bool shared, std::map<std::string, std::string>& data,
           std::map<std::string, std::string>& vpnSecrets, std::string* error)
{
    const std::vector<Field> fields = schema::fields(kind);
    if (fields.empty()) {
        if (error)
            *error = tr("This kind of VPN can't be set up by hand.");
        return false;
    }

    for (const Field& field : fields) {
        // A hidden field keeps what the profile has, as the editors leave it alone too.
        if (!shown(field, fields, values))
            continue;
        const auto raw = values.find(field.key);
        const std::string value = raw == values.end() ? std::string() : raw->second;
        const std::string trimmed = text::trimmed(value);

        switch (field.type) {
        case Type::Bool: {
            const bool on = toBool(value) != field.inverted;
            const bool written =
                field.defaultValue.empty() ? on : (on ? field.trueValue : field.falseValue) != field.defaultValue;
            if (on)
                data[field.key] = field.trueValue;
            else if (!field.falseValue.empty() && written)
                data[field.key] = field.falseValue;
            else
                data.erase(field.key);
            for (const auto& [key, also] : field.alsoSet) {
                if (on)
                    data[key] = also;
                else
                    data.erase(key);
            }
            break;
        }
        case Type::Choice:
            if (field.exclusiveKeys) {
                for (const Choice& choice : field.choices) {
                    if (!choice.value.empty())
                        data.erase(choice.value);
                }
                if (!trimmed.empty())
                    data[trimmed] = "yes";
            } else if (trimmed.empty() || trimmed == field.defaultValue) {
                data.erase(field.key);
            } else {
                data[field.key] = trimmed;
            }
            break;
        case Type::Password: {
            const auto chosen = storage.find(field.key);
            const uint32_t flags = chosen != storage.end() ? chosen->second
                                                           : uint32_t(shared ? SecretSystemOwned : SecretAgentOwned);
            data[field.key + "-flags"] = std::to_string(flags);
            if (!field.legacyTypeKey.empty())
                data[field.legacyTypeKey] = legacyType(flags);
            const auto typed = secrets.find(field.key);
            if (flags & (SecretNotSaved | SecretNotRequired))
                vpnSecrets.erase(field.key);
            else if (typed != secrets.end() && !typed->second.empty())
                vpnSecrets[field.key] = typed->second;
            break;
        }
        case Type::Number:
            if (!trimmed.empty()) {
                char* end = nullptr;
                const long number = std::strtol(trimmed.c_str(), &end, 10);
                const bool ok = end && *end == '\0' && number >= INT32_MIN && number <= INT32_MAX;
                if (!ok || (field.maximum > field.minimum && (number < field.minimum || number > field.maximum))) {
                    if (error)
                        *error = format(tr("{field} must be a number from {minimum} to {maximum}."),
                                        {{"field", field.label},
                                         {"minimum", std::to_string(field.minimum)},
                                         {"maximum", std::to_string(field.maximum)}});
                    return false;
                }
            }
            [[fallthrough]];
        default:
            if (trimmed.empty() && field.required) {
                if (error)
                    *error = missingMessage(field);
                return false;
            }
            if (trimmed.empty())
                data.erase(field.key);
            else
                data[field.key] = trimmed;
            break;
        }
    }
    return true;
}

} // namespace Lingmo::Networkmanager::vpn::schema
