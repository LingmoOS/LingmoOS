/**
 * @brief     RTF files into HTML сonverter
 * @package   rtf
 * @file      keyword.cpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright lvu (https://github.com/lvu/rtf2html)
 * @date      31.07.2016 -- 18.10.2017
 */
#include "keyword.hpp"


namespace rtf {

Keyword::Keyword(std::string::iterator& it) {
	char currentChar = *it;
	m_isControlChar = !isalpha(currentChar);

	if (m_isControlChar) {
		m_controlChar = currentChar;
		++it;
	}
	else {
		do {
			m_name += currentChar;
		}
		while (isalpha(currentChar = *++it));

		std::string parameterText;
		while ((isdigit(currentChar) || currentChar == '-')) {
			parameterText += currentChar;
			currentChar    = *++it;
		}

		if (parameterText.empty())
			m_parameter = -1;
		else
			m_parameter = std::stoi(parameterText);

		if (currentChar == ' ')
			++it;
	}
}

}  // End namespace