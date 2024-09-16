#ifndef __OFD_PAGE_H__
#define __OFD_PAGE_H__

#include <memory>
#include <string>
#include "ofd/Common.h"
#include "ofd/Layer.h"

namespace ofd{

    class Page : public std::enable_shared_from_this<Page> {
        private:
            Page(DocumentPtr document);
        public:
            static PagePtr CreateNewPage(DocumentPtr document);
            virtual ~Page();
            PagePtr GetSelf(){return shared_from_this();};
            std::string to_string() const;

            // =============== Public Attributes ================
        public:
            uint64_t ID;
            std::string BaseLoc;
            CT_PageArea Area;

            // =============== Public Methods ================
        public:
            bool Open();
            void Close();
            
            // 获得完整显示在窗口中需要的缩放比例
            double GetFitScaling(double screenWidth, double screenHeight, double resolutionX, double resolutionY);

            LayerPtr AddNewLayer(LayerType layerType);
            const LayerPtr GetBodyLayer() const;
            LayerPtr GetBodyLayer();
            void AddObject(ObjectPtr object) {GetBodyLayer()->AddObject(object);};

            // Called by Package::Save()
            std::string GeneratePageXML() const;

            // ---------------- Private Attributes ----------------
        public:
            bool IsOpened() const {return m_opened;};
            const DocumentPtr GetDocument() const {return m_document.lock();};
            DocumentPtr GetDocument(){return m_document.lock();};

        private:
            std::weak_ptr<Document> m_document;
            bool m_opened;
            LayerArray m_layers;

            // Called by Page::GeneratePageXML()
            void generateContentXML(utils::XMLWriter &writer) const;

            // Called by Page::Open()
            bool fromPageXML(const std::string &strPageXML);
            bool fromContentXML(utils::XMLElementPtr contentElement);
            LayerPtr fromLayerXML(utils::XMLElementPtr layerElement);

    }; // class Page;


}; // namespace ofd

#endif // __OFD_PAGE_H__
