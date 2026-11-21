// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/i18n.h"
#include "pages/network/common/textchecks.h"
#include "pages/network/proxy/proxysettings.h"
#include "ui/button.h"
#include "ui/check_box.h"
#include "ui/color_provider.h"
#include "ui/file_chooser.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/painter.h"
#include "ui/cell_layout.h"
#include "ui/text_field.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/colors.h"
#include "widgets/metrics.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace Lingmo::ui;
using Lingmo::i18n::tr;

namespace settings::network {

static constexpr double kPortWidth = 80;

namespace {

// A hairline across, with air above and below.
class Divider : public View
{
public:
    Divider() { setLayoutHints({.fillWidth = true}); }

    SizeF sizeHint() const override { return {0, metrics::m}; }

protected:
    void paint(Painter& painter) override
    {
        // FishUI's Separator: a RoundedRectangle one device pixel high, centred.
        const double height = 1 / painter.scale();
        painter.drawRoundedRect({0, rounding::centeredStart(geometry().height, height), geometry().width, height}, {},
                                color(ColorId::Separator));
    }
};

class ProxyPage : public Page
{
public:
    explicit ProxyPage(SettingsWindow& window)
        : Page(window, tr("Proxy"))
        , m_fileChooser(&window.window())
    {
        Card* modes = content()->addChild<Card>();
        m_noProxy = addRadio(modes, tr("No proxy"), 0);
        m_noProxy->setName("proxy:none");
        m_auto = addRadio(modes, tr("Use proxy auto configuration URL"), 1);
        m_auto->setVisible(false);
        m_manual = addRadio(modes, tr("Use manually specified proxy configuration"), 2);
        m_manual->setName("proxy:manual");

        m_proxyItem = content()->addChild<Card>();
        m_proxyItem->setName("proxy:servers");

        m_scriptRow = m_proxyItem->addChild<RowLayout>();
        m_scriptRow->setSpacing(metrics::controlSpacing);
        m_script = m_scriptRow->addChild<TextField>(m_proxy.scriptProxy());
        m_script->setLayoutHints({.fillWidth = true});
        m_script->onTextEdited = [this] { markChanged(); };
        Button* selectFile = m_scriptRow->addChild<Button>(tr("Select file"));
        selectFile->onClicked = [this] { m_fileChooser.open(); };
        m_fileChooser.onAccepted = [this](const std::vector<std::string>& paths, const std::string&) {
            if (paths.empty())
                return;
            setField(m_script, paths.front());
            m_proxy.setScriptProxy(paths.front());
        };

        m_grid = m_proxyItem->addChild<GridLayout>(4);
        m_grid->setColumnSpacing(metrics::rowSpacing);
        m_grid->setRowSpacing(metrics::formRowSpacing);
        addLabel(tr("HTTP proxy"));
        m_http = addHostField(m_proxy.httpProxy());
        addLabel(tr("Port"));
        m_httpPort = addPortField(m_proxy.httpProxyPort());
        addSpacer();
        m_forFtp = m_grid->addChild<CheckBox>(tr("Also use this proxy for FTP"));
        m_forFtp->setLayoutHints({.fillWidth = true});
        m_forFtp->setChecked(m_proxy.useSameProxy());
        m_forFtp->onToggled = [this](bool) {
            mirrorFtp(true);
            markChanged();
        };
        addSpacer();
        addSpacer();
        addLabel(tr("FTP proxy"));
        m_ftp = addHostField(m_proxy.ftpProxy());
        addLabel(tr("Port"));
        m_ftpPort = addPortField(m_proxy.ftpProxyPort());
        addLabel(tr("SOCKS proxy"));
        m_socks = addHostField(m_proxy.socksProxy());
        addLabel(tr("Port"));
        m_socksPort = addPortField(m_proxy.socksProxyPort());

        m_divider = m_proxyItem->addChild<Divider>();
        m_saveRow = m_proxyItem->addChild<RowLayout>();
        m_saveRow->setSpacing(metrics::rowSpacing);
        m_saveRow->addChild<View>()->setLayoutHints({.fillWidth = true});
        Button* save = m_saveRow->addChild<Button>(tr("Save"));
        save->onClicked = [this] { saveFields(); };

        m_subscription = m_proxy.changed.connect([this](unsigned fields) { reload(fields); });
        mirrorFtp(false);
        // Filling the fields is no change to save.
        m_needSave = false;
        update();
    }

private:
    RadioButton* addRadio(Card* card, const char* text, int flag)
    {
        RadioButton* radio = card->addChild<RadioButton>(text);
        // The circle, not the button's padding, starts on the text column.
        radio->setPadding({0, 4, 0, 4});
        radio->setGroup(&m_modes);
        radio->setChecked(m_proxy.flag() == flag);
        radio->onToggled = [this, flag](bool checked) {
            if (!checked)
                return;
            m_proxy.setFlag(flag);
            update();
        };
        return radio;
    }

    void addLabel(const char* text)
    {
        m_grid->addChild<Label>(text);
    }

    void addSpacer()
    {
        View* spacer = m_grid->addChild<View>();
        spacer->setLayoutHints({.fallbackWidth = 1});
    }

    TextField* addHostField(const std::string& text)
    {
        TextField* field = m_grid->addChild<TextField>(text);
        field->setLayoutHints({.fillWidth = true});
        field->onTextEdited = [this] {
            mirrorFtp(false);
            markChanged();
        };
        return field;
    }

    TextField* addPortField(const std::string& text)
    {
        TextField* field = m_grid->addChild<TextField>(text);
        field->setLayoutHints({.preferredWidth = kPortWidth});
        field->setValidator(isPortText);
        field->onTextEdited = [this] {
            mirrorFtp(false);
            markChanged();
        };
        return field;
    }

    // While "Also use this proxy for FTP" is on, the FTP fields show the HTTP
    // proxy; off again, the FTP server comes back and the port stays.
    void mirrorFtp(bool toggled)
    {
        const bool same = m_forFtp->isChecked();
        m_ftp->setEnabled(!same);
        m_ftpPort->setEnabled(!same);
        if (same) {
            setField(m_ftp, m_http->text());
            setField(m_ftpPort, m_httpPort->text());
        } else if (toggled) {
            setField(m_ftp, m_proxy.ftpProxy());
        }
    }

    // A field the page fills in itself, which still counts as a change to save.
    void setField(TextField* field, const std::string& text)
    {
        if (field->text() == text)
            return;
        field->setText(text);
        markChanged();
    }

    void markChanged()
    {
        m_needSave = true;
        update();
    }

    void reload(unsigned fields)
    {
        // The page's own values coming back while it saves them.
        if (m_saving)
            return;
        if (fields & NetworkProxy::FlagField) {
            const int flag = m_proxy.flag();
            m_noProxy->setChecked(flag == 0);
            m_auto->setChecked(flag == 1);
            m_manual->setChecked(flag == 2);
        }
        if (fields & NetworkProxy::ScriptField)
            setField(m_script, m_proxy.scriptProxy());
        if (fields & NetworkProxy::HttpField)
            setField(m_http, m_proxy.httpProxy());
        if (fields & NetworkProxy::HttpPortField)
            setField(m_httpPort, m_proxy.httpProxyPort());
        if (fields & NetworkProxy::UseSameProxyField)
            m_forFtp->setChecked(m_proxy.useSameProxy());
        if (!m_forFtp->isChecked()) {
            if (fields & NetworkProxy::FtpField)
                setField(m_ftp, m_proxy.ftpProxy());
            if (fields & NetworkProxy::FtpPortField)
                setField(m_ftpPort, m_proxy.ftpProxyPort());
        }
        if (fields & NetworkProxy::SocksField)
            setField(m_socks, m_proxy.socksProxy());
        if (fields & NetworkProxy::SocksPortField)
            setField(m_socksPort, m_proxy.socksProxyPort());
        mirrorFtp(false);
        update();
    }

    void saveFields()
    {
        m_needSave = false;
        m_saving = true;
        m_proxy.setScriptProxy(m_script->text());
        m_proxy.setHttpProxy(m_http->text());
        m_proxy.setHttpProxyPort(m_httpPort->text());
        m_proxy.setUseSameProxy(m_forFtp->isChecked());
        m_proxy.setFtpProxy(m_ftp->text());
        m_proxy.setFtpProxyPort(m_ftpPort->text());
        m_proxy.setSocksProxy(m_socks->text());
        m_proxy.setSocksProxyPort(m_socksPort->text());
        m_saving = false;
        update();
    }

    void update()
    {
        m_proxyItem->setVisible(!m_noProxy->isChecked());
        m_scriptRow->setVisible(m_auto->isChecked());
        m_grid->setVisible(m_manual->isChecked());
        m_divider->setVisible(m_needSave);
        m_saveRow->setVisible(m_needSave);
        requestLayout();
    }

    NetworkProxy m_proxy;
    FileChooser m_fileChooser;
    Lingmo::base::Subscription m_subscription;
    ButtonGroup m_modes;
    bool m_needSave = false;
    bool m_saving = false;
    RadioButton* m_noProxy;
    RadioButton* m_auto;
    RadioButton* m_manual;
    Card* m_proxyItem;
    RowLayout* m_scriptRow;
    TextField* m_script;
    GridLayout* m_grid;
    TextField* m_http;
    TextField* m_httpPort;
    CheckBox* m_forFtp;
    TextField* m_ftp;
    TextField* m_ftpPort;
    TextField* m_socks;
    TextField* m_socksPort;
    Divider* m_divider;
    RowLayout* m_saveRow;
};

} // namespace

static const PageRegistration registration("proxy", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<ProxyPage>(window);
});

} // namespace settings::network
