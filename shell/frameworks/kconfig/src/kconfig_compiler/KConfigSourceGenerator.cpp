/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2003 Zack Rusin <zack@kde.org>
    SPDX-FileCopyrightText: 2006 Michaël Larouche <michael.larouche@kdemail.net>
    SPDX-FileCopyrightText: 2008 Allen Winter <winter@kde.org>
    SPDX-FileCopyrightText: 2020 Tomaz Cananbrava <tcanabrava@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KConfigSourceGenerator.h"

#include <QRegularExpression>

KConfigSourceGenerator::KConfigSourceGenerator(const QString &inputFile, const QString &baseDir, const KConfigParameters &cfg, ParseResult &parseResult)
    : KConfigCodeGeneratorBase(inputFile, baseDir, baseDir + cfg.baseName + QLatin1String(".cpp"), cfg, parseResult)
{
}

void KConfigSourceGenerator::start()
{
    KConfigCodeGeneratorBase::start();
    stream() << '\n';
    createHeaders();

    if (!cfg().nameSpace.isEmpty()) {
        stream() << "using namespace " << cfg().nameSpace << ";";
        stream() << "\n\n";
    }

    createPrivateDPointerImplementation();
    createSingletonImplementation();
    createPreamble();
    doConstructor();
    doGetterSetterDPointerMode();
    createDefaultValueGetterSetter();
    createDestructor();
    createNonModifyingSignalsHelper();
    createSignalFlagsHandler();
}

void KConfigSourceGenerator::createHeaders()
{
    QString headerName = cfg().baseName + QLatin1Char('.') + cfg().headerExtension;

    // TODO: Make addQuotes return a string instead of replacing it inplace.
    addQuotes(headerName);

    addHeaders({headerName});
    stream() << '\n';

    addHeaders(cfg().sourceIncludes);
    if (cfg().setUserTexts && cfg().translationSystem == KConfigParameters::KdeTranslation) {
        addHeaders({QStringLiteral("klocalizedstring.h")});
        stream() << '\n';
    }

    // Header required by singleton implementation
    if (cfg().singleton) {
        addHeaders({QStringLiteral("qglobal.h"), QStringLiteral("QFile")});

        // HACK: Add single line to fix test.
        if (cfg().singleton && parseResult.cfgFileNameArg) {
            stream() << '\n';
        }
    }

    if (cfg().singleton && parseResult.cfgFileNameArg) {
        addHeaders({QStringLiteral("QDebug")});
    }

    if (cfg().dpointer && parseResult.hasNonModifySignals) {
        addHeaders({QStringLiteral("QSet")});
    }

    if (cfg().qmlRegistration && cfg().singleton) {
        addHeaders({QStringLiteral("QQmlEngine")});
    }

    if (cfg().singleton) {
        stream() << '\n';
    }
}

void KConfigSourceGenerator::createPrivateDPointerImplementation()
{
    // private class implementation
    if (!cfg().dpointer) {
        return;
    }

    QString group;
    beginNamespaces();
    stream() << "class " << cfg().className << "Private\n";
    stream() << "{\n";
    stream() << "  public:\n";

    // Create Members
    for (const auto *entry : std::as_const(parseResult.entries)) {
        if (entry->group != group) {
            group = entry->group;
            stream() << '\n';
            stream() << "    // " << group << '\n';
        }
        stream() << "    " << cppType(entry->type) << " " << varName(entry->name, cfg());
        if (!entry->param.isEmpty()) {
            stream() << QStringLiteral("[%1]").arg(entry->paramMax + 1);
        }
        stream() << ";\n";
    }
    stream() << "\n    // items\n";

    // Create Items.
    for (const auto *entry : std::as_const(parseResult.entries)) {
        const QString declType = entry->signalList.isEmpty() ? QString(cfg().inherits + QStringLiteral("::Item") + itemType(entry->type))
                                                             : QStringLiteral("KConfigCompilerSignallingItem");

        stream() << "    " << declType << " *" << itemVar(entry, cfg());
        if (!entry->param.isEmpty()) {
            stream() << QStringLiteral("[%1]").arg(entry->paramMax + 1);
        }
        stream() << ";\n";
    }

    if (parseResult.hasNonModifySignals) {
        stream() << "    QSet<quint64> " << varName(QStringLiteral("settingsChanged"), cfg()) << ";\n";
    }

    stream() << "};\n\n";
    endNamespaces();
}

void KConfigSourceGenerator::createSingletonImplementation()
{
    // Singleton implementation
    if (!cfg().singleton) {
        return;
    }

    beginNamespaces();
    stream() << "class " << cfg().className << "Helper\n";
    stream() << '{' << '\n';
    stream() << "  public:\n";
    stream() << "    " << cfg().className << "Helper() : q(nullptr) {}\n";
    stream() << "    ~" << cfg().className << "Helper() { delete q; q = nullptr; }\n";
    stream() << "    " << cfg().className << "Helper(const " << cfg().className << "Helper&) = delete;\n";
    stream() << "    " << cfg().className << "Helper& operator=(const " << cfg().className << "Helper&) = delete;\n";
    stream() << "    " << cfg().className << " *q;\n";
    stream() << "};\n";
    endNamespaces();

    stream() << "Q_GLOBAL_STATIC(" << cfg().className << "Helper, s_global" << cfg().className << ")\n";

    stream() << cfg().className << " *" << cfg().className << "::self()\n";
    stream() << "{\n";
    if (parseResult.cfgFileNameArg) {
        stream() << "  if (!s_global" << cfg().className << "()->q)\n";
        stream() << "     qFatal(\"you need to call " << cfg().className << "::instance before using\");\n";
    } else {
        stream() << "  if (!s_global" << cfg().className << "()->q) {\n";
        stream() << "    new " << cfg().className << ';' << '\n';
        stream() << "    s_global" << cfg().className << "()->q->read();\n";
        stream() << "  }\n\n";
    }
    stream() << "  return s_global" << cfg().className << "()->q;\n";
    stream() << "}\n\n";

    if (cfg().qmlRegistration && cfg().singleton) {
        stream() << cfg().className << " *" << cfg().className << "::create(QQmlEngine *, QJSEngine *)\n";
        stream() << "{\n";
        stream() << "  QQmlEngine::setObjectOwnership(self(), QQmlEngine::CppOwnership);\n";
        stream() << "  return self();\n";
        stream() << "}\n\n";
    }

    if (parseResult.cfgFileNameArg) {
        auto instance = [this](const QString &type, const QString &arg, bool isString) {
            stream() << "void " << cfg().className << "::instance(" << type << " " << arg << ")\n";
            stream() << "{\n";
            stream() << "  if (s_global" << cfg().className << "()->q) {\n";
            stream() << "     qDebug() << \"" << cfg().className << "::instance called after the first use - ignoring\";\n";
            stream() << "     return;\n";
            stream() << "  }\n";
            stream() << "  new " << cfg().className << "(";
            if (parseResult.cfgStateConfig) {
                stream() << "KSharedConfig::openStateConfig(" << arg << ")";
            } else if (isString) {
                stream() << "KSharedConfig::openConfig(" << arg << ")";
            } else {
                stream() << "std::move(" << arg << ")";
            }
            stream() << ");\n";
            stream() << "  s_global" << cfg().className << "()->q->read();\n";
            stream() << "}\n\n";
        };
        instance(QStringLiteral("const QString&"), QStringLiteral("cfgfilename"), true);
        instance(QStringLiteral("KSharedConfig::Ptr"), QStringLiteral("config"), false);
    }
}

void KConfigSourceGenerator::createPreamble()
{
    QString cppPreamble;
    for (const auto *entry : std::as_const(parseResult.entries)) {
        if (entry->paramValues.isEmpty()) {
            continue;
        }

        cppPreamble += QStringLiteral("const char* const ") + cfg().className + QStringLiteral("::") + enumName(entry->param);
        cppPreamble += cfg().globalEnums
            ? QStringLiteral("ToString[] = { \"") + entry->paramValues.join(QStringLiteral("\", \"")) + QStringLiteral("\" };\n")
            : QStringLiteral("::enumToString[] = { \"") + entry->paramValues.join(QStringLiteral("\", \"")) + QStringLiteral("\" };\n");
    }

    if (!cppPreamble.isEmpty()) {
        stream() << cppPreamble << '\n';
    }
}

void KConfigSourceGenerator::createConstructorParameterList()
{
    if (parseResult.cfgFileNameArg) {
        if (!cfg().forceStringFilename) {
            stream() << " KSharedConfig::Ptr config";
        } else {
            stream() << " const QString& config";
        }
        stream() << (parseResult.parameters.isEmpty() ? "" : ",");
    }

    for (auto it = parseResult.parameters.cbegin(); it != parseResult.parameters.cend(); ++it) {
        if (it != parseResult.parameters.cbegin()) {
            stream() << ",";
        }
        stream() << " " << param((*it).type) << " " << (*it).name;
    }

    if (cfg().parentInConstructor) {
        if (parseResult.cfgFileNameArg || !parseResult.parameters.isEmpty()) {
            stream() << ",";
        }
        stream() << " QObject *parent";
    }
}

void KConfigSourceGenerator::createParentConstructorCall()
{
    stream() << cfg().inherits << "(";
    if (parseResult.cfgStateConfig) {
        stream() << " KSharedConfig::openStateConfig(QStringLiteral( \"" << parseResult.cfgFileName << "\") ";
    } else if (!parseResult.cfgFileName.isEmpty()) {
        stream() << " QStringLiteral( \"" << parseResult.cfgFileName << "\" ";
    }
    if (parseResult.cfgFileNameArg) {
        if (!cfg().forceStringFilename) {
            stream() << " std::move( config ) ";
        } else {
            stream() << " config ";
        }
    }
    if (!parseResult.cfgFileName.isEmpty()) {
        stream() << ") ";
    }
    stream() << ")\n";
}

void KConfigSourceGenerator::createInitializerList()
{
    for (const auto &parameter : std::as_const(parseResult.parameters)) {
        stream() << "  , mParam" << parameter.name << "(" << parameter.name << ")\n";
    }
}

void KConfigSourceGenerator::createEnums(const CfgEntry *entry)
{
    if (entry->type != QLatin1String("Enum")) {
        return;
    }
    stream() << "  QList<" << cfg().inherits << "::ItemEnum::Choice> values" << entry->name << ";\n";

    for (const auto &choice : std::as_const(entry->choices.choices)) {
        stream() << "  {\n";
        stream() << "    " << cfg().inherits << "::ItemEnum::Choice choice;\n";
        stream() << "    choice.name = QStringLiteral(\"" << choice.name << "\");\n";
        if (cfg().setUserTexts) {
            if (!choice.label.isEmpty()) {
                stream() << "    choice.label = " << translatedString(cfg(), choice.label, choice.context) << ";\n";
            }
            if (!choice.toolTip.isEmpty()) {
                stream() << "    choice.toolTip = " << translatedString(cfg(), choice.toolTip, choice.context) << ";\n";
            }
            if (!choice.whatsThis.isEmpty()) {
                stream() << "    choice.whatsThis = " << translatedString(cfg(), choice.whatsThis, choice.context) << ";\n";
            }
        }
        stream() << "    values" << entry->name << ".append( choice );\n";
        stream() << "  }\n";
    }
}

void KConfigSourceGenerator::createNormalEntry(const CfgEntry *entry, const QString &key)
{
    const QString itemVarStr = itemPath(entry, cfg());
    const QString innerItemVarStr = innerItemVar(entry, cfg());
    if (!entry->signalList.isEmpty()) {
        stream() << "  " << innerItemVarStr << " = " << newInnerItem(entry, key, entry->defaultValue, cfg()) << '\n';
    }

    stream() << "  " << itemVarStr << " = " << newItem(entry, key, entry->defaultValue, cfg()) << '\n';

    if (!entry->min.isEmpty()) {
        stream() << "  " << innerItemVarStr << "->setMinValue(" << entry->min << ");\n";
    }

    if (!entry->max.isEmpty()) {
        stream() << "  " << innerItemVarStr << "->setMaxValue(" << entry->max << ");\n";
    }

    if (cfg().setUserTexts) {
        stream() << userTextsFunctions(entry, cfg());
    }

    if (cfg().allNotifiers || cfg().notifiers.contains(entry->name)) {
        stream() << "  " << itemVarStr << "->setWriteFlags(KConfigBase::Notify);\n";
    }

    for (const CfgEntry::Choice &choice : std::as_const(entry->choices.choices)) {
        if (!choice.val.isEmpty()) {
            stream() << "  " << innerItemVarStr << "->setValueForChoice(QStringLiteral( \"" << choice.name << "\" ), QStringLiteral( \"" << choice.val
                     << "\" ));\n";
        }
    }

    if (!entry->parentGroup.isEmpty()) {
        stream() << "  " << itemVarStr << "->setGroup(cg" << QString(entry->group).remove(QRegularExpression(QStringLiteral("\\W"))) << ");\n";
    }

    stream() << "  addItem( " << itemVarStr;
    QString quotedName = entry->name;
    addQuotes(quotedName);
    if (quotedName != key) {
        stream() << ", QStringLiteral( \"" << entry->name << "\" )";
    }
    stream() << " );\n";
}

// TODO : Some compiler option won't work or generate bogus settings file.
// * Does not manage properly Notifiers=true kcfgc option for parameterized entries :
// ** KConfigCompilerSignallingItem generated with wrong userData parameter (4th one).
// ** setWriteFlags() is missing.
// * Q_PROPERTY signal won't work
void KConfigSourceGenerator::createIndexedEntry(const CfgEntry *entry, const QString &key)
{
    for (int i = 0; i <= entry->paramMax; i++) {
        const QString argBracket = QStringLiteral("[%1]").arg(i);
        const QString innerItemVarStr = innerItemVar(entry, cfg()) + argBracket;

        const QString defaultStr = !entry->paramDefaultValues[i].isEmpty() ? entry->paramDefaultValues[i]
            : !entry->defaultValue.isEmpty()                               ? paramString(entry->defaultValue, entry, i)
                                                                           : defaultValue(entry->type);

        if (!entry->signalList.isEmpty()) {
            stream() << "  " << innerItemVarStr << " = " << newInnerItem(entry, paramString(key, entry, i), defaultStr, cfg(), argBracket) << '\n';
        }

        const QString itemVarStr = itemPath(entry, cfg()) + argBracket;

        stream() << "  " << itemVarStr << " = " << newItem(entry, paramString(key, entry, i), defaultStr, cfg(), argBracket) << '\n';

        if (!entry->min.isEmpty()) {
            stream() << "  " << innerItemVarStr << "->setMinValue(" << entry->min << ");\n";
        }
        if (!entry->max.isEmpty()) {
            stream() << "  " << innerItemVarStr << "->setMaxValue(" << entry->max << ");\n";
        }

        for (const CfgEntry::Choice &choice : std::as_const(entry->choices.choices)) {
            if (!choice.val.isEmpty()) {
                stream() << "  " << innerItemVarStr << "->setValueForChoice(QStringLiteral( \"" << choice.name << "\" ), QStringLiteral( \"" << choice.val
                         << "\" ));\n";
            }
        }

        if (cfg().setUserTexts) {
            stream() << userTextsFunctions(entry, cfg(), itemVarStr, entry->paramName);
        }

        // Make mutators for enum parameters work by adding them with $(..) replaced by the
        // param name. The check for isImmutable in the set* functions doesn't have the param
        // name available, just the corresponding enum value (int), so we need to store the
        // param names in a separate static list!.
        const bool isEnum = entry->paramType == QLatin1String("Enum");
        const QString arg = isEnum ? entry->paramValues[i] : QString::number(i);

        QString paramName = entry->paramName;

        stream() << "  addItem( " << itemVarStr << ", QStringLiteral( \"";
        stream() << paramName.replace(QStringLiteral("$(") + entry->param + QLatin1Char(')'), QLatin1String("%1")).arg(arg);
        stream() << "\" ) );\n";
    }
}

void KConfigSourceGenerator::handleCurrentGroupChange(const CfgEntry *entry)
{
    if (entry->group == mCurrentGroup) {
        return;
    }

    // HACK: This fixes one spacing line in the diff. Remove this in the future and adapt the testcases.
    static bool first = true;
    if (!entry->group.isEmpty()) {
        if (!first) {
            stream() << '\n';
        }
        first = false;
    }

    mCurrentGroup = entry->group;

    if (!entry->parentGroup.isEmpty()) {
        QString parentGroup = QString(entry->parentGroup).remove(QRegularExpression(QStringLiteral("\\W")));
        if (!mConfigGroupList.contains(parentGroup)) {
            stream() << "  KConfigGroup cg" << parentGroup << "(this->config(), " << paramString(entry->parentGroup, parseResult.parameters) << ");\n";
            mConfigGroupList << parentGroup;
        }
        QString currentGroup = QString(mCurrentGroup).remove(QRegularExpression(QStringLiteral("\\W")));
        if (!mConfigGroupList.contains(currentGroup)) {
            stream() << "  KConfigGroup cg" << currentGroup << " = cg" << QString(entry->parentGroup).remove(QRegularExpression(QStringLiteral("\\W")))
                     << ".group(" << paramString(mCurrentGroup, parseResult.parameters) << ");\n";
            mConfigGroupList << currentGroup;
        }
    } else {
        stream() << "  setCurrentGroup( " << paramString(mCurrentGroup, parseResult.parameters) << " );";
        stream() << "\n\n";
    }
}

void KConfigSourceGenerator::doConstructor()
{
    // Constructor
    stream() << cfg().className << "::" << cfg().className << "(";
    createConstructorParameterList();
    stream() << " )\n";
    stream() << "  : ";
    createParentConstructorCall();
    createInitializerList();

    stream() << "{\n";

    if (cfg().parentInConstructor) {
        stream() << "  setParent(parent);\n";
    }

    if (cfg().dpointer) {
        stream() << "  d = new " << cfg().className << "Private;\n";
    }

    // Needed in case the singleton class is used as baseclass for
    // another singleton.
    if (cfg().singleton) {
        stream() << "  Q_ASSERT(!s_global" << cfg().className << "()->q);\n";
        stream() << "  s_global" << cfg().className << "()->q = this;\n";
    }

    if (!parseResult.signalList.isEmpty()) {
        // this cast to base-class pointer-to-member is valid C++
        // https://stackoverflow.com/questions/4272909/is-it-safe-to-upcast-a-method-pointer-and-use-it-with-base-class-pointer/
        stream() << "  KConfigCompilerSignallingItem::NotifyFunction notifyFunction ="
                 << " static_cast<KConfigCompilerSignallingItem::NotifyFunction>(&" << cfg().className << "::itemChanged);\n";

        stream() << '\n';
    }

    for (const auto *entry : std::as_const(parseResult.entries)) {
        handleCurrentGroupChange(entry);

        const QString key = paramString(entry->key, parseResult.parameters);
        if (!entry->code.isEmpty()) {
            stream() << entry->code << '\n';
        }
        createEnums(entry);

        stream() << itemDeclaration(entry, cfg());

        if (entry->param.isEmpty()) {
            createNormalEntry(entry, key);
        } else {
            createIndexedEntry(entry, key);
        }
    }

    stream() << "}\n\n";
}

void KConfigSourceGenerator::createGetterDPointerMode(const CfgEntry *entry)
{
    // Accessor
    if (cfg().useEnumTypes && entry->type == QLatin1String("Enum")) {
        stream() << enumType(entry, cfg().globalEnums);
    } else {
        stream() << cppType(entry->type);
    }

    stream() << " " << getFunction(entry->name, cfg().className) << "(";
    if (!entry->param.isEmpty()) {
        stream() << " " << cppType(entry->paramType) << " i ";
    }
    stream() << ")" << Const() << '\n';

    // function body inline only if not using dpointer
    // for BC mode
    startScope();
    // HACK: Fix memberAccessorBody
    stream() << "  " << memberAccessorBody(entry, cfg().globalEnums);
    endScope();
    stream() << '\n';
}

void KConfigSourceGenerator::createImmutableGetterDPointerMode(const CfgEntry *entry)
{
    stream() << whitespace() << "";
    stream() << "bool "
             << " " << immutableFunction(entry->name, cfg().className) << "(";
    if (!entry->param.isEmpty()) {
        stream() << " " << cppType(entry->paramType) << " i ";
    }
    stream() << ")" << Const() << '\n';
    startScope();
    memberImmutableBody(entry, cfg().globalEnums);
    endScope();
    stream() << '\n';
}

void KConfigSourceGenerator::createSetterDPointerMode(const CfgEntry *entry)
{
    // Manipulator
    if (!(cfg().allMutators || cfg().mutators.contains(entry->name))) {
        return;
    }

    stream() << "void " << setFunction(entry->name, cfg().className) << "( ";
    if (!entry->param.isEmpty()) {
        stream() << cppType(entry->paramType) << " i, ";
    }

    if (cfg().useEnumTypes && entry->type == QLatin1String("Enum")) {
        stream() << enumType(entry, cfg().globalEnums);
    } else {
        stream() << param(entry->type);
    }
    stream() << " v )\n";

    // function body inline only if not using dpointer
    // for BC mode
    startScope();
    memberMutatorBody(entry);
    endScope();
    stream() << '\n';
}

void KConfigSourceGenerator::createItemGetterDPointerMode(const CfgEntry *entry)
{
    // Item accessor
    if (!cfg().itemAccessors) {
        return;
    }
    stream() << '\n';
    stream() << cfg().inherits << "::Item" << itemType(entry->type) << " *" << getFunction(entry->name, cfg().className) << "Item(";
    if (!entry->param.isEmpty()) {
        stream() << " " << cppType(entry->paramType) << " i ";
    }
    stream() << ")\n";
    startScope();
    stream() << "  " << itemAccessorBody(entry, cfg());
    endScope();
}

void KConfigSourceGenerator::doGetterSetterDPointerMode()
{
    if (!cfg().dpointer) {
        return;
    }

    // setters and getters go in Cpp if in dpointer mode
    for (const auto *entry : std::as_const(parseResult.entries)) {
        createSetterDPointerMode(entry);
        createGetterDPointerMode(entry);
        createImmutableGetterDPointerMode(entry);
        createItemGetterDPointerMode(entry);
        stream() << '\n';
    }
}

void KConfigSourceGenerator::createDefaultValueGetterSetter()
{
    // default value getters always go in Cpp
    for (const auto *entry : std::as_const(parseResult.entries)) {
        QString n = entry->name;
        QString t = entry->type;

        // Default value Accessor, as "helper" function
        if ((cfg().allDefaultGetters || cfg().defaultGetters.contains(n)) && !entry->defaultValue.isEmpty()) {
            stream() << cppType(t) << " " << getDefaultFunction(n, cfg().className) << "_helper(";
            if (!entry->param.isEmpty()) {
                stream() << " " << cppType(entry->paramType) << " i ";
            }
            stream() << ")" << Const() << '\n';
            startScope();
            stream() << memberGetDefaultBody(entry) << '\n';
            endScope();
            stream() << '\n';
        }
    }
}

void KConfigSourceGenerator::createDestructor()
{
    stream() << cfg().className << "::~" << cfg().className << "()\n";
    startScope();
    if (cfg().dpointer) {
        stream() << "  delete d;\n";
    }
    if (cfg().singleton) {
        const QString qgs = QLatin1String("s_global") + cfg().className;
        stream() << "  if (" << qgs << ".exists() && !" << qgs << ".isDestroyed()) {\n";
        stream() << "    " << qgs << "()->q = nullptr;\n";
        stream() << "  }\n";
    }
    endScope();
    stream() << '\n';
}

void KConfigSourceGenerator::createNonModifyingSignalsHelper()
{
    if (!parseResult.hasNonModifySignals) {
        return;
    }
    stream() << "bool " << cfg().className << "::"
             << "usrSave()\n";
    startScope();
    stream() << "  const bool res = " << cfg().inherits << "::usrSave();\n";
    stream() << "  if (!res) return false;\n\n";
    for (const Signal &signal : std::as_const(parseResult.signalList)) {
        if (signal.modify) {
            continue;
        }

        stream() << "  if (" << varPath(QStringLiteral("settingsChanged"), cfg()) << ".contains(" << signalEnumName(signal.name) << "))\n";
        stream() << "    Q_EMIT " << signal.name << "(";
        auto it = signal.arguments.cbegin();
        const auto itEnd = signal.arguments.cend();
        while (it != itEnd) {
            Param argument = *it;
            bool cast = false;
            if (cfg().useEnumTypes && argument.type == QLatin1String("Enum")) {
                for (int i = 0, end = parseResult.entries.count(); i < end; ++i) {
                    if (parseResult.entries.at(i)->name == argument.name) {
                        stream() << "static_cast<" << enumType(parseResult.entries.at(i), cfg().globalEnums) << ">(";
                        cast = true;
                        break;
                    }
                }
            }
            stream() << varPath(argument.name, cfg());
            if (cast) {
                stream() << ")";
            }
            if (++it != itEnd) {
                stream() << ", ";
            }
        }

        stream() << ");\n";
    }

    stream() << "  " << varPath(QStringLiteral("settingsChanged"), cfg()) << ".clear();\n";
    stream() << "  return true;\n";
    endScope();
}

void KConfigSourceGenerator::createSignalFlagsHandler()
{
    if (parseResult.signalList.isEmpty()) {
        return;
    }

    stream() << '\n';
    stream() << "void " << cfg().className << "::"
             << "itemChanged(quint64 signalFlag) {\n";
    if (parseResult.hasNonModifySignals) {
        stream() << "  " << varPath(QStringLiteral("settingsChanged"), cfg()) << ".insert(signalFlag);\n";
    }

    if (!parseResult.signalList.isEmpty()) {
        stream() << '\n';
    }

    bool modifySignalsWritten = false;
    for (const Signal &signal : std::as_const(parseResult.signalList)) {
        if (signal.modify) {
            if (!modifySignalsWritten) {
                stream() << "  switch (signalFlag) {\n";
                modifySignalsWritten = true;
            }
            stream() << "  case " << signalEnumName(signal.name) << ":\n";
            stream() << "    Q_EMIT " << signal.name << "();\n";
            stream() << "    break;\n";
        }
    }
    if (modifySignalsWritten) {
        stream() << "  }\n";
    }

    stream() << "}\n";
}
