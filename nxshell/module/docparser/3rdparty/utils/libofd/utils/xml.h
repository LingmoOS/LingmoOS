#ifndef __UTILS_XML_H__
#define __UTILS_XML_H__

#include <memory>
#include <string>
#include <tuple>
#include "utils/utils.h"

#define OFDXML_HEAD_ATTRIBUTES \
    writer.WriteAttribute("xmlns:ofd", std::string("http://www.ofdspec.org/2016")); 

// Defined in libxml/tree.h
struct _xmlNode;

namespace utils{

class XMLWriter{
public:
    XMLWriter(bool bHead=false);
    ~XMLWriter();

    std::string GetString() const;

    void StartDocument(const std::string &encoding = "utf-8");
    void StartElement(const std::string &name);
    void EndElement();
    void WriteElement(const std::string &name, const std::string &value);
    void WriteElement(const std::string &name, uint64_t value);
    void WriteElement(const std::string &name, double value, int precision=3);
    void WriteElement(const std::string &name, bool value);
    void WriteAttribute(const std::string &name, const std::string &value);
    void WriteAttribute(const std::string &name, uint64_t value);
    void WriteAttribute(const std::string &name, double value, int precision=3);
    void WriteAttribute(const std::string &name, bool value);
    void WriteRaw(const std::string &text);
    void WriteString(const std::string &text);
    void EndDocument();

private:
    class ImplCls;
    std::unique_ptr<ImplCls> m_impl;

}; // class XMLWriter

//class XMLReader{
//public:
    //XMLReader();
    //~XMLReader();

    //bool ParseXML(const std::string &xmlString);
    //XMLElementPtr GetRootElement(const std::string &xmlString);

    //bool HasElement() const;
    //void NextElement();
    //bool EnterChildElement(const std::string &name);
    //void BackParentElement();
    //bool CheckElement(const std::string &name);

    //std::string GetElementName() const;

    //bool ReadElement(std::string &value);
    //bool ReadElement(uint64_t &value);
    //bool ReadElement(double &value);
    //bool ReadElement(bool &value);
    //bool ReadAttribute(const std::string &name, std::string &value);
    //bool ReadAttribute(const std::string &name, uint64_t &value);
    //bool ReadAttribute(const std::string &name, double &value);
    //bool ReadAttribute(const std::string &name, bool &value);

//private:
    //class ImplCls;
    //std::unique_ptr<ImplCls> m_impl;

//}; // class XMLReader


class XMLElement{
public:
    XMLElement(_xmlNode *node);
    ~XMLElement();

    static XMLElementPtr ParseRootElement(const std::string &xmlString);

    XMLElementPtr GetFirstChildElement();
    XMLElementPtr GetNextSiblingElement();

    std::string GetName() const;

    std::tuple<std::string, bool> GetStringValue() const;
    std::tuple<uint64_t, bool> GetIntValue() const;
    std::tuple<double, bool> GetFloatValue() const;
    std::tuple<bool, bool> GetBooleanValue() const;

    std::tuple<std::string, bool> GetStringAttribute(const std::string &name) const;
    std::tuple<uint64_t, bool> GetIntAttribute(const std::string &name) const;
    std::tuple<double, bool> GetFloatAttribute(const std::string &name) const;
    std::tuple<bool, bool> GetBooleanAttribute(const std::string &name) const;

private:
    _xmlNode *m_node;

}; // class XMLElement

}

#endif // __UTILS_XML_H__
