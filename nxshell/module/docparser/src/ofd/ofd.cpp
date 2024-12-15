// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ofd.h"

#include <ofd/Package.h>
#include <ofd/Document.h>
#include <ofd/Page.h>
#include <ofd/TextObject.h>

#include <cassert>

namespace ofd {

static std::string objectText(LayerPtr layer, size_t index)
{
    assert(layer);
    ObjectPtr object = layer->GetObject(index);
    if (!object)
        return {};
    std::string outputText;
    TextObject *textObject = dynamic_cast<TextObject *>(object.get());
    if (textObject && textObject->Type == ObjectType::TEXT) {
        size_t codeSize = textObject->GetNumTextCodes();
        for (size_t i = 0; i != codeSize; ++i) {
            Text::TextCode code = textObject->GetTextCode(i);
            outputText.append(code.Text);
        }
    }
    return outputText;
}

static std::string pageText(DocumentPtr doc, size_t index)
{
    assert(doc);
    PagePtr page = doc->GetPage(index);
    if (!page)
        return {};
    page->Open();
    LayerPtr layer = page->GetBodyLayer();
    if (!layer)
        return {};

    size_t totalObjects = layer->GetNumObjects();
    std::string output;
    for (size_t i = 0; i != totalObjects; ++i)
        output.append(objectText(layer, i));
    return output;
}

Ofd::Ofd(const std::string &fileName)
    : FileExtension(fileName) {}

int Ofd::convert(bool addStyle, bool extractImages, char mergingMode)
{
    (void)addStyle;
    (void)extractImages;
    (void)mergingMode;

    ofd::PackagePtr package = std::make_shared<ofd::Package>();
    if (!package->Open(m_fileName))
        return 1;

    DocumentPtr document = package->GetDefaultDocument();
    if (!document)
        return 1;

    bool opened = document->Open();
    if (!opened)
        return 1;

    size_t totalPages = document->GetNumPages();
    if (totalPages <= 0)
        return 1;

    std::string output;
    for (size_t i = 0; i != totalPages; ++i)
        output.append(pageText(document, i));

    m_text = output;
    return 0;
}

}   // namespace ofd
