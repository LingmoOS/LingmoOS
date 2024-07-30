// kdelibs4.js of Japanese KDE translation
// Maintained by Chusslove Illich <caslav.ilic@gmx.net>

// +++ Global dictionary of states.
// _states_[state], if defined, is set to some value for the state.
var _states_ = {};

// *** Set global states to values.
// : setStates(state1 value1 state2 value2 ...)
// Returns empty string.
function setStates (/*...*/) {
    var f = "setStates: ";
    if (arguments.length % 2 != 0) {
        throw Error(f + "must have even number of arguments");
    }
    for (var i = 0; i < arguments.length; i += 2) {
        var state = arguments[i];
        var val = arguments[i + 1];
        _states_[state] = val;
    }
    return "";
}

// *** Get global state value; empty string if not defined.
function getState (state) {
    if (state in _states_) {
        return _states_[state];
    }
    return "";
}

// *** Set properties to the phrase given by current msgstrf.
// : setMsgstrProperties(prop1 value1 prop2 value2...)
// Returns empty string.
function setMsgstrProperties (/*...*/) {
    var f = "setMsgstrProperties: ";
    if (arguments.length % 2 != 0) {
        throw Error(f + "must have even number of arguments");
    }

    var phrase = Ts.msgstrf();
    for (var i = 0; i < arguments.length; i += 2) {
        var property = arguments[i];
        var val = arguments[i + 1];
        Ts.setProp(phrase, property, val);
    }
    return "";
}

// *** Get property value of the given phrase; empty string if not defined.
function getProperty (phrase, property) {
    var val = Ts.getProp(phrase, property);
    if (!val) {
        return "";
    }
    return val;
}

// *** Get form of the given phrase.
// A "form" is taken to be a special type of property which defaults to the
// phrase itself when not defined. Hence, if the requested form has not been
// defined, phrase is returned as is.
function getFormProperty (phrase, formm) {
    var val = Ts.getProp(phrase, formm);
    if (!val) {
        return phrase;
    }
    return val;
}

// *** Select one of the strings according to the test value.
// : selectByValue(testvalue value1 str1 value2 str2 ... defaultstr)
// Select the string to the first value equal to the test value,
// or returns default string if there is no match.
function selectByValue (/*...*/) {
    var f = "selectByValue: ";
    if (arguments.length % 2 != 0) {
        throw Error(f + "must have even number of arguments.");
    }
    if (arguments.length < 2)
        throw Error(f + "must have at least two arguments.");

    var testval = arguments[0];
    for (var i = 1; i < arguments.length - 1; i += 2) {
        var val = arguments[i];
        var str = arguments[i + 1];
        if (testval == val) {
            return str;
        }
    }
    // Return the default choice if no other returned before.
    return arguments[arguments.length - 1];
}

// *** Return first non-empty string among the arguments, or empty if all empty.
// : firstNonEmpty(str1 str2 ...)
function firstNonEmpty (/*...*/) {
    for (var i = 0; i < arguments.length; ++i) {
        if (arguments[i].length > 0) {
            return arguments[i];
        }
    }
    return "";
}

// *** Return the finalized ordinary msgstr.
function theMsgstr () {
    return Ts.msgstrf();
}

// *** If the first character in the phrase is non-Japanese, return
// the phrase with space added to the start, otherwise return as is.
function spaceOnDemandLeft (phrase) {
    if (phrase.length == 0) {
        return phrase;
    }
    var firstChar = phrase.charAt(0);
    if (firstChar != " " && firstChar < "\u2E00") {
        return " " + phrase;
    }
    return phrase;
}

// *** If the last character in the phrase is non-Japanese, return
// the phrase with space added to the end, otherwise return as is.
function spaceOnDemandRight (phrase) {
    if (phrase.length == 0) {
        return phrase;
    }
    var lastChar = phrase.charAt(phrase.length - 1);
    if (lastChar != " " && lastChar < "\u2E00") {
        return phrase + " ";
    }
    return phrase;
}

// *** Combination of spaceOnDemandLeft and spaceOnDemandRight: both first
// and last characters are checked, and spaces added where needed.
function spaceOnDemandBoth (phrase) {
    return spaceOnDemandLeft(spaceOnDemandRight(phrase));
}

// *** Strip the accelerator marker, composed of an ASCII alphanumeric
// within parenthesis (with or without an ampersand in front).
stripAccRx = /\((&|&amp;)?[a-zA-Z0-9]\)/;
function stripAccelerator (phrase) {
    return phrase.replace(stripAccRx, "");
}

// ==> Exports to PO calls
Ts.setcall("~set", setStates);
Ts.setcall("~get", getState);
Ts.setcall("~setProps", setMsgstrProperties);
Ts.setcall("~getProp", getProperty);
Ts.setcall("~getForm", getFormProperty);
Ts.setcall("~case", selectByValue);
Ts.setcall("~first", firstNonEmpty);
Ts.setcall("~this", theMsgstr);
Ts.setcall("~spacerL", spaceOnDemandLeft);
Ts.setcall("~spacerR", spaceOnDemandRight);
Ts.setcall("~spacerB", spaceOnDemandBoth);
Ts.setcall("~stripAccel", stripAccelerator);


// ------------------------------
// When verbs such as "Configure" are used as the positive answer in question
// dialogs, "設定する", as opposed to "設定しない (Do Not Configure)", would
// be more appropriate than "設定". However, it would sound funny if the same
// translation is used in other contexts (e.g. @title:window), so we cannot
// do it safely.
// Instead, attach the proper answers to the message caption/text in the PO,
// and then retrieve them in filtering messages for message dialog buttons
// in kdelibs4.po (those with context "@action:button filter-...").

// Dictionary of answers.
var _answers_ = {};

// Set answers as key-value pairs, as many as needed.
// The call always signals fallback.
function setAnswers (/*...*/)
{
    if (arguments.length % 2 != 0)
        throw Error("Answers setter given odd number of arguments.");

    for (var i = 0; i < arguments.length; i += 2) {
        var akey = arguments[i];
        var answer = arguments[i + 1];
        _answers_[akey] = answer;
    }

    throw Ts.fallback();
}
Ts.setcall("set-answers", setAnswers);
// msgid "... Would you like to configure it now?"
// msgstr ""
// "... 今設定しますか？"
// "|/|"
// "$[set-answers yes '設定する(&Y)' no '設定しない(&N)']"

// Get an answer by key.
// Signals fallback if the answer with the given key is not set.
// Deletes the answer from the dictionary, so that it doesn't happen
// that it gets retrieved in a later, unrelated question.
function getAnswer (akey)
{
    answer = _answers_[akey];
    if (!answer) throw Ts.fallback();
    delete _answers_[akey];
    return answer;
}
Ts.setcall("get-answer", getAnswer);
// msgctxt "@action:button filter-yes"
// msgid "%1"
// msgstr "%1|/|$[get-answer yes]"

// Reset all answers, so that they don't get used for the wrong question.
// Always signals fallback.
function resetAnswers ()
{
    _answers_ = {};
}
Ts.setcall("reset-answers", resetAnswers);
// msgctxt "@action:button post-filter"
// msgid "."
// msgstr ".|/|$[reset-answers]"
