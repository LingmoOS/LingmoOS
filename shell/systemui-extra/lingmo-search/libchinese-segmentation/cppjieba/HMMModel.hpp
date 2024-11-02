/*
 * The MIT License (MIT)
 *
 * Copyright (C) 2013 Yanyi Wu
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
*/
#pragma once

#include "limonp/StringUtil.hpp"
//#define USE_CEDAR_SEGMENT //使用cedar初步测试性能损失3%-5%左右，内存占用降低近1M
#ifdef USE_CEDAR_SEGMENT
#include "cedar/cedar.h"
#endif
namespace cppjieba {

using namespace limonp;
#ifdef USE_CEDAR_SEGMENT
typedef cedar::da<float, -1, -2, false> EmitProbMap;
#else
typedef unordered_map<Rune, double> EmitProbMap;
#endif
struct HMMModel {
    /*
     * STATUS:
     * 0: HMMModel::B, 1: HMMModel::E, 2: HMMModel::M, 3:HMMModel::S
     * */
    enum {B = 0, E = 1, M = 2, S = 3, STATUS_SUM = 4};

    HMMModel(const string& modelPath) {
        memset(startProb, 0, sizeof(startProb));
        memset(transProb, 0, sizeof(transProb));
        statMap[0] = 'B';
        statMap[1] = 'E';
        statMap[2] = 'M';
        statMap[3] = 'S';
        emitProbVec.push_back(&emitProbB);
        emitProbVec.push_back(&emitProbE);
        emitProbVec.push_back(&emitProbM);
        emitProbVec.push_back(&emitProbS);
        LoadModel(modelPath);
    }
    ~HMMModel() {
    }
    void LoadModel(const string& filePath) {
        ifstream ifile(filePath.c_str());
        XCHECK(ifile.is_open()) << "open " << filePath << " failed";
        string line;
        vector<string> tmp;
        vector<string> tmp2;
        //Load startProb
        XCHECK(GetLine(ifile, line));
        Split(line, tmp, " ");
        XCHECK(tmp.size() == STATUS_SUM);

        for (size_t j = 0; j < tmp.size(); j++) {
            startProb[j] = atof(tmp[j].c_str());
        }

        //Load transProb
        for (size_t i = 0; i < STATUS_SUM; i++) {
            XCHECK(GetLine(ifile, line));
            Split(line, tmp, " ");
            XCHECK(tmp.size() == STATUS_SUM);

            for (size_t j = 0; j < tmp.size(); j++) {
                transProb[i][j] = atof(tmp[j].c_str());
            }
        }

        //Load emitProbB
        XCHECK(GetLine(ifile, line));
        XCHECK(LoadEmitProb(line, emitProbB));

        //Load emitProbE
        XCHECK(GetLine(ifile, line));
        XCHECK(LoadEmitProb(line, emitProbE));

        //Load emitProbM
        XCHECK(GetLine(ifile, line));
        XCHECK(LoadEmitProb(line, emitProbM));

        //Load emitProbS
        XCHECK(GetLine(ifile, line));
        XCHECK(LoadEmitProb(line, emitProbS));
    }
    double GetEmitProb(const EmitProbMap* ptMp, Rune key,
                       double defVal)const {
#ifdef USE_CEDAR_SEGMENT
        char str_key[8];
        snprintf(str_key, sizeof(str_key), "%d", key);
        float result = ptMp->exactMatchSearch<float>(str_key);
        return result < 0 ? defVal : result;
#else
        EmitProbMap::const_iterator cit = ptMp->find(key);

        if (cit == ptMp->end()) {
            return defVal;
        }

        return cit->second;
#endif
    }
    bool GetLine(ifstream& ifile, string& line) {
        while (getline(ifile, line)) {
            Trim(line);

            if (line.empty()) {
                continue;
            }

            if (StartsWith(line, "#")) {
                continue;
            }

            return true;
        }

        return false;
    }
    bool LoadEmitProb(const string& line, EmitProbMap& mp) {
        if (line.empty()) {
            return false;
        }

        vector<string> tmp, tmp2;
        RuneArray unicode;
        Split(line, tmp, ",");

        for (size_t i = 0; i < tmp.size(); i++) {
            Split(tmp[i], tmp2, ":");

            if (2 != tmp2.size()) {
                XLOG(ERROR) << "emitProb illegal.";
                return false;
            }

            if (!DecodeRunesInString(tmp2[0], unicode) || unicode.size() != 1) {
                XLOG(ERROR) << "TransCode failed.";
                return false;
            }
#ifdef USE_CEDAR_SEGMENT
            char str_key[8];
            snprintf(str_key, sizeof(str_key), "%d", unicode[0]);
            mp.update(str_key, std::strlen(str_key), atof(tmp2[1].c_str()));
#else
            mp[unicode[0]] = atof(tmp2[1].c_str());
#endif
        }

        return true;
    }

    char statMap[STATUS_SUM];
    double startProb[STATUS_SUM];
    double transProb[STATUS_SUM][STATUS_SUM];
    EmitProbMap emitProbB;
    EmitProbMap emitProbE;
    EmitProbMap emitProbM;
    EmitProbMap emitProbS;
    vector<EmitProbMap* > emitProbVec;
}; // struct HMMModel

} // namespace cppjieba

