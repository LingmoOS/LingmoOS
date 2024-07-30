/* SPDX-FileCopyrightText: 2023 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <KAboutData>
#include <KCompressionDevice>
#include <KSvg/Svg>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QSvgRenderer>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

using namespace Qt::Literals::StringLiterals; // for ""_L1

static KSvg::Svg s_ksvg;
static QSvgRenderer s_renderer;

// https://developer.mozilla.org/en-US/docs/Web/SVG/Element#renderable_elements
static const QStringList s_renderableElements = {
    "a"_L1, "circle"_L1, "ellipse"_L1, "foreignObject"_L1, "g"_L1, "image"_L1,
    "line"_L1, "path"_L1, "polygon"_L1, "polyline"_L1, "rect"_L1, // excluding <svg>
    "switch"_L1, "symbol"_L1, "text"_L1, "textPath"_L1, "tspan"_L1, "use"_L1
};

QString joinedStrings(const QStringList &strings)
{
    return strings.join("\", \""_L1).prepend("\""_L1).append("\""_L1);
}

// Translate the current element to (0,0) if possible.
// FIXME: Does not necessarily translate to (0,0) in one go.
void writeElementTranslation(QXmlStreamReader &reader, QXmlStreamWriter &writer, qreal dx, qreal dy)
{
    if ((qIsFinite(dx) && dx != 0) || (qIsFinite(dy) && dy != 0)) {
        writer.writeStartElement(reader.qualifiedName()); // The thing reader has currently read.
        auto attributes = reader.attributes();
        bool wasTranslated = false;
        QString svgTranslate = "translate(%1,%2)"_L1.arg(QString::number(dx), QString::number(dy));
        for (int i = 0; i < attributes.size(); ++i) {
            if (attributes[i].qualifiedName() == "transform"_L1) {
                auto svgTransform = attributes[i].value().toString();
                if (!svgTransform.isEmpty()) {
                    svgTransform += " "_L1;
                }
                attributes[i] = {"transform"_L1, svgTransform + svgTranslate};
                wasTranslated = true;
            }
            writer.writeAttribute(attributes[i]);
        }
        if (!wasTranslated) {
            writer.writeAttribute("transform"_L1, svgTranslate);
        }
    } else {
        writer.writeCurrentToken(reader); // The thing reader has currently read.
    }
}

QMap<QString, QByteArray> splitSvg(const QString &inputArg, const QByteArray &inputContents)
{
    s_renderer.load(inputContents);
    QMap<QString, QByteArray> outputMap; // filename, contents
    QXmlStreamReader reader(inputContents);
    reader.setNamespaceProcessing(false);

    QString stylesheet;

    while (!reader.atEnd() && !reader.hasError()) {
        reader.readNextStartElement();
        if (reader.hasError()) {
            break;
        }

        const auto qualifiedName = reader.qualifiedName();
        const auto attributes = reader.attributes();
        QString id = attributes.value("id"_L1).toString();

        // Skip elements without IDs since they aren't icons.
        // Make sure you don't miss children when you make the output contents though.
        // Also skip hints and groups with the layer1 ID
        if (id.isEmpty() || id.startsWith("hint-"_L1) || (qualifiedName == "g"_L1 && id == "layer1"_L1)) {
            continue;
        }

        // Some SVGs have multiple stylesheets.
        // They really shouldn't, but that's just how it is sometimes.
        // The last stylesheet with the correct ID is the one we will use.
        static const auto s_stylesheetId = "current-color-scheme"_L1;
        if (qualifiedName == "style"_L1 && id == s_stylesheetId) {
            reader.readNext();
            auto text = reader.text();
            if (!text.isEmpty()) {
                stylesheet = text.toString();
            }
            continue;
        }

        // ignore non-renderable elements
        if (!s_renderableElements.contains(qualifiedName)) {
            continue;
        }

        // NOTE: Does not include its own transform.
        QTransform transform = s_renderer.transformForElement(id);
        QRectF mappedRect = transform.mapRect(s_renderer.boundsOnElement(id));

        // Skip invisible renderable elements.
        if (mappedRect.isEmpty()) {
            continue;
        }

        QString outputFilename = id + ".svg"_L1;
        QByteArray outputContents;
        QXmlStreamWriter writer(&outputContents);
        // Start writing document
        writer.setAutoFormatting(true);
        writer.writeStartDocument();

        // <svg>
        writer.writeStartElement("svg"_L1);
        writer.writeDefaultNamespace("http://www.w3.org/2000/svg"_L1);
        writer.writeNamespace("http://www.w3.org/1999/xlink"_L1, "xlink"_L1);
        writer.writeNamespace("http://creativecommons.org/ns#"_L1, "cc"_L1);
        writer.writeNamespace("http://purl.org/dc/elements/1.1/"_L1, "dc"_L1);
        writer.writeNamespace("http://www.w3.org/1999/02/22-rdf-syntax-ns#"_L1, "rdf"_L1);
        writer.writeNamespace("http://www.inkscape.org/namespaces/inkscape"_L1, "inkscape"_L1);
        writer.writeNamespace("http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"_L1, "sodipodi"_L1);
        writer.writeAttribute("width"_L1, QString::number(mappedRect.width()));
        writer.writeAttribute("height"_L1, QString::number(mappedRect.height()));

        // <style>
        writer.writeStartElement("style"_L1);
        writer.writeAttribute("type"_L1, "text/css"_L1);
        writer.writeAttribute("id"_L1, s_stylesheetId);
        // CSS
        writer.writeCharacters(stylesheet);
        writer.writeEndElement();
        // </style>

        // Translation via parent
        auto dx = -mappedRect.x();
        auto dy = -mappedRect.y();
        writeElementTranslation(reader, writer, dx, dy);

        // Write contents until we're no longer writing the current element or any of its children.
        int depth = 0;
        while (depth >= 0 && !reader.atEnd() && !reader.hasError()) {
            reader.readNext();
            if (reader.isStartElement()) {
                ++depth;
            }
            if (reader.isEndElement()) {
                --depth;
            }
            writer.writeCurrentToken(reader);
        }

        if (reader.hasError()) {
            qWarning() << inputArg << "has an error:" << reader.errorString();
            break;
        }

        writer.writeEndElement();
        // </svg>

        writer.writeEndDocument();

        if (!outputFilename.isEmpty() && !outputContents.isEmpty()) {
            outputMap.insert(outputFilename, outputContents);
        }
    }
    return outputMap;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    KAboutData aboutData(app.applicationName(), app.applicationName(), "1.0"_L1,
                         "Splits Plasma/KSVG SVGs into individual SVGs"_L1,
                         KAboutLicense::LGPL_V2, "2023 Noah Davis"_L1);
    aboutData.addAuthor("Noah Davis"_L1, {}, "noahadvs@gmail.com"_L1);
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser commandLineParser;
    commandLineParser.addPositionalArgument("inputs"_L1, "Input files (separated by spaces)"_L1, "inputs..."_L1);
    commandLineParser.addPositionalArgument("output"_L1, "Output folder (optional, must exist). The default output folder is the current working directory."_L1, "[output]"_L1);
    aboutData.setupCommandLine(&commandLineParser);

    commandLineParser.process(app);
    aboutData.processCommandLine(&commandLineParser);

    const QStringList &positionalArguments = commandLineParser.positionalArguments();
    if (positionalArguments.isEmpty()) {
        qWarning() << "The arguments are missing.";
        return 1;
    }

    QFileInfo lastArgInfo(positionalArguments.last());
    if (positionalArguments.size() == 1 && lastArgInfo.isDir()) {
        qWarning() << "Input file arguments are missing.";
        return 1;
    }

    QDir outputDir = lastArgInfo.isDir() ? lastArgInfo.absoluteFilePath() : QDir::currentPath();
    QFileInfo outputDirInfo(outputDir.absolutePath());
    if (!outputDirInfo.isWritable()) {
        // Using the arg instead of just path or filename so the user sees what they typed.
        auto output = lastArgInfo.isDir() ? positionalArguments.last() : QDir::currentPath();
        qWarning() << output << "is not a writable output folder.";
        return 1;
    }

    QStringList inputArgs;
    QStringList ignoredArgs;
    for (int i = 0; i < positionalArguments.size() - lastArgInfo.isDir(); ++i) {
        if (!QFileInfo::exists(positionalArguments[i])) {
            ignoredArgs << positionalArguments[i];
            continue;
        }
        inputArgs << positionalArguments[i];
    }

    if (inputArgs.isEmpty()) {
        qWarning() << "None of the input files could be found.";
        return 1;
    }

    if (!ignoredArgs.isEmpty()) {
        // Using the arg instead of path or filename so the user sees what they typed.
        qWarning() << "The following input files could not be found:";
        qWarning().noquote() << joinedStrings(ignoredArgs);
    }

    bool wasAnyFileWritten = false;
    for (const QString &inputArg : inputArgs) {
        QFileInfo inputInfo(inputArg);

        const QString &absoluteInputPath = inputInfo.absoluteFilePath();
        // Avoid reading from a theme with relative paths by accident.
        s_ksvg.setImagePath(absoluteInputPath);
        if (!s_ksvg.isValid()) {
            qWarning() << inputArg << "is not a valid Plasma theme SVG.";
            continue;
        }

        KCompressionDevice inputFile(absoluteInputPath, KCompressionDevice::GZip);
        if (!inputFile.open(QIODevice::ReadOnly)) {
            qWarning() << inputArg << "could not be read.";
            continue;
        }
        const auto outputMap = splitSvg(inputArg, inputFile.readAll());
        inputFile.close();

        if (outputMap.isEmpty()) {
            qWarning() << inputArg << "could not be split.";
            continue;
        }

        const auto outputSubDirPath = outputDir.absoluteFilePath(inputInfo.baseName());
        outputDir.mkpath(outputSubDirPath);
        QDir outputSubDir(outputSubDirPath);
        QStringList unwrittenFiles;
        QStringList invalidSvgs;
        for (auto it = outputMap.cbegin(); it != outputMap.cend(); ++it) {
            const QString &key = it.key();
            const QByteArray &value = it.value();
            if (key.isEmpty() || value.isEmpty()) {
                unwrittenFiles << key;
                continue;
            }
            const auto absoluteOutputPath = outputSubDir.absoluteFilePath(key);
            QFile outputFile(absoluteOutputPath);
            if (!outputFile.open(QIODevice::WriteOnly)) {
                unwrittenFiles << key;
                continue;
            }
            wasAnyFileWritten |= outputFile.write(value);
            outputFile.close();
            s_renderer.load(absoluteOutputPath);
            if (!s_renderer.isValid()) {
                // Write it even if it isn't valid so that the user can examine the output.
                invalidSvgs << key;
            }
        }
        if (unwrittenFiles.size() == outputMap.size()) {
            qWarning().nospace() << "No files could be written for " << inputArg << ".";
        } else if (!unwrittenFiles.isEmpty()) {
            qWarning().nospace() << "The following files could not be written for " << inputArg << ":";
            qWarning().noquote() << joinedStrings(unwrittenFiles);
        }
        if (!invalidSvgs.isEmpty()) {
            qWarning().nospace() << "The following files written for " << inputArg << " are not valid SVGs:";
            qWarning().noquote() << joinedStrings(invalidSvgs);
        }
    }

    return wasAnyFileWritten ? 0 : 1;
}
