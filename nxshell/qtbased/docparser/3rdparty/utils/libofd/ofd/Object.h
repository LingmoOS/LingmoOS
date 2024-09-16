#ifndef __OFD_OBJECT_H__
#define __OFD_OBJECT_H__

#include <memory>
#include "ofd/Common.h"

namespace ofd{

    enum class ObjectType{
        UNKNOWN = -1, // 未知对象
        TEXT,         // 文本对象
        PATH,         // 图形对象
        IMAGE,        // 图像对象
        VIDEO,        // 视频对象
        COMPOSITE,    // 复合对象
    };

    class Object : public std::enable_shared_from_this<Object>{
        public:
        public:
            Object(LayerPtr layer, ObjectType objectType, const std::string& objectLabel);
            virtual ~Object();
            ObjectPtr GetSelf(){return shared_from_this();};

            // =============== Public Attributes ================
        public:
            uint64_t     ID;
            ObjectType   Type;
            std::string  ObjectLabel;

            // -------- GraphUnit attributes --------
            // OFD P50. Page.xsd.
            //ST_Box       Boundary; 
            ofd::Boundary       Boundary; 
            std::string  Name;
            bool         Visible;
            double       CTM[6];
            double       LineWidth;
            int          Alpha;

            // =============== Public Methods ================
        public:
            virtual std::string to_string() const;
            void GenerateXML(utils::XMLWriter &writer) const;
            bool FromXML(utils::XMLElementPtr objectElement);
            virtual void RecalculateBoundary(){};

        protected:
            virtual void GenerateAttributesXML(utils::XMLWriter &writer) const;
            virtual void GenerateElementsXML(utils::XMLWriter &writer) const;

            virtual bool FromAttributesXML(utils::XMLElementPtr objectElement);
            virtual bool IterateElementsXML(utils::XMLElementPtr childElement);

            // ---------------- Private Attributes ----------------
        public:
            const LayerPtr GetLayer() const;
            LayerPtr GetLayer();
            const PagePtr GetPage() const;
            PagePtr GetPage();
            const DocumentPtr GetDocument() const;
            DocumentPtr GetDocument();
            const ResourcePtr GetDocumentRes() const;
            ResourcePtr GetDocumentRes();
            const ResourcePtr GetPublicRes() const;
            ResourcePtr GetPublicRes();

        protected:
            std::weak_ptr<Layer> m_layer;

        private:
            bool FromElementsXML(utils::XMLElementPtr objectElement);

    }; // class Object


}; // namespace ofd

#endif // __OFD_OBJECT_H__

