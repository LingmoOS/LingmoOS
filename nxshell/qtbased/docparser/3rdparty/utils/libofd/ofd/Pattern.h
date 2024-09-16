#ifndef __OFD_PATTERN_H__
#define __OFD_PATTERN_H__

#include "ofd/Common.h"

namespace ofd {

enum class PatternReflectMethod {
    Normal = 0,
    Column,
    Row,
    RowAndColumn,
};

class Pattern
{
public:
    Pattern();
    virtual ~Pattern();

    // =============== Public Attributes ================
public:
    double Width;
    double Height;
    double XStep;
    double YStep;
    std::string ReflectMethod;
    std::string ReleativeTo;
    double CTM[6];
    PageBlockPtr CellContent;
    uint64_t Thumbnail;

    // =============== Public Methods ================
public:
    virtual void WritePatternXML(utils::XMLWriter &writer) const {};

};   // class Pattern

};   // namespace ofd

#endif   // __OFD_PATTERN_H__
