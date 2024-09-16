/**
 * @brief   CSV files into HTML сonverter
 * @package csv
 * @file    csv.hpp
 * @author  dmryutov (dmryutov@gmail.com)
 * @date    03.04.2017 -- 18.10.2017
 */
#include <algorithm>
#include <sstream>
#include <vector>
#include <pugixml.hpp>

#include "tools.hpp"

#include "csv.hpp"


namespace csv {

/** Max file chunk size */
const size_t MAX_SIZE = 4000;

// public:
Csv::Csv(const std::string& fileName, const char& delimiter, const char& newline, const char& quote)
	: FileExtension(fileName), m_delimiter(delimiter), m_newline(newline), m_quote(quote) {}

int Csv::convert(bool addStyle, bool extractImages, char mergingMode) {
	m_addStyle      = addStyle;
	m_extractImages = extractImages;
	m_mergingMode   = mergingMode;

	std::ifstream file(m_fileName);
	detectDelimiter(file);

	auto tableTag = m_htmlTree.append_child("html").append_child("body").append_child("table");

	std::string line;
	while(std::getline(file, line)) {
		auto tr = tableTag.append_child("tr");

		std::string cell;
		bool inQuote = false;
		auto lineEnd = line.end();
		for (auto it = line.begin(); it != lineEnd; ++it) {
			char c = *it;
			if (c == m_quote && *std::prev(it) != '\\')
				inQuote = !inQuote;
			if (inQuote || c != m_delimiter)
				cell += c;
			else {
				tr.append_child("td").append_child(pugi::node_pcdata)
					.set_value(clearCell(cell).c_str());
				cell.clear();
			}
		}
		if (!cell.empty())
			tr.append_child("td").append_child(pugi::node_pcdata)
				.set_value(clearCell(cell).c_str());
	}

	file.close();
    return 0;
}


// private:
void Csv::detectDelimiter(std::ifstream& file) {
	// Get file size
	file.seekg(0, std::ios::end);
	size_t fileSize = file.tellg();
	file.seekg(0, std::ios::beg);
	// Default variables
	bool hasNewline     = false;
	bool hasSingleQuote = false;
	bool inQuote        = false;
	size_t size         = (std::min)(fileSize, MAX_SIZE);
	std::vector<int>  counters {0, 0, 0, 0};
	std::vector<char> buffer;

	buffer.resize(size);
	file.read(buffer.data(), size);
	auto bufferEnd = buffer.end();

	for (auto it = buffer.begin(); it != bufferEnd; ++it) {
		char c = *it;
		if (c == m_quote && *std::prev(it) != '\\')
			inQuote = !inQuote;
		if (inQuote)
			continue;

		switch (c) {
			case '\r':
				m_newline  = '\r';
				hasNewline = true;
				break;
			case '\n':
				hasNewline = true;
				break;
			case '\'':
				if (!hasSingleQuote) {
					m_quote = c;
					hasSingleQuote = true;
				}
				break;
			case ',':
				if (!hasNewline) ++counters[0];
				break;
			case '\t':
				if (!hasNewline) ++counters[1];
				break;
			case '|':
				if (!hasNewline) ++counters[2];
				break;
			case ';':
				if (!hasNewline) ++counters[3];
				break;
		}
	}

	// Detect delimiter
	if (counters[1] > 0 && counters[1] > counters[0])
		m_delimiter = '\t';
	else if (counters[2] > counters[0])
		m_delimiter = '|';
	else if (counters[3] > counters[0])
		m_delimiter = ';';
	else
		m_delimiter = ',';

	file.seekg(0, std::ios::beg);
}

std::string Csv::clearCell(std::string& cell) const {
	if (cell.empty())
		return "";
	cell = tools::rtrim(cell);
	if (cell[0] == m_quote && cell[cell.size()-1] == m_quote)
		return cell.substr(1, cell.size() - 2);
	return cell;
}

}  // End namespace
