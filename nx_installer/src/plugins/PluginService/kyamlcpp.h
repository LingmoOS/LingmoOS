#ifndef KYAMLCPP_H
#define KYAMLCPP_H

#include <QVariantMap>
#include <QFileInfo>
#include <yaml-cpp/yaml.h>


namespace KServer {

    QVariantMap loadYaml( const QString& filename, bool* ok = nullptr );

    QVariantMap loadYaml( const QFileInfo&, bool* ok = nullptr );

    QVariant yamlToVariant( const YAML::Node& node );
    QVariant yamlScalarToVariant( const YAML::Node& scalarNode );
    QVariantList yamlSequenceToVariant( const YAML::Node& sequenceNode );
    QVariantMap yamlMapToVariant( const YAML::Node& mapNode );

    QStringList yamlToStringList( const YAML::Node& listNode );

    bool saveYaml( const QString& filename, const QVariantMap& map );


    void explainYamlException( const YAML::Exception& e, const QByteArray& data, const char* label );
    void explainYamlException( const YAML::Exception& e, const QByteArray& data, const QString& label );
    void explainYamlException( const YAML::Exception& e, const QByteArray& data );\

}

#endif // KYAMLCPP_H
