/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2011 Laszlo Papp <djszapi@archlinux.us>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "achievementparser.h"
#include "atticautils.h"

using namespace Attica;

Achievement Achievement::Parser::parseXml(QXmlStreamReader &xml)
{
    Achievement achievement;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("id")) {
                achievement.setId(xml.readElementText());
            } else if (xml.name() == QLatin1String("content_id")) {
                achievement.setContentId(xml.readElementText());
            } else if (xml.name() == QLatin1String("name")) {
                achievement.setName(xml.readElementText());
            } else if (xml.name() == QLatin1String("description")) {
                achievement.setDescription(xml.readElementText());
            } else if (xml.name() == QLatin1String("explanation")) {
                achievement.setExplanation(xml.readElementText());
            } else if (xml.name() == QLatin1String("points")) {
                achievement.setPoints(xml.readElementText().toInt());
            } else if (xml.name() == QLatin1String("image")) {
                achievement.setImage(QUrl(xml.readElementText()));
            } else if (xml.name() == QLatin1String("dependencies")) {
                QStringList dependencies = parseXmlDependencies(xml);
                achievement.setDependencies(dependencies);
            } else if (xml.name() == QLatin1String("visibility")) {
                achievement.setVisibility(Achievement::stringToAchievementVisibility(xml.readElementText()));
            } else if (xml.name() == QLatin1String("type")) {
                achievement.setType(Achievement::stringToAchievementType(xml.readElementText()));
            } else if (xml.name() == QLatin1String("options")) {
                QStringList options = parseXmlOptions(xml);
                achievement.setOptions(options);
            } else if (xml.name() == QLatin1String("steps")) {
                achievement.setSteps(xml.readElementText().toInt());
            } else if (xml.name() == QLatin1String("progress")) {
                switch (achievement.type()) {
                case Achievement::FlowingAchievement:
                    achievement.setProgress(QVariant(xml.readElementText().toFloat()));
                    break;
                case Achievement::SteppedAchievement:
                    achievement.setProgress(QVariant(xml.readElementText().toInt()));
                    break;
                case Achievement::NamedstepsAchievement:
                    achievement.setProgress(QVariant(xml.readElementText()));
                    break;
                case Achievement::SetAchievement: {
                    QVariant progress = parseXmlProgress(xml);
                    achievement.setProgress(progress);
                    break;
                }
                default:
                    break;
                }
            }
        } else if (xml.isEndElement() && xml.name() == QLatin1String("achievement")) {
            break;
        }
    }

    return achievement;
}

QStringList Achievement::Parser::parseXmlDependencies(QXmlStreamReader &xml)
{
    QStringList dependencies;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("achievement_id")) {
                dependencies.append(xml.readElementText());
            }
        } else if (xml.isEndElement() && xml.name() == QLatin1String("dependencies")) {
            break;
        }
    }

    return dependencies;
}

QStringList Achievement::Parser::parseXmlOptions(QXmlStreamReader &xml)
{
    QStringList options;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("option")) {
                options.append(xml.readElementText());
            }
        } else if (xml.isEndElement() && xml.name() == QLatin1String("options")) {
            break;
        }
    }

    return options;
}

QVariant Achievement::Parser::parseXmlProgress(QXmlStreamReader &xml)
{
    QStringList progress;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("reached")) {
                progress.append(xml.readElementText());
            }
        } else if (xml.isEndElement() && xml.name() == QLatin1String("progress")) {
            break;
        }
    }

    return progress;
}

QStringList Achievement::Parser::xmlElement() const
{
    return QStringList(QStringLiteral("achievement"));
}
