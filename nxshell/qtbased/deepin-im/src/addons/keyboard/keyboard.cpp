// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "keyboard.h"

#include "config.h"
#include "dimcore/Events.h"
#include "dimcore/InputContext.h"
#include "dimcore/InputMethodEntry.h"

#define XK_MISCELLANY
#include <X11/keysymdef.h>
#undef XK_MISCELLANY

#include <common/common.h>
#include <libintl.h>

#include <QDir>
#include <QDomDocument>
#include <QList>

using namespace org::deepin::dim;

constexpr uint32_t BUFF_SIZE = 100;

DIM_ADDON_FACTORY(Keyboard)

struct Variant
{
    QString name;
    QString shortDescription;
    QString description;
    QList<QString> languageList;
};

struct Layout
{
    QString name;
    QString shortDescription;
    QString description;
    QList<QString> languageList;
    QList<Variant> variantList;
};

Keyboard::Keyboard(Dim *dim)
    : InputMethodAddon(dim, "keyboard", "keyboard")
{
    ctx_.reset(xkb_context_new(XKB_CONTEXT_NO_FLAGS));

    if (!ctx_) {
        throw std::runtime_error("Failed to create xkb context");
    }

    keymap_.reset(nullptr);
    state_.reset(nullptr);

    QDir dir(QStringLiteral(XKEYBOARDCONFIG_XKBBASE) + QDir::separator() + "rules");
    QString rules = dir.absoluteFilePath(QString("%1.xml").arg(DEFAULT_XKB_RULES));
    QString extraRules = dir.absoluteFilePath(QString("%1.extras.xml").arg(DEFAULT_XKB_RULES));

    parseRule(rules);
    parseRule(extraRules);
}

Keyboard::~Keyboard()
{
    keyboards_.clear();
}

const QList<InputMethodEntry> &Keyboard::getInputMethods()
{
    return keyboards_;
}

void Keyboard::initInputMethods()
{
    Q_EMIT addonInitFinished(this);
}

static std::pair<std::string, std::string> splitLayout(const std::string &layout)
{
    auto pos = layout.find_first_of('_');
    if (pos == std::string::npos) {
        return { layout, "" };
    }

    return {
        { layout.cbegin(), layout.cbegin() + pos },
        { layout.cbegin() + pos + 1, layout.cend() },
    };
}

static bool keysymIsModifier(uint32_t keysym)
{
    return keysym == XK_Control_L || keysym == XK_Control_R || keysym == XK_Meta_L
        || keysym == XK_Meta_R || keysym == XK_Alt_L || keysym == XK_Alt_R || keysym == XK_Super_L
        || keysym == XK_Super_R || keysym == XK_Hyper_L || keysym == XK_Hyper_R
        || keysym == XK_Shift_L || keysym == XK_Shift_R;
}

bool Keyboard::keyEvent(const InputMethodEntry &entry, InputContextKeyEvent &keyEvent)
{
    auto *ic = keyEvent.ic();

    if (keyEvent.isRelease()) {
        return false;
    }

    if (keysymIsModifier(keyEvent.keySym())) {
        return false;
    }

    struct xkb_rule_names names;
    const auto &layout = entry.name();
    if (layout == "us") {
        return false;
    }

    const auto [layoutName, variantName] = splitLayout(layout);

    names.layout = layoutName.c_str();
    names.variant = variantName.c_str();
    names.rules = DEFAULT_XKB_RULES;
    names.model = "";
    names.options = "";
    keymap_.reset(xkb_keymap_new_from_names(ctx_.get(), &names, XKB_KEYMAP_COMPILE_NO_FLAGS));

    keymap_ ? state_.reset(xkb_state_new(keymap_.get())) : state_.reset();

    if (state_) {
        char buf[BUFF_SIZE] = {};
        xkb_keysym_to_utf8(keyEvent.keySym(), buf, BUFF_SIZE);
        if (buf[0] == '\n' || buf[0] == '\r' || buf[0] == '\b' || buf[0] == '\033'
            || buf[0] == '\x7f') {
            return false;
        }

        ic->commitString(buf);
        ic->commit();

        return true;
    }

    return false;
}

void Keyboard::updateSurroundingText(InputContextEvent &event) { }

// static QList<QString> parseLanguageList(const QDomElement &languageListEle) {
//     QList<QString> languageList;
//     for (auto language = languageListEle.firstChildElement("iso639Id"); !language.isNull();
//          language = language.nextSiblingElement("iso639Id")) {
//         auto languageName = language.text();
//         languageList.append(languageName);
//     }

//     return languageList;
// }

void Keyboard::parseLayoutList(const QDomElement &layoutListEle)
{
    for (auto layoutEle = layoutListEle.firstChildElement("layout"); !layoutEle.isNull();
         layoutEle = layoutEle.nextSiblingElement("layout")) {
        auto configItemEle = layoutEle.firstChildElement("configItem");

        const std::string name = configItemEle.firstChildElement("name").text().toStdString();
        const std::string shortDescription =
            configItemEle.firstChildElement("shortDescription").text().toStdString();
        const auto label = shortDescription.empty() ? name : shortDescription;
        const std::string description =
            configItemEle.firstChildElement("description").text().toStdString();
        // QString languageList =
        // parseLanguageList(configItemEle.firstChildElement("languageList"));

        keyboards_.append(InputMethodEntry(key(),
                                           name,
                                           name,
                                           dgettext("xkeyboard-config", description.c_str()),
                                           label,
                                           "input-keyboard"));

        parseVariantList(name, layoutEle.firstChildElement("variantList"));
    }
}

void Keyboard::parseVariantList(const std::string &layoutName, const QDomElement &variantListEle)
{
    for (auto variantEle = variantListEle.firstChildElement("variant"); !variantEle.isNull();
         variantEle = variantEle.nextSiblingElement("variant")) {
        auto configItemEle = variantEle.firstChildElement("configItem");

        const std::string name = configItemEle.firstChildElement("name").text().toStdString();
        const std::string shortDescription =
            configItemEle.firstChildElement("shortDescription").text().toStdString();
        const auto label = shortDescription.empty() ? name : shortDescription;
        const std::string description =
            configItemEle.firstChildElement("description").text().toStdString();
        // QString languageList =
        // parseLanguageList(configItemEle.firstChildElement("languageList"));

        const std::string fullname = layoutName + "_" + name;

        keyboards_.append(InputMethodEntry(key(),
                                           fullname,
                                           fullname,
                                           dgettext("xkeyboard-config", description.c_str()),
                                           label,
                                           "input-keyboard"));
    }
}

void Keyboard::parseRule(const QString &file)
{
    QFile xmlFile(file);
    if (!xmlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QDomDocument xmlReader;
    xmlReader.setContent(&xmlFile);
    auto layoutListEle = xmlReader.documentElement().firstChildElement("layoutList");

    parseLayoutList(layoutListEle);
}
