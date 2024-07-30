// kdelibs4.js of Serbian KDE translation

// ------------------------------
// Property getter object contains the following data attributes:
// - callname: the name of the getter call as exposed to PO files
// - propkey: the key of the property as used in pmap files
// - overrides: dictionary of values of this property for certain phrases,
//              which were manually set in the PO file
function Propgetter (callname, propkey)
{
    this.callname = callname;
    this.propkey = propkey;
    this.overrides = {};

    this.getbase = Propgetter_getbase;

    this.getprop = Propgetter_getprop;
    this.getprop_uc = Propgetter_getprop_uc;
    this.getform = Propgetter_getform;
    this.getform_uc = Propgetter_getform_uc;
}

// Base of property/form getter methods attached to getter objects.
// First the property for the given phrase is looked up in overrides,
// and then in Transcript property map (read from pmap files).
// If the property is not found, fallback is signalled if retself is false,
// otherwise the phrase itself is returned.
function Propgetter_getbase (phrase, retself)
{
    if (phrase in this.overrides) {
        return this.overrides[phrase];
    }
    else {
        var prop = Ts.getProp(phrase, this.propkey)
        if (prop != undefined)
            return prop;
    }
    if (retself) {
        return phrase;
    }
    throw Ts.fallback();
}

// Property getter method attached to getter objects.
function Propgetter_getprop (phrase)
{
    return this.getbase(phrase, false);
}

// As previous, but returns the property with the first letter upcased.
function Propgetter_getprop_uc (phrase)
{
    var val = this.getprop(phrase);

    // The second argument is indicating the number of alternatives per
    // alternatives directive -- in case the first letter is within
    // an alternatives directive, all alternatives in that directive
    // should be processed.
    return Ts.toUpperFirst(val, 2);
}

// Form getter method attached to getter objects.
function Propgetter_getform (phrase)
{
    return this.getbase(phrase, true);
}

// As previous, but returns the form with the first letter upcased.
function Propgetter_getform_uc (phrase)
{
    var val = this.getform(phrase);
    // See the comment in Propgetter_getprop_uc().
    return Ts.toUpperFirst(val, 2);
}

// ------------------------------
// Exposing property getters to PO.

// Contains all global property getters.
var _propgetters_ = {};

// Set PO calls for given property getter object.
function setcalls_prop (pgetr)
{
    // Default call.
    Ts.setcall(pgetr.callname, pgetr.getprop, pgetr);
    // "Open with $[callname %1]"

    // Form call.
    Ts.setcall(pgetr.callname + "/f", pgetr.getform, pgetr);
    // "Open with $[callname/f %1]"

    // The calls which capitalize the first letter of the value,
    // named as the default calls but with the first letter capitalized.
    // Only set if the first letter of the call name is actually lowercase.
    callname_uc = Ts.toUpperFirst(pgetr.callname);
    if (callname_uc != pgetr.callname) {
        Ts.setcall(callname_uc, pgetr.getprop_uc, pgetr);
        // "$[Callname %1] starting..."
        Ts.setcall(callname_uc + "/f", pgetr.getform_uc, pgetr);
        // "$[Callname/f %1] starting..."
    }

    // Record getter objects globally.
    // Only for the original name, since the uppercase/form variants are not
    // used when properties are being set (when the global store is needed).
    _propgetters_[pgetr.callname] = pgetr;
}

// Set property value of phrase.
function setprop (phrase, pkey, pval)
{
    // Either create new, or select existing getter.
    var pgetr;
    if (!_propgetters_[pkey]) {
        // Populate new getter if not already defined.
        pgetr = new Propgetter(pkey, pkey);
        // Expose calls to PO.
        setcalls_prop(pgetr);
    }
    else {
        // Get previously defined getter.
        pgetr = _propgetters_[pkey];
    }

    // Add the property into overrides of selected getter.
    pgetr.overrides[phrase] = pval;
}

// Get property value of phrase.
// Signals fallback if the property/phrase combination is not defined.
function getprop (phrase, pkey)
{
    if (_propgetters_[pkey]) {
        return _propgetters_[pkey].getprop(phrase);
    }
    throw Ts.fallback();
}

// Get form of the phrase, or phrase itself if no such form.
function getform (phrase, fkey)
{
    if (_propgetters_[fkey]) {
        return _propgetters_[fkey].getform(phrase);
    }
    return phrase;
}

// Returns true if the phrase has the property.
function hasprop (phrase, pkey)
{
    pg = _propgetters_[pkey];
    if (!pg) {
        return false;
    }
    if (!pg.overrides[phrase] && !Ts.getProp(phrase, pg.propkey)) {
        return false;
    }
    return true;
}

// ------------------------------
// Predefined property getters.

// Call names and corresponding keys in pmap for predefined getters.
// The first letter in a call name should be lowercase; for each call
// another call with the first letter in uppercase will be defined,
// which will upcase the first letter in the property value before
// returning it.
var call_name_to_prop = {
    // Nouns.
    "_izv" : "_izvor", // english original
    "_rod" : "_rod", // gender
    "_broj" : "_broj", // number

    "nom" : "n", // nominative case
    "gen" : "g", // genitive case
    "dat" : "d", // dative case
    "aku" : "a", // accusative case
    "ins" : "i", // instrumental case
    "lok" : "l", // locative case

    // Expressive variants.
    "naredbeno" : "_narb", // command
    "spiskovno" : "_spis", // listed

    // Adjectives.
    "nom-m" : "nm", // nominative, masculine
    "gen-m" : "gm", // genitive, masculine
    "dat-m" : "dm", // dative, masculine
    "aku-m" : "am", // accusative, masculine
    "ins-m" : "im", // instrumental, masculine
    "lok-m" : "lm", // locative, masculine
    "nom-ž" : "nz", // nominative, feminine
    "gen-ž" : "gz", // genitive, feminine
    "dat-ž" : "dz", // dative, feminine
    "aku-ž" : "az", // accusative, feminine
    "ins-ž" : "iz", // instrumental, feminine
    "lok-ž" : "lz", // locative, feminine
    "nom-s" : "ns", // nominative, neuter
    "gen-s" : "gs", // genitive, neuter
    "dat-s" : "ds", // dative, neuter
    "aku-s" : "as", // accusative, neuter
    "ins-s" : "is", // instrumental, neuter
    "lok-s" : "ls", // locative, neuter
    "nom-mk" : "nmk", // nominative, masculine, plural
    "gen-mk" : "gmk", // genitive, masculine, plural
    "dat-mk" : "dmk", // dative, masculine, plural
    "aku-mk" : "amk", // accusative, masculine, plural
    "ins-mk" : "imk", // instrumental, masculine, plural
    "lok-mk" : "lmk", // locative, masculine, plural
    "nom-žk" : "nzk", // nominative, feminine, plural
    "gen-žk" : "gzk", // genitive, feminine, plural
    "dat-žk" : "dzk", // dative, feminine, plural
    "aku-žk" : "azk", // accusative, feminine, plural
    "ins-žk" : "izk", // instrumental, feminine, plural
    "lok-žk" : "lzk", // locative, feminine, plural
    "nom-sk" : "nsk", // nominative, neuter, plural
    "gen-sk" : "gsk", // genitive, neuter, plural
    "dat-sk" : "dsk", // dative, neuter, plural
    "aku-sk" : "ask", // accusative, neuter, plural
    "ins-sk" : "isk", // instrumental, neuter, plural
    "lok-sk" : "lsk", // locative, neuter, plural
};

// Create getter objects for predefined getters.
for (cname in call_name_to_prop) {
    // Create getter object as defined above.
    var pgetr = new Propgetter(cname, call_name_to_prop[cname]);
    // Expose calls to PO.
    setcalls_prop(pgetr);
}

// Special handling for instrumental case, when used for tool application:
// don't pass it along as-is if same as nominative case of the phrase,
// since the sentence can get very different, yet semantically correct meaning.
// Instead, pass genitive case with the preposition "by the help of".
{
    var pgetr = new Propgetter("ins-p", "i");

    // Replace default getter method.
    pgetr.getprop = function (phrase)
    {
        var prop_ins = _propgetters_["ins"].getprop(phrase);
        var prop_nom = _propgetters_["nom"].getprop(phrase);
        if (prop_ins == prop_nom) {
            var prop_gen = _propgetters_["gen"].getprop(phrase);
            return "pomoću " + prop_gen;
        }
        else {
            return prop_ins;
        }
    }

    setcalls_prop(pgetr);
}

// ------------------------------
// Set properties of the given phrase.
// The arguments to the call are the phrase, and a list of as many keys
// followed by their value as desired (i.e. number of elements must be even).
// Keys may also be comma-separated lists instead of a single key, in order
// not to have to repeat the same value when it corresponds to several keys.
//
// The property keys become property getters which can be used to retrive
// the value at a later point. If the getter for a given key already exists,
// the new value is added into its overrides.
//
// Returns undefined.
//
function setprops (phrase, keyvals)
{
    if (keyvals.length % 2 != 0)
        throw Error("Property setter given odd number of key/value elements.");

    for (var i = 0; i < keyvals.length; i += 2) {
        // Several keys may be given for a single prop, comma-separated.
        var pkeys = keyvals[i].split(",");
        var pval = keyvals[i + 1];

        // Set the value to each property key.
        for (var j = 0; j < pkeys.length; j += 1) {
            setprop(phrase, pkeys[j], pval);
        }
    }
}

// Manually set properties of the phrase given by the finalized msgstr
// in the PO file and signal fallback.
// For the rest of the behavior, see setprops()
function setprops_msgstrf (/*...*/)
{
    if (arguments.length % 2 != 0)
        throw Error("Msgstr property setter given odd number of arguments.");
    setprops(Ts.msgstrf(), arguments);
    throw Ts.fallback();
}
Ts.setcall("svojstva", setprops_msgstrf);
// "$[callname prop1 value1 prop2 value2 ...]"

// Manually set properties of the phrase given by the finalized msgstr
// in the PO file and return empty string.
// For the rest of the behavior, see setprops()
function setprops_msgstrf_e (/*...*/)
{
    if (arguments.length % 2 != 0)
        throw Error("Msgstr property setter given odd number of arguments.");
    setprops(Ts.msgstrf(), arguments);
    return "";
}
Ts.setcall("svojstva/p", setprops_msgstrf_e);
// "$[callname prop1 value1 prop2 value2 ...]"

// ------------------------------
// Manual plural handling.
// Only first three forms, as the fourth form is most likely not needed
// when the plural needs to be scripted.
// The first argument should be the proper value, not the substitution string
// (i.e. do not call as $[~ %1 ...] but as $[~ ^1 ...]).
function plural3 (n, form0, form1, form2)
{
    if (n % 10 == 1 && n % 100 != 11)
        return form0;
    else if (n % 10 >= 2 && n % 10 <= 4 && (n % 100 < 10 || n % 100 >= 20))
        return form1;
    else
        return form2;
}
Ts.setcall("množ", plural3);
// "...and %2 $[callname ^2 file fila files]"

// ------------------------------
// General choice-by-case.
function select_by_case (/* test, case1, choice1, ..., [default_choice] */)
{
    if (arguments.length < 1)
        throw Error("Choice by case takes at least the test value.");

    for (var i = 1; i < arguments.length - 1; i += 2) {
        if (arguments[0] == arguments[i]) {
            return arguments[i + 1];
        }
    }
    // No case matched, see if we have a default.
    if ((arguments.length - 1) % 2 != 0) {
        return arguments[arguments.length - 1];
    } else {
        throw Ts.fallback();
    }
}
Ts.setcall("kada", select_by_case);
// "Do you want to %1 $[callname %1 open 'this bar' access 'thisu baru']"

// ------------------------------
// Select one of three forms according to the gender of the phrase.
function select_by_gender (phrase, form_m, form_f, form_n)
{
    // Select gender (throws fallback if phrase not found).
    var gender = getprop(phrase, "_rod");

    if (gender == "m") {
        return form_m;
    }
    else if (gender == "ž") {
        return form_f;
    }
    else if (gender == "s") {
        return form_n;
    }
    else {
        throw Ts.fallback();
    }
}
Ts.setcall("po-rodu", select_by_gender);
// "Delete $[callname %1 this thisa thiso] %1?"

// ------------------------------
// Select one of two forms according to the number of the phrase.
function select_by_number (phrase, form_s, form_p)
{
    // Select number (default to singular if not found).
    var number = "j";
    if (hasprop(phrase, "_broj")) {
        number = getprop(phrase, "_broj");
    }

    if (number == "k") { // plural
        return form_p;
    } else {
        return form_s;
    }
}
Ts.setcall("po-broju", select_by_number);
// "%1 $[callname %1 waalks waalk] by the river."

// ------------------------------
// Select one of six forms according to the gender and number of the phrase.
function select_by_number_gender (phrase,
                                  form_ms, form_fs, form_ns, // singulars
                                  form_mp, form_fp, form_np) // plurals
{
    // Select number (default to singular if not found).
    var number = "j";
    if (hasprop(phrase, "_broj")) {
        number = getprop(phrase, "_broj");
    }

    if (number == "k") { // plural
        return select_by_gender(phrase, form_mp, form_fp, form_np);
    } else {
        return select_by_gender(phrase, form_ms, form_fs, form_ns);
    }
}
Ts.setcall("po-rodu-broju", select_by_number_gender);
// "Delete $[callname %1 this thisa thiso thees theesa theeso] %1?"

// ------------------------------
// Select one the form according to the case and gender of another argument.
function select_by_case_gender (gcase, gphrase, phrase) // plurals
{
    var gender = getprop(gphrase, "_rod");
    return getprop(phrase, gcase + "-" + gender);
}
Ts.setcall("po-padežu-rodu", select_by_case_gender);
// "Delete $[callname case %2 %1] [case %2]?"

// ------------------------------
// Object to query whether a character is one of expected letters.
letter_arr = (""
    + "abvgdđežzijklljmnnjoprstćufhcčdžšABVGDĐEŽZIJKLLJMNNJOPRSTĆUFHCČDŽŠ"
    + "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
).split("");
letter = {};
for (var i = 0; i < letter_arr.length; ++i) {
    letter[letter_arr[i]] = 1;
}

// ------------------------------
// Split phrase into words and intersections,
// where words are taken as contiguous segments of letters.
// The return value is a tuple of arrays of words and intersections.
// There is always one more of intersections than words, so that
// the original phrase can be reassembled as
// intrs[0] + words[0] + ... + intrs[n - 2] + words[n - 2] + intrs[n - 1].
function split_phrase (phrase)
{
    intrs = [];
    words = [];

    var i = 0;
    while (i < phrase.length) {
        var i1 = i;
        while (i1 < phrase.length && !letter[phrase[i1]]) {
            ++i1;
        }
        intrs.push(phrase.substring(i, i1));
        var i2 = i1;
        while (i2 < phrase.length && letter[phrase[i2]]) {
            ++i2;
        }
        if (i2 > i1) {
            words.push(phrase.substring(i1, i2));
            if (i2 == phrase.length) {
                intrs.push("");
            }
        }
        i = i2;
    }

    return [words, intrs];
}

// ------------------------------
// Apply call to each word in the phrase.
// The call must accept the word as the last argument.
function apply_to_word (/* phrase, callname, ...args before word... */)
{
    if (arguments.length < 2)
        throw Error("Applicator takes at least the phrase and the call name.");

    phrase = arguments[0];
    calln = arguments[1];
    cargs = [calln];
    for (var i = 2; i < arguments.length; ++i) {
        cargs.push(arguments[i]);
    }

    lst = split_phrase(phrase);
    words = lst[0];
    intrs = lst[1];

    nwords = [];
    for (var i = 0; i < words.length; ++i) {
        cargs.push(words[i]);
        nwords.push(Ts.acall.apply(Ts, cargs));
        cargs.pop();
    }

    str = ""
    for (var i = 0; i < nwords.length; ++i) {
        str += intrs[i] + nwords[i];
    }
    str += intrs[nwords.length];

    return str;
}
Ts.setcall("na-riječ", apply_to_word);
// "Repeat until $[callname casecall %1], and on this date..."

// ------------------------------
// Decline person's name into given case.
// Parse name into first and last name, determine gender according to
// first name, decline according to gender and assemble.
// If name cannot be fully declined, returns original name if retself is true,
// otherwise signals fallback.
// TODO: Just delegates to ordinary getters for the time being.
function decline_person_name_base (gcase, fullname, retself)
{
    if (retself) {
        return getform(fullname, gcase);
    }
    else {
        return getprop(fullname, gcase);
    }
}

// Decline person's name, signal fallback if not possible.
function decline_person_name (gcase, fullname)
{
    return decline_person_name_base(gcase, fullname, false);
}
Ts.setcall("imenski", decline_person_name);
// "You have invited $[callname case %1] to the party."

// Decline person's name, return as-is if not possible.
function decline_person_name_nf (gcase, fullname)
{
    return decline_person_name_base(gcase, fullname, true);
}
Ts.setcall("imenski/f", decline_person_name_nf);
// "You have invited $[callname case %1] to the party."

// ------------------------------
// Match style attributes to gender of the font family name,
// for the requested grammatical case.
// The message must have dynamic context 'family' equal to the family name,
// so that its gender can be obtained.
// Style string may be composed of several space-separated attributes.
// Family name and style attributes are expected in the nominative case.
// Returns composed style string in the proper gender/case.
function match_style_to_font (compstyle, gcase)
{
    var family = Ts.dynctxt("family");
    if (!family) {
        throw Ts.fallback();
    }
    var gender = getprop(family, "_rod");
    var number = ""
    if (hasprop(family, "_broj")) {
        number = getprop(family, "_broj");
    }
    var styles = compstyle.split(" ");
    var final = "";
    for (var i = 0; i < styles.length; i += 1) {
        final += " " + getprop(styles[i], gcase + "-" + gender + number);
    }
    return final.substr(1); // to remove initial space
}
Ts.setcall("stil-prema-fontu", match_style_to_font);

// ------------------------------
// Pick a phrase depending on a dynamic context field.
// Input is the keyword of the context field, followed by pairs of
// regex matcher on context value and corresponding phrase,
// and optionally followed by default phrase in case the value does not match.
// If the value does not match and default phrase is not given,
// fallback is signaled.
// If requested dynamic context field does not exist, fallback is signaled.
function select_by_context (/* ctxt_key,
                               valrx_1, phrase_1, ..., valrx_N, phrase_N
                               [, default_phrase]*/)
{
    if (arguments.length < 1)
        throw Error("Selector by context takes at least the context keyword.");

    var ctxtkey = arguments[0];
    var ctxtval = Ts.dynctxt(ctxtkey);

    var phrase;
    for (var i = 1; i < arguments.length; i += 2) {
        if (ctxtval.match(RegExp(arguments[i]))) {
            phrase = arguments[i + 1];
            break;
        }
    }
    if (phrase == undefined) {
        if (arguments.length % 2 == 0) {
            phrase = arguments[arguments.length - 1];
        } else {
            throw Ts.fallback();
        }
    }

    return phrase;
}
Ts.setcall("po-kontekstu", select_by_context);

// ------------------------------
// Load property maps.
Ts.loadProps("trapnakron");
// // Do not load fonts pmap if the user requested so.
// if (!Ts.getConfBool("translate-fonts", true)) {
// }
