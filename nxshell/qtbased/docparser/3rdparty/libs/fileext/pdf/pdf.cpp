/**
 * @brief     PDF files into HTML сonverter
 * @package   pdf
 * @file      pdf.cpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright Alex Rembish (https://github.com/rembish/TextAtAnyCost)
 * @date      06.08.2017 -- 29.01.2018
 */
#include "pdf.hpp"

#include <poppler-document.h>
#include <poppler-page.h>
#include <iostream>

namespace pdf {

// public:
Pdf::Pdf(const std::string& fileName)
	: FileExtension(fileName) {}

int Pdf::convert(bool addStyle, bool extractImages, char mergingMode) {
    poppler::document *doc = poppler::document::load_from_file(m_fileName);
    if (!doc || doc->is_locked()) {
        std::cerr << "PDF file load failed:" << m_fileName << std::endl;
        delete doc;
        return -1;
    }

    int numPage = doc->pages();
    for (int i = 0; i < numPage; ++i) {
        poppler::page *page = doc->create_page(i);
        if (page) {
            const auto &text = page->text();
            if (!text.empty()) {
                const auto strutf8 = text.to_utf8();
                std::string str;
                str.assign(strutf8.begin(), strutf8.end());
                m_text += str;
            }
            delete page;
        }
    }

    delete doc;
    return 0;
}

}  // End namespace
