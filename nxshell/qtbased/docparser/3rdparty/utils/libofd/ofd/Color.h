#ifndef __OFD_COLOR_H__
#define __OFD_COLOR_H__

#include <memory.h>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include "ofd/Common.h"

namespace ofd{

    class ColorSpace;
    typedef std::shared_ptr<ColorSpace> ColorSpacePtr;
    typedef std::vector<ColorSpacePtr> ColorSpaceArray;

    // 颜色空间的类型。
    enum class ColorSpaceType{
        GRAY = 0,
        RGB,
        CMYK,
    };
    __attribute__((unused)) static std::string GetColorSpaceTypeLabel(ColorSpaceType type){
        if ( type == ColorSpaceType::GRAY ){
            return "GRAY";
        } else if ( type == ColorSpaceType::RGB ){
            return "RGB";
        } else if ( type == ColorSpaceType::CMYK ){
            return "CMYK";
        }
        throw "Unknown ofd::ColorSpace::Type: " + std::to_string((int)type);
    }

    // ======== struct ColorSpace ========
    // OFD (section 8.3.1) P31，Res.xsd。

    class Color;
    typedef std::shared_ptr<Color> ColorPtr;
    typedef std::vector<ColorPtr> ColorArray;

    class ColorSpace;
    typedef std::map<uint64_t, ColorSpacePtr> ColorSpaceMap;

    class ColorSpace{
        public:
            static ColorSpacePtr DefaultInstance;
            static ColorSpaceMap GlobalColorSpaces;

            static void GlobalClearColorSpaces();
            static uint64_t GlobalAddColorSpace(ColorSpacePtr colorSpace);
            static ColorSpacePtr GlobalGetColorSpace(uint64_t refID);

        public:
            ColorSpace();

            // =============== Public Attributes ================
        public:
            ColorSpaceType        Type;    // 颜色空间的类型，必选。
            uint32_t              BPC;     // 每个颜色通道使用的位数，有效取值为1,2,4,8,16，默认值为8。
            std::string           Profile; // 指向包内颜色配置文件
            ColorArray            Palette; // 调色板。调色板中颜色的索引编号从0开始。

            // ---------------- Private Attributes ----------------
        public:
            uint64_t GetRefID() const {return m_refID;};
            void SetRefID(uint64_t refID){m_refID = refID;};
        private:
            uint64_t              m_refID; // 资源引用ID。缺省值0，指向文档设定的颜色空间。 

    }; // class ColorSpace


    typedef struct ColorRGB{
        uint32_t Red;   // 红色
        uint32_t Green; // 绿色
        uint32_t Blue;  // 蓝色
        ColorRGB() : Red(0), Green(0), Blue(0){
        }
        ColorRGB(uint32_t r, uint32_t g, uint32_t b):
            Red(r), Green(g), Blue(b){ 
        }

        std::tuple<double, double, double> GetRGB() const{
            double r = (double)Red / 255.0;
            double g = (double)Green / 255.0;
            double b = (double)Blue / 255.0;
            return std::make_tuple(r, g, b);
        }
    } ColorRGB_t;

    typedef struct ColorCMYK{
        uint32_t Cyan;    // 青色
        uint32_t Magenta; // 品红色
        uint32_t Yellow;  // 黄色
        uint32_t blacK;   // 黑色
        ColorCMYK() : Cyan(0), Magenta(0), Yellow(0), blacK(0){
        }
        ColorCMYK(uint32_t c, uint32_t m, uint32_t y, uint32_t k):
            Cyan(c), Magenta(m), Yellow(y), blacK(k){
        }
        std::tuple<double, double, double, double> GetCMYK() const{
            double c = (double)Cyan / 255.0;
            double m = (double)Magenta / 255.0;
            double y = (double)Yellow / 255.0;
            double k = (double)blacK / 255.0;
            return std::make_tuple(c, m, y, k);
        }
    } ColorCMYK_t;

    typedef struct ColorValue{

        union{
            uint32_t  Gray;      // 灰度，只包含一个通道来表明灰度值。
            ColorRGB  RGB;       // 包含三个通道，依次是红、绿、蓝。
            ColorCMYK CMYK;      // 包含四个通道，依次是青、黄、品红、黑。
            uint32_t  Values[4]; // 用于按序号访问颜色通道数值。
        };

        ColorValue(){
        }

        ColorValue(uint32_t gray) {
            Gray = gray;
            Values[1] = 0;
            Values[2] = 0;
            Values[3] = 0;
        }

        ColorValue(uint32_t r, uint32_t g, uint32_t b){
            RGB.Red = r;
            RGB.Green = g;
            RGB.Blue = b;
            Values[3] = 0;
        }

        ColorValue(uint32_t c, uint32_t m, uint32_t y, uint32_t k){
            CMYK.Cyan = c;
            CMYK.Magenta = m;
            CMYK.Yellow = y;
            CMYK.blacK = k;
        }

        bool operator ==(const ColorValue& colorValue) const {
            return Values[0] == colorValue.Values[0] &&
                    Values[1] == colorValue.Values[1] &&
                    Values[2] == colorValue.Values[2] &&
                    Values[3] == colorValue.Values[3];
        }

        bool IsSameColor(const ColorValue& cv, double delta)const{
            double v0 = (double)Values[0] / 255.0;
            double v1 = (double)Values[1] / 255.0;
            double v2 = (double)Values[2] / 255.0;
            double v3 = (double)Values[3] / 255.0;

            double y0 = (double)cv.Values[0] / 255.0;
            double y1 = (double)cv.Values[1] / 255.0;
            double y2 = (double)cv.Values[2] / 255.0;
            double y3 = (double)cv.Values[3] / 255.0;

            if ( fabs(v0 - y0) > delta ) return false;
            if ( fabs(v1 - y1) > delta ) return false;
            if ( fabs(v2 - y2) > delta ) return false;
            if ( fabs(v3 - y3) > delta ) return false;
            return true;
        }

        void AverageColor(const ColorValue& cv){
            Values[0] = (Values[0] + cv.Values[0]) / 2;
            Values[1] = (Values[1] + cv.Values[1]) / 2;
            Values[2] = (Values[2] + cv.Values[2]) / 2;
            Values[3] = (Values[3] + cv.Values[3]) / 2;
        }

    } ColorValue_t;


    // ======== struct Color ========
    // OFD (section 8.3.2) P32, Page.xsd。
    class Color{
        public:
            static ColorPtr Instance(uint32_t gray, ColorSpacePtr colorSpace = ColorSpace::DefaultInstance, uint32_t alpha = 255){
                return std::make_shared<Color>(gray, colorSpace, alpha);
            }
            static ColorPtr Instance(uint32_t r, uint32_t g, uint32_t b, ColorSpacePtr colorSpace = ColorSpace::DefaultInstance, uint32_t alpha = 255){
                return std::make_shared<Color>(r, g, b, colorSpace, alpha);
            }
            static ColorPtr Instance(uint32_t c, uint32_t m, uint32_t y, uint32_t k, ColorSpacePtr colorSpace = ColorSpace::DefaultInstance, uint32_t alpha = 255){
                return std::make_shared<Color>(c, m, y, k, colorSpace, alpha);
            }
            static ColorPtr Instance(const ColorValue &colorValue, ColorSpacePtr colorSpace = ColorSpace::DefaultInstance, uint32_t alpha = 255){
                return std::make_shared<Color>(colorValue, colorSpace, alpha);
            }
            static ColorPtr Instance(ColorSpacePtr colorSpace, uint32_t index, uint32_t alpha = 255){
                return std::make_shared<Color>(colorSpace, index, alpha);
            }

        public:
            Color(uint32_t gray, ColorSpacePtr colorSpace = ColorSpace::DefaultInstance, uint32_t alpha = 255);
            Color(uint32_t r, uint32_t g, uint32_t b, ColorSpacePtr colorSpace = ColorSpace::DefaultInstance, uint32_t alpha = 255);
            Color(uint32_t c, uint32_t m, uint32_t y, uint32_t k, ColorSpacePtr colorSpace = ColorSpace::DefaultInstance, uint32_t alpha = 255);
            Color(const ColorValue &colorValue, ColorSpacePtr colorSpace = ColorSpace::DefaultInstance, uint32_t alpha = 255);
            Color(ColorSpacePtr colorSpace, uint32_t index, uint32_t alpha = 255);
            bool IsSameColor(ColorPtr color, double delta) const {return Value.IsSameColor(color->Value, delta);};
            void AverageColor(ColorPtr color){Value.AverageColor(color->Value);};

            // =============== Public Attributes ================
        public:
            typedef std::weak_ptr<ColorSpace> Parent_t;
            Parent_t         weakColorSpace; // 引用资源文件中颜色空间的标识。默认值为文档设定的颜色空间。
            ColorValue       Value;          // 颜色值，指定了当前颜色空间下各通道的取值。
                                             // Value的取值应符合“通道1 通道2 通道3 ...”格式。
                                             // 此属性不出现时，应采用Index属性从颜色空间的调色板中取值。
                                             // 当二者都不出现时，该颜色各通道的值全部为0。
            uint32_t         Index;          // 调色板中颜色的编号，非负整数，将从当前颜色空间的调色板中
                                             // 取出相应索引的预定义颜色用来绘制。索引从0开始。
            uint32_t         Alpha;          // 颜色透明度，在0-255之间取值。默认值为255，表示完全不透明。

            // TODO 
            // Pattern;      // 底纹 标准 P34 页，Page.xsd。
            // AxialShd;     // 轴向渐变 标准 P36 页，Page.xsd。
            // RadialShd;    // 径向渐变 标准 P40 页，Page.xsd。
            // GouraudShd;   // 高洛德渐变 标准 P47 页，Page.xsd。
            // LaGouraudShd; // 格构高洛德渐变 标准 P48 页，Page.xsd。


            // =============== Public Methods ================
        public:
            void WriteColorXML(utils::XMLWriter &writer) const;
            static std::tuple<ColorPtr, bool> ReadColorXML(utils::XMLElementPtr colorElement);
            bool Equal(ColorPtr color) const;
            std::tuple<double, double, double, double> GetRGBA()const;
            std::tuple<double, double, double, double> GetCMYK()const;

            // ---------------- Private Attributes ----------------
        public:
            bool IsUsePalette() const {return m_bUsePalette;};
            void SetUsePalette(bool bEnable = true){m_bUsePalette = bEnable;};
            ColorSpacePtr GetColorSpace() const;

        private:
            bool                  m_bUsePalette; // 使用颜色空间调色板标志

    }; // class Color
    #define COLOR_BLACK ofd::Color::Instance(0,0,0) 
    #define COLOR_WHITE ofd::Color::Instance(255,255,255) 
    #define COLOR_TRANSPARENT ofd::Color::Instance(0,0,0,nullptr, 0)
    #define COLOR_RED ofd::Color::Instance(255,0,0) 
    #define COLOR_GREEN ofd::Color::Instance(0,255,0) 
    #define COLOR_BLUE ofd::Color::Instance(0,0,255) 

    // ======== struct ColorStop_t ========
    typedef struct _ColorStop {

        ColorPtr Color;
        double Offset;

        _ColorStop(ColorPtr color, double offset) : 
            Color(color), Offset(offset){
            }

    } ColorStop_t;

    typedef std::vector<ColorStop_t> ColorStopArray;

}; // namespace ofd

#endif // __OFD_COLOR_H__
