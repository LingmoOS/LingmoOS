// Copyright 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "keyboard_model.h"
#include <algorithm>
#include <linux/input-event-codes.h>
#include <xkbcommon/xkbcommon-keysyms.h>
namespace Lingmo::Shell {
std::vector<std::vector<ScreenKey>> ScreenKeyboardModel::rows() const {
    const bool upper = m_caps != m_shift;
    std::vector<std::vector<ScreenKey>> result(5);
    result[0].push_back({"escape", "Esc", XKB_KEY_Escape});
    const std::string digits = "1234567890", shifted = "!@#$%^&*()";
    for (size_t i=0;i<digits.size();++i) {
        const char c = m_shift || m_symbols ? shifted[i] : digits[i];
        result[0].push_back({"digit-"+std::to_string(i),std::string(1,c),uint32_t(c)});
    }
    result[0].push_back({"backspace","⌫",XKB_KEY_BackSpace,1.7});
    result[1].push_back({"tab","Tab",XKB_KEY_Tab,1.4});
    const std::vector<std::string> letters={"qwertyuiop","asdfghjkl","zxcvbnm"};
    const std::vector<std::string> symbols={"[]{}<>/\\|~","+-=_%&*!?",".,:;\"'@"};
    for (size_t row=0;row<3;++row) {
        if (row==1) result[row+1].push_back({"caps","Caps",0,1.7});
        if (row==2) result[row+1].push_back({"shift","Shift",0,2.0});
        for(size_t i=0;i<letters[row].size();++i) {
            char c = m_symbols ? symbols[row][i] : letters[row][i];
            if(!m_symbols && upper) c = char(c-'a'+'A');
            result[row+1].push_back({std::string("letter-")+letters[row][i],std::string(1,c),uint32_t(c)});
        }
    }
    result[1].push_back({"delete","Del",XKB_KEY_Delete,1.3});
    result[2].push_back({"enter","Enter",XKB_KEY_Return,2.0});
    result[3].push_back({"comma",",",XKB_KEY_comma});
    result[3].push_back({"period",".",XKB_KEY_period});
    result[3].push_back({"up","↑",XKB_KEY_Up});
    result[4]={{"symbols",m_symbols?"ABC":"#+=",0,1.4},{"ctrl","Ctrl",0,1.4},{"alt","Alt",0,1.4},
        {"space","Space",XKB_KEY_space,6},{"left","←",XKB_KEY_Left},{"down","↓",XKB_KEY_Down},{"right","→",XKB_KEY_Right}};
    return result;
}
bool ScreenKeyboardModel::toggle(const std::string& id) {
    if(id=="shift")m_shift=!m_shift;
    else if(id=="caps")m_caps=!m_caps;
    else if(id=="ctrl")m_control=!m_control;
    else if(id=="alt")m_alt=!m_alt;
    else if(id=="symbols")m_symbols=!m_symbols;
    else return false;
    return true;
}
void ScreenKeyboardModel::consumed(){m_shift=m_control=m_alt=false;}
void ScreenKeyboardModel::reset(){m_shift=m_caps=m_control=m_alt=m_symbols=false;}
bool ScreenKeyboardModel::selected(const std::string& id) const {
    return id=="shift"?m_shift:id=="caps"?m_caps:id=="ctrl"?m_control:id=="alt"?m_alt:id=="symbols"?m_symbols:false;
}
std::vector<uint32_t> screenKeyboardChord(uint32_t key,const std::vector<uint32_t>& modifiers,bool control,bool alt,const std::vector<uint32_t>& held) {
    if(std::find(held.begin(),held.end(),key)!=held.end())return {};
    std::vector<uint32_t> result;
    auto add=[&](uint32_t code){if(std::find(held.begin(),held.end(),code)==held.end() && std::find(result.begin(),result.end(),code)==result.end())result.push_back(code);};
    if(control && std::find(held.begin(),held.end(),KEY_RIGHTCTRL)==held.end())add(KEY_LEFTCTRL);
    if(alt && std::find(held.begin(),held.end(),KEY_RIGHTALT)==held.end())add(KEY_LEFTALT);
    for(auto modifier:modifiers)add(modifier);
    add(key);return result;
}
}
