#ifndef __OFD_TEXTOBJECT_H__
#define __OFD_TEXTOBJECT_H__

#include "ofd/Object.h"

namespace ofd {

namespace Text {

// ======== class Text::TextCode ========
// 标准 P64 页，Page.xsd。
class TextCode
{
public:
    double X;
    double Y;
    DoubleArray DeltaX;
    DoubleArray DeltaY;
    std::string Text;

    TextCode()
        : X(0.0), Y(0.0) {};
};   // class TextCode
typedef std::vector<TextCode> TextCodeArray;

// 阅读方向，可选值为0,90,180,270，默认值为0。
enum class ReadDirection {
    ANGLE0,
    ANGLE90,
    ANGLE180,
    ANGLE270,
};

// 字符方向，指定了文字放置的方式，可选值为：0, 90, 180, 270，默认值为0。
enum class CharDirection {
    ANGLE0,
    ANGLE90,
    ANGLE180,
    ANGLE270,
};

// 文字对象的粗细值，可选取值为100, 200, 300, 400, 500, 600, 700, 800, 900。
enum class Weight {
    WEIGHT100,
    WEIGHT200,
    WEIGHT300,
    WEIGHT400,
    WEIGHT500,
    WEIGHT600,
    WEIGHT700,
    WEIGHT800,
    WEIGHT900,
};

};   // namespace Text

class TextObject : public Object
{
public:
    static ColorPtr DefaultStrokeColor;
    static ColorPtr DefaultFillColor;

public:
    TextObject(LayerPtr layer);
    virtual ~TextObject();

    // =============== Public Attributes ================
public:
    double FontSize;   // 字号，单位为毫米。
    bool Stroke;   // 是否勾边，默认值为false。
    bool Fill;   // 是否填充，默认值为true。
    double HScale;   // 字型在水平方向的缩放比，默认值为1.0。
    // 例如：当HScale值为0.5时，表示实际显示的字宽为原来字宽的一半。
    Text::ReadDirection RD;   // 阅读方向，可选值为0,90,180,270，默认值为0。
    Text::CharDirection CD;   // 字符方向，指定了文字放置的方式，
    // 可选值为：0, 90, 180, 270，默认值为0。
    Text::Weight Weight;   // 文字对象的粗细值，可选取值为100, 200, 300, 400, 500,
    // 600, 700, 800, 900。 默认值为400。
    bool Italic;   // 是否是斜体样式，默认值为false。

private:
    ColorPtr FillColor;   // 填充色，默认值为黑色。
    ColorPtr StrokeColor;   // 勾边色，默认值为透明色。

    // =============== Public Methods ================
public:
    virtual std::string to_string() const override;

    // 字号，单位为毫米。
    double GetFontSize() const { return FontSize; };
    void SetFontSize(double fontSize) { FontSize = fontSize; };

    // 是否勾边，默认值为false。
    bool IsStroke() const { return Stroke; };
    void EnableStroke(bool bStroke) { Stroke = bStroke; };

    // 是否填充，默认值为true。
    bool IsFill() const { return Fill; };
    void EnableFill(bool bFill) { Fill = bFill; };

    // 字型在水平方向的缩放比，默认值为1.0。
    // 例如：当HScale值为0.5时，表示实际显示的字宽为原来字宽的一半。
    bool GetHScale() const { return HScale; };
    void SetHScale(double dHScale) { HScale = dHScale; };

    // 阅读方向，可选值为0,90,180,270，默认值为0。
    Text::ReadDirection GetReadDirection() const { return RD; };
    void SetReadDirection(Text::ReadDirection readDirection) { RD = readDirection; };

    // 字符方向，指定了文字放置的方式，可选值为：0, 90, 180, 270，默认值为0。
    Text::CharDirection GetCharDirection() const { return CD; };
    void SetCharDirection(Text::CharDirection charDirection) { CD = charDirection; };

    // 文字对象的粗细值，可选取值为100, 200, 300, 400, 500, 600, 700, 800, 900。
    // 默认值为400。
    Text::Weight GetWeight() const { return Weight; };
    void SetWeight(Text::Weight weight) { Weight = weight; };

    // 是否是斜体样式，默认值为false。
    bool IsItalic() const { return Italic; };
    void SetItalic(bool bItalic) { Italic = bItalic; };

    // 填充色，默认值为黑色。
    ColorPtr GetFillColor() const { return FillColor; };
    void SetFillColor(ColorPtr fillColor);

    // 勾边色，默认值为透明色。
    ColorPtr GetStrokeColor() const { return StrokeColor; };
    void SetStrokeColor(ColorPtr strokeColor);

    // TODO
    //CT_CGTransform CGTransform; // 指定字符编码到字符索引之间的变换关系。
protected:
    virtual void GenerateAttributesXML(utils::XMLWriter &writer) const override;
    virtual void GenerateElementsXML(utils::XMLWriter &writer) const override;

    virtual bool FromAttributesXML(utils::XMLElementPtr objectElement) override;
    virtual bool IterateElementsXML(utils::XMLElementPtr childElement) override;
    virtual void RecalculateBoundary() override;

    // ---------------- Private Attributes ----------------
public:
    size_t GetNumTextCodes() const { return m_textCodes.size(); };
    const Text::TextCode &GetTextCode(size_t idx) const { return m_textCodes[idx]; };
    Text::TextCode &GetTextCode(size_t idx) { return m_textCodes[idx]; };

    void AddTextCode(const Text::TextCode &textCode) { m_textCodes.push_back(textCode); };
    void ClearTextCodes() { m_textCodes.clear(); };

private:
    Text::TextCodeArray m_textCodes;

};   // class TextObject
};   // namespace ofd

#endif   // __OFD_TEXTOBJECT_H__
