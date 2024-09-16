// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package inputdevices

import (
	"encoding/xml"
	"io/ioutil"

	"github.com/linuxdeepin/dde-daemon/inputdevices1/iso639"
	"github.com/linuxdeepin/go-lib/gettext"
	lib_locale "github.com/linuxdeepin/go-lib/locale"
	"github.com/linuxdeepin/go-lib/strv"
)

const (
	kbdLayoutsXml = "/usr/share/X11/xkb/rules/base.xml"
	kbdTextDomain = "xkeyboard-config"
)

type XKBConfigRegister struct {
	Layouts []XLayout `xml:"layoutList>layout"`
}

type XLayout struct {
	ConfigItem XConfigItem   `xml:"configItem"`
	Variants   []XConfigItem `xml:"variantList>variant>configItem"`
}

type XConfigItem struct {
	Name        string   `xml:"name"`
	Description string   `xml:"description"`
	Languages   []string `xml:"languageList>iso639Id"`
}

func parseXML(filename string) (XKBConfigRegister, error) {
	var v XKBConfigRegister
	xmlByte, err := ioutil.ReadFile(filename)
	if err != nil {
		return v, err
	}

	err = xml.Unmarshal(xmlByte, &v)
	if err != nil {
		return v, err
	}

	return v, nil
}

type layoutMap map[string]layoutDetail

type layoutDetail struct {
	Languages   []string
	Description string
}

func getLayoutsFromFile(filename string) (layoutMap, error) {
	xmlData, err := parseXML(filename)
	if err != nil {
		return nil, err
	}

	result := make(layoutMap)
	for _, layout := range xmlData.Layouts {
		layoutName := layout.ConfigItem.Name
		desc := layout.ConfigItem.Description
		result[layoutName+layoutDelim] = layoutDetail{
			Languages:   layout.ConfigItem.Languages,
			Description: gettext.DGettext(kbdTextDomain, desc),
		}

		variants := layout.Variants
		for _, v := range variants {
			languages := v.Languages
			if len(v.Languages) == 0 {
				languages = layout.ConfigItem.Languages
			}
			result[layoutName+layoutDelim+v.Name] = layoutDetail{
				Languages:   languages,
				Description: gettext.DGettext(kbdTextDomain, v.Description),
			}
		}
	}

	return result, nil
}

func (layoutMap layoutMap) filterByLocales(locales []string) map[string]string {
	var localeLanguages []string
	for _, locale := range locales {
		components := lib_locale.ExplodeLocale(locale)
		lang := components.Language
		if lang != "" &&
			!strv.Strv(localeLanguages).Contains(lang) {
			localeLanguages = append(localeLanguages, lang)
		}
	}

	languages := make([]string, len(localeLanguages), 3*len(localeLanguages))
	copy(languages, localeLanguages)
	for _, t := range localeLanguages {
		a3Codes := iso639.ConvertA2ToA3(t)
		languages = append(languages, a3Codes...)
	}
	logger.Debug("languages:", languages)

	result := make(map[string]string)
	for layout, layoutDetail := range layoutMap {
		if layoutDetail.matchAnyLang(languages) {
			result[layout] = layoutDetail.Description
		}
	}
	return result
}

func (v *layoutDetail) matchAnyLang(languages []string) bool {
	for _, l := range languages {
		for _, ll := range v.Languages {
			if ll == l {
				return true
			}
		}
	}
	return false
}
