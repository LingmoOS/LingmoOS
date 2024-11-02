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

#include <iostream>
#include <fstream>
#include <memory.h>
#include <cassert>
#include "HMMModel.hpp"
#include "SegmentBase.hpp"

namespace cppjieba {

const double MIN_DOUBLE = -3.14e+100;

class HMMSegment: public SegmentBase {
public:
    HMMSegment(const HMMModel* model)
        : model_(model) {
    }
    ~HMMSegment() { }

    virtual void Cut(RuneStrArray::const_iterator begin, RuneStrArray::const_iterator end, vector<WordRange>& res, bool,
                     size_t) const override {
        RuneStrArray::const_iterator left = begin;
        RuneStrArray::const_iterator right = begin;

        while (right != end) {
            if (right->rune < 0x80) { //asc码
                if (left != right) {
                    InternalCut(left, right, res);
                }

                left = right;

                do {
                    right = SequentialLetterRule(left, end);//非英文字符则返回left，否则返回left后非英文字母的位置

                    if (right != left) {
                        break;
                    }

                    right = NumbersRule(left, end);//非数字则返回left，否则返回left后非数字的位置

                    if (right != left) {
                        break;
                    }

                    right ++;
                } while (false);

                WordRange wr(left, right - 1);
                res.push_back(wr);
                left = right;
            } else {
                right++;
            }
        }

        if (left != right) {
            InternalCut(left, right, res);
        }
    }

    virtual void CutWithSentence(const string& s, RuneStrArray::const_iterator begin, RuneStrArray::const_iterator end, vector<string>& res, bool hmm,
                     size_t) const override {
        std::ignore = s;
        std::ignore = begin;
        std::ignore = end;
        std::ignore = res;
        std::ignore = hmm;
    }
    virtual void CutWithSentence(const string& s, RuneStrArray::const_iterator begin, RuneStrArray::const_iterator end, unordered_map<string, KeyWord>& res, bool hmm,
                     size_t) const override {
        std::ignore = s;
        std::ignore = begin;
        std::ignore = end;
        std::ignore = res;
        std::ignore = hmm;
    }
private:
    // sequential letters rule
    RuneStrArray::const_iterator SequentialLetterRule(RuneStrArray::const_iterator begin,
                                                      RuneStrArray::const_iterator end) const {
        Rune x = begin->rune;

        if (('a' <= x && x <= 'z') || ('A' <= x && x <= 'Z')) {
            begin ++;
        } else {
            return begin;
        }

        while (begin != end) {
            x = begin->rune;

            if (('a' <= x && x <= 'z') || ('A' <= x && x <= 'Z') || ('0' <= x && x <= '9')) {
                begin ++;
            } else {
                break;
            }
        }

        return begin;
    }
    //
    RuneStrArray::const_iterator NumbersRule(RuneStrArray::const_iterator begin, RuneStrArray::const_iterator end) const {
        Rune x = begin->rune;

        if ('0' <= x && x <= '9') {
            begin ++;
        } else {
            return begin;
        }

        while (begin != end) {
            x = begin->rune;

            if (('0' <= x && x <= '9') || x == '.') {
                begin++;
            } else {
                break;
            }
        }

        return begin;
    }
    void InternalCut(RuneStrArray::const_iterator begin, RuneStrArray::const_iterator end, vector<WordRange>& res) const {
        vector<size_t> status;
        Viterbi(begin, end, status);

        RuneStrArray::const_iterator left = begin;
        RuneStrArray::const_iterator right;

        for (size_t i = 0; i < status.size(); i++) {
            if (status[i] % 2) { //if (HMMModel::E == status[i] || HMMModel::S == status[i])
                right = begin + i + 1;
                WordRange wr(left, right - 1);
                res.push_back(wr);
                left = right;
            }
        }
    }

    void Viterbi(RuneStrArray::const_iterator begin,
                 RuneStrArray::const_iterator end,
                 vector<size_t>& status) const {
        size_t Y = HMMModel::STATUS_SUM;
        size_t X = end - begin;

        size_t XYSize = X * Y;
        size_t now, old, stat;
        double tmp, endE, endS;

        //vector<int> path(XYSize);
        //vector<double> weight(XYSize);
        int path[XYSize];
        double weight[XYSize];

        //start
        for (size_t y = 0; y < Y; y++) {
            weight[0 + y * X] = model_->startProb[y] + model_->GetEmitProb(model_->emitProbVec[y], begin->rune, MIN_DOUBLE);
            path[0 + y * X] = -1;
        }

        double emitProb;

        for (size_t x = 1; x < X; x++) {
            for (size_t y = 0; y < Y; y++) {
                now = x + y * X;
                weight[now] = MIN_DOUBLE;
                path[now] = HMMModel::E; // warning
                emitProb = model_->GetEmitProb(model_->emitProbVec[y], (begin + x)->rune, MIN_DOUBLE);

                for (size_t preY = 0; preY < Y; preY++) {
                    old = x - 1 + preY * X;
                    tmp = weight[old] + model_->transProb[preY][y] + emitProb;

                    if (tmp > weight[now]) {
                        weight[now] = tmp;
                        path[now] = preY;
                    }
                }
            }
        }

        endE = weight[X - 1 + HMMModel::E * X];
        endS = weight[X - 1 + HMMModel::S * X];
        stat = 0;

        if (endE >= endS) {
            stat = HMMModel::E;
        } else {
            stat = HMMModel::S;
        }

        status.resize(X);

        for (int x = X - 1 ; x >= 0; x--) {
            status[x] = stat;
            stat = path[x + stat * X];
        }
    }

    const HMMModel* model_;
}; // class HMMSegment

} // namespace cppjieba

