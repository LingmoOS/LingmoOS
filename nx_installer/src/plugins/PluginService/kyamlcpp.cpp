#include "kyamlcpp.h"
#include <QFile>

#include <QByteArray>
#include <yaml-cpp/yaml.h>
#include <QtWidgets/QMessageBox>
#include <QDebug>
#include <QFile>

#include "klogger.h"

void operator>>( const YAML::Node& node, QStringList& v )
{
    for ( size_t i = 0; i < node.size(); ++i )
    {
        v.append( QString::fromStdString( node[ i ].as< std::string >() ) );
    }
}

namespace KServer {

const QRegExp _yamlScalarTrueValues = QRegExp( "true|True|TRUE|on|On|ON" );
const QRegExp _yamlScalarFalseValues = QRegExp( "false|False|FALSE|off|Off|OFF" );

QVariant yamlToVariant( const YAML::Node& node )
{
    switch ( node.Type() )
    {
    case YAML::NodeType::Scalar:
        return yamlScalarToVariant( node );

    case YAML::NodeType::Sequence:
        return yamlSequenceToVariant( node );

    case YAML::NodeType::Map:
        return yamlMapToVariant( node );

    case YAML::NodeType::Null:
    case YAML::NodeType::Undefined:
        return QVariant();
    }

    // NOTREACHED
    return QVariant();
}

QVariant yamlScalarToVariant( const YAML::Node& scalarNode )
{
    std::string stdScalar = scalarNode.as< std::string >();
    QString scalarString = QString::fromStdString( stdScalar );
    if ( _yamlScalarTrueValues.exactMatch( scalarString ) )
    {
        return QVariant( true );
    }
    if ( _yamlScalarFalseValues.exactMatch( scalarString ) )
    {
        return QVariant( false );
    }
    if ( QRegExp( "[-+]?\\d+" ).exactMatch( scalarString ) )
    {
        return QVariant( scalarString.toLongLong() );
    }
    if ( QRegExp( "[-+]?\\d*\\.?\\d+" ).exactMatch( scalarString ) )
    {
        return QVariant( scalarString.toDouble() );
    }
    return QVariant( scalarString );
}

QVariantList yamlSequenceToVariant( const YAML::Node& sequenceNode )
{
    QVariantList vl;
    for ( YAML::const_iterator it = sequenceNode.begin(); it != sequenceNode.end(); ++it )
    {
        vl << yamlToVariant( *it );
    }
    return (QVariantList)vl;
}


QVariantMap yamlMapToVariant( const YAML::Node& mapNode )
{
    QVariantMap vm;
    for ( YAML::const_iterator it = mapNode.begin(); it != mapNode.end(); ++it )
    {
        vm.insert( QString::fromStdString( it->first.as< std::string >() ), yamlToVariant( it->second ) );
    }
    return (QVariantMap)vm;
}

QStringList yamlToStringList( const YAML::Node& listNode )
{
    QStringList l;
    listNode >> l;
    return (QStringList)l;
}


void explainYamlException( const YAML::Exception& e, const QByteArray& yamlData, const char* label )
{
    qWarning() << "YAML error " << e.what() << "in" << label << '.';
    explainYamlException( e, yamlData );
}

void explainYamlException( const YAML::Exception& e, const QByteArray& yamlData, const QString& label )
{
    qWarning() << "YAML error " << e.what() << "in" << label << '.';
    explainYamlException( e, yamlData );
}

void explainYamlException( const YAML::Exception& e, const QByteArray& yamlData )
{
    if ( ( e.mark.line >= 0 ) && ( e.mark.column >= 0 ) )
    {
        // Try to show the line where it happened.
        int linestart = 0;
        for ( int linecount = 0; linecount < e.mark.line; ++linecount )
        {
            linestart = yamlData.indexOf( '\n', linestart );
            // No more \ns found, weird
            if ( linestart < 0 )
            {
                break;
            }
            linestart += 1;  // Skip that \n
        }
        int lineend = linestart;
        if ( linestart >= 0 )
        {
            lineend = yamlData.indexOf( '\n', linestart );
            if ( lineend < 0 )
            {
                lineend = yamlData.length();
            }
        }

        int rangestart = linestart;
        int rangeend = lineend;
        // Adjust range (linestart..lineend) so it's not too long
        if ( ( linestart >= 0 ) && ( e.mark.column > 30 ) )
        {
            rangestart += ( e.mark.column - 30 );
        }
        if ( ( linestart >= 0 ) && ( rangeend - rangestart > 40 ) )
        {
            rangeend = rangestart + 40;
        }

        if ( linestart >= 0 )
        {
            qWarning() << "offending YAML data:" << yamlData.mid( rangestart, rangeend - rangestart ).constData();
        }
    }
}

QVariantMap loadYaml( const QFileInfo& fi, bool* ok )
{
    return loadYaml( fi.absoluteFilePath(), ok );
}

QVariantMap loadYaml( const QString& filename, bool* ok )
{
    if ( ok )
    {
        *ok = false;
    }

    QFile yamlFile( filename );
    QVariant yamlContents;
    if ( yamlFile.exists() && yamlFile.open( QFile::ReadOnly | QFile::Text ) )
    {
        QByteArray ba = yamlFile.readAll();
        try
        {
            YAML::Node doc = YAML::Load( ba.constData() );
            yamlContents = KServer::yamlToVariant( doc );
        }
        catch ( YAML::Exception& e )
        {
            explainYamlException( e, ba, filename );
            return QVariantMap();
        }
    }


    if ( yamlContents.isValid() && !yamlContents.isNull() && yamlContents.type() == QVariant::Map )
    {
        if ( ok )
        {
            *ok = true;
        }
        return yamlContents.toMap();
    }

    return QVariantMap();
}


static bool dumpToYaml(QFile& file, const QVariantMap& map, int tabspace);

static void writeSpace(QFile &file, int tabspace)
{
    while ( tabspace-- > 0) {
        file.write("    ");
    }
}
static void dumpYamlElement(QFile &file, QVariant value, int tabspace)
{
    if(value.type() == QVariant::Type::Bool) {
        //布尔型
        file.write(value.toBool() ? "true" : "false");
    }
    else if(value.type() == QVariant::Type::String) {
        //字符型
        file.write("\"");
        file.write(value.toString().toUtf8());
        file.write("\"");

    }
    else if(value.type() == QVariant::Type::Int) {
        //整形
        file.write(QString::number(value.toInt()).toUtf8());
    }
    else if(value.type() == QVariant::Type::Double) {
        //浮点型
        file.write(QString::number(value.toDouble()).toUtf8());
    }
    else if(value.type() == QVariant::Type::LongLong) {
        //长整形
        file.write(QString::number(value.toLongLong()).toUtf8());
    }
    else if(value.type() == QVariant::Type::ULongLong) {
        //无符号长整形
        file.write(QString::number(value.toULongLong()).toUtf8());
    }
    else if(value.type() == QVariant::Type::List) {
        //列表
        int i = 0;
        for(auto it : value.toList()) {
            ++i;
            file.write("\n");
            writeSpace(file, tabspace);
            file.write("-");
            dumpYamlElement(file, it, tabspace + 1);
        }
        if(!i) {
            file.write("[]");
        }
    }
    else if(value.type() == QVariant::Type::Map) {
        //map型
        file.write("\n");
        dumpToYaml(file, value.toMap(), tabspace + 1);
    }
    else {
        file.write("<");
        file.write(value.typeName());
        file.write(">");
    }

}


static bool dumpToYaml(QFile &file, const QVariantMap& map, int tabspace)
{
    for(auto it = map.cbegin(); it !=map.cend(); ++it) {
    writeSpace(file, tabspace);
    file.write("\"");
    file.write(it.key().toUtf8());
    file.write("\"");
    file.write("\"");

    }
    return true;
}

bool saveInfoToYaml(const QString& fileName, const QVariantMap& map)
{
    QFile file(fileName);
    if(!file.open(QFile::WriteOnly)) {
        return false;
    }
    file.write("-----向YAML文件写入信息");
    file.write(map.values().at(0).toByteArray());
    return true;

}
}
