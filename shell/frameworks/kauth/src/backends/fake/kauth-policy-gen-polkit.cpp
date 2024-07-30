/*
    SPDX-FileCopyrightText: 2008 Nicola Gigante <nicola.gigante@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <auth/policy-gen/policy-gen.h>

#include <QDebug>
#include <QTextStream>
#include <cstdio>

const char header[] =
    ""
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<!DOCTYPE policyconfig PUBLIC\n"
    "\"-//freedesktop//DTD PolicyKit Policy Configuration 1.0//EN\"\n"
    "\"http://www.freedesktop.org/standards/PolicyKit/1.0/policyconfig.dtd\">\n"
    "<policyconfig>\n";

const char policy_tag[] =
    ""
    "      <defaults>\n"
    "         <allow_inactive>no</allow_inactive>\n"
    "         <allow_active>%1</allow_active>\n"
    "      </defaults>\n";

const char dent[] = "   ";

void output(const QList<Action> &actions, const QMap<QString, QString> &domain)
{
    Q_UNUSED(domain)

    QTextStream out(stdout);
    out << header;

    for (const Action &action : std::as_const(actions)) {
        out << dent << "<action id=\"" << action.name << "\" >\n";

        const auto lstKeys = action.descriptions.keys();
        for (const QString &lang : lstKeys) {
            out << dent << dent << "<description";
            if (lang != "en") {
                out << " xml:lang=\"" << lang << '"';
            }
            out << '>' << action.messages.value(lang) << "</description>\n";
        }

        const auto lstMessagesKeys = action.messages.keys();
        for (const QString &lang : lstMessagesKeys) {
            out << dent << dent << "<message";
            if (lang != "en") {
                out << " xml:lang=\"" << lang << '"';
            }
            out << '>' << action.descriptions.value(lang) << "</message>\n";
        }

        QString policy = action.policy;
        if (!action.persistence.isEmpty()) {
            policy += "_keep_" + action.persistence;
        }

        out << QString(policy_tag).arg(policy);

        out << dent << "</action>\n";
    }

    out << "</policyconfig>\n";
}
