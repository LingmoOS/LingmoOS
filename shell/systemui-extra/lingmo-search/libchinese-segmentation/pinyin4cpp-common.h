/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: jixiaoxu <jixiaoxu@kylinos.cn>
 *
 */

#ifndef PINYIN4CPP_COMMON_H
#define PINYIN4CPP_COMMON_H

/**
 * @brief The PinyinDataStyle enum
 * Default 默认模式，“中心” return “zhong xin”
 * Tone 带读音模式 #“中心” return “zhōng xīn”
 * Tone2 带读音模式2 #“中心” return “zho1ng xi1n”
 * Tone3 带读音模式3 #“中心” return “zhong1 xin1”
 * FirstLetter 首字母模式 #“中心” return “z x”
 * English 英文翻译模式(暂不支持) #“中心” return “center,heart,core”
 */
enum class PinyinDataStyle {
    Default       = 1u << 0,
    Tone          = 1u << 1,
    Tone2         = 1u << 2,
    Tone3         = 1u << 3,
    FirstLetter   = 1u << 4,
    English       = 1u << 5
};

/**
 * @brief The SegType enum
 * Segmentation 默认带分词 #“银河灵墨”->“银河”“灵墨”
 * NoSegmentation 无分词模式 #“银河灵墨”
 */
enum class SegType {
    Segmentation    = 1u << 0,
    NoSegmentation  = 1u << 1
};

/**
 * @brief The PolyphoneType enum
 * Disable 默认不启用多音字，“奇安信”return “qi an xin”多音字按照常用读音返回
 * Enable 启用多音字 “奇安信” return“qi,ji an xin”
 * 注意：多音词返回格式为 “朝阳” return "zhao/chao yang/yang"
 */
enum class PolyphoneType {
    Disable       = 1u << 0,
    Enable        = 1u << 1
};

/**
 * @brief The ExDataProcessType enum
 * Default 默认无拼音数据直接返回，“123木头人” return "123 mu tou ren"（分词模式）
 * Delete  删除多余数据，#“123木头人” return "mu tou ren"（分词模式）
 */
enum class ExDataProcessType {
    Default       = 1u << 0,
    Delete        = 1u << 1
};

#endif //PINYIN4CPP_COMMON_H
