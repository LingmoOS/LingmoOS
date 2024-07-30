Ts.setcall("test_basic", function(arg) {
    return arg + " bar";
});

Ts.setcall("test_unicode", function(arg) {
    return arg + " фу";
});

Ts.setcall("test_hascall", function(arg) {
    return Ts.hascall(arg) ? "yes" : "no";
});

Ts.setcall("test_acall", function(call, arg) {
    return Ts.acall(call, arg);
});

Ts.setcall("test_load", function() {
    Ts.load("loaded");
    return fooIt(blurb);
});

Ts.setcall("test_fallback", function() {
    Ts.fallback();
    return "ignored";
});

Ts.setcall("test_msgid", function() {
    return Ts.msgid();
});

Ts.setcall("test_msgtrf", function() {
    return Ts.msgstrf();
});

Ts.setcall("test_msgctxt", function() {
    return Ts.msgctxt();
});

Ts.setcall("test_msgkey", function() {
    return Ts.msgkey();
});

Ts.setcall("test_nsubs", function() {
    return Ts.nsubs().toString();
});

Ts.setcall("test_subs", function(ind) {
    return Ts.subs(ind);
});

Ts.setcall("test_vals", function(ind, mul) {
    return (Ts.subs(ind) * mul).toString();
});

Ts.setcall("test_dynctxt", function(key) {
    return Ts.dynctxt(key);
});

Ts.setcall("test_dbgputs", function() {
    Ts.dbgputs("Some debugging info...");
    return "debugged";
});

Ts.setcall("test_warnputs", function() {
    Ts.warnputs("Some warning info...");
    return "warned";
});

Ts.setcall("test_setcallForall", function() {
    Ts.setcallForall("print_msgkey", function() {
        Ts.dbgputs(Ts.msgkey());
    });
    return "done";
});

Ts.setcall("test_toUpperFirst", function(arg) {
    return Ts.toUpperFirst(arg);
});

Ts.setcall("test_toLowerFirst", function(arg) {
    return Ts.toLowerFirst(arg);
});

Ts.setcall("test_loadProps", function(relpath) {
    Ts.loadProps(relpath);
    return "loaded";
});

Ts.setcall("test_getProp", function(relpath, phrase, prop) {
    Ts.loadProps(relpath);
    return Ts.getProp(phrase, prop);
});

Ts.setcall("test_setProp", function(phrase, prop, value) {
    Ts.setProp(phrase, prop, value);
    return Ts.getProp(phrase, prop);
});

Ts.setcall("test_normKey", function(phrase) {
    return Ts.normKey(phrase);
});

Ts.setcall("test_getConfString", function(key) {
    return Ts.getConfString(key);
});

Ts.setcall("test_getConfStringWithDefault", function(key, defValue) {
    return Ts.getConfString(key, defValue);
});

Ts.setcall("test_getConfBool", function(key) {
    return "" + Ts.getConfBool(key);
});

Ts.setcall("test_getConfBoolWithDefault", function(key, defValue) {
    return "" + Ts.getConfBool(key, defValue);
});

Ts.setcall("test_getConfNumber", function(key) {
    return "" + Ts.getConfNumber(key);
});

Ts.setcall("test_getConfNumberWithDefault", function(key, defValue) {
    return "" + Ts.getConfNumber(key, defValue);
});
