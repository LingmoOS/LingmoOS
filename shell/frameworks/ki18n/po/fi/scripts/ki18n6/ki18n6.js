function commonNameToLowerFirst(phr) {
    var str = Ts.subs(0);
    var pval = Ts.getProp(str, "yleisnimi");

    if (pval == "kyllä") {
        return Ts.toLowerFirst(phr);
    }
    else {
        return phr;
    }
}

Ts.setcall("yleisnimi_pienellä", commonNameToLowerFirst);

// Converts the first character in the string to lower case
function toLowerFirst(str)
{
    return Ts.toLowerFirst(str);
}

Ts.setcall("pieni_alkukirjain", toLowerFirst);

function conditionalHyphen(str1, str2)
{
    var spaceBeforeHyphen = false;
    if (str1.indexOf(' ') != -1) {
        spaceBeforeHyphen = true;
        // Simple logic for detecting if str1 consists of only an HTML 'a' tag
        // In that case only check for user visible whitespaces inside the tag.
        // This is to prevent "foo -bar" behaviour when "foo-bar" is wanted.
        // This obviously catches only the most basic cases.
        // Also it is assumed that this kind of link string is never used as
        // the first parameter to this function if it is meant to be displayed
        // as plain text (i.e. not as a link).
        if (str1.length > 4 && str1[0] == '<' && str1[1] == 'a' && str1[2] == ' ') {
            var tagEnd = str1.indexOf('>', 3);
            if (tagEnd != -1) {
                var textStart = tagEnd + 1;
                var endTagStart = str1.indexOf('<', textStart);
                if (endTagStart != -1 && str1.length == endTagStart+4 &&
                    str1[endTagStart+1] == '/' &&
                    str1[endTagStart+2] == 'a' && str1[endTagStart+3] == '>')
                {
                    str1Text = str1.substring(textStart, endTagStart-1);
                    if (str1Text.indexOf(' ') != -1) {
                        spaceBeforeHyphen = true;
                    }
                    else {
                        spaceBeforeHyphen = false;
                    }
                }
            }
        }
    }

    if (spaceBeforeHyphen) {
        return str1 + " -" + str2;
    } else {
        return str1 + "-" + str2;
    }
}

Ts.setcall("yhdysmerkki", conditionalHyphen);


// Set properties of the phrase given by the finalized msgstr in the PO file.
// The arguments to the call are consecutive pairs of keys and values,
// as many as needed (i.e. total number of arguments must be even).
//
// The property keys are registered as PO calls taking single argument,
// which can be used to retrive the property values for this msgstr
// when it is later used as placeholder replacement in another message.
//
// Always signals fallback.
//
function setMsgstrProps (/*KEY1, VALUE1, ...*/)
{
    if (arguments.length % 2 != 0)
        throw Error("Property setter given odd number of arguments.");
 
    // Collect finalized msgstr.
    phrase = Ts.msgstrf()
 
    // Go through all key-value pairs.
    for (var i = 0; i < arguments.length; i += 2) {
        var pkey = arguments[i];
        var pval = arguments[i + 1];
 
        // Set the value of the property for this phrase.
        Ts.setProp(phrase, pkey, pval);
 
        // Set the PO call for getting this property, if not already set.
        if (!Ts.hascall(pkey)) {
            Ts.setcall(pkey,
                       function (phr) { return Ts.getProp(phr, this.pkey) },
                       {"pkey" : pkey});
        }
    }
 
    throw Ts.fallback();
}
Ts.setcall("aseta", setMsgstrProps);

// NOTE: You can replace "aseta" in the line above with any UTF-8 string,
// e.g. one in your language so that it blends nicely inside POs.

// The following things are copied from the croation kdelibs4.js and used for
// noun cases

// ------------------------------
// Create a scripting call linked to property key in pmaps.
// If the call name starts with lowercase letter,
// another call with the first letter in uppercase will be defined,
// which will upcase the first letter in the property value before
// returning it.
function create_pgetter (cname, pkey)
{
    if (!Ts.hascall(cname)) {
        Ts.setcall(cname,
            function (phr) {
                if (this.pkey.constructor == Array) {
                    for (var i = 0; i < this.pkey.length; ++i) {
                        var pval = Ts.getProp(phr, this.pkey[i]);
                        if (pval != undefined) {
                            return pval;
                        }
                    }
                    return undefined;
                } else {
                    return Ts.getProp(phr, this.pkey);
                }
            },
            {"pkey" : pkey});

        cname_uc = Ts.toUpperFirst(cname);
        if (cname_uc != cname) {
            Ts.setcall(cname_uc,
                function (phr) {
                    return Ts.toUpperFirst(Ts.acall(this.cname_lc, phr));
                },
                {"cname_lc" : cname});
        }
    }
}

// ------------------------------
// Predefined property getters.
// Call names with corresponding pmap keys for predefined getters.
// The first letter in the call name should be in lowercase;
// see the comment to create_pgetter() function for the reason.
var call_name_to_prop = {
    "nom" : "nom", // nominative case // is this really needed?
    "gen" : "gen", // genitive case
    "part" : "part", // partitive case
    "elat" : "elat", // elative case
    "adess" : "adess", // adessive case
    "iness": "iness", // inessive case
    "illat" : "illat", // illative case
    "hakumuoto" : "hakumuoto",
    "teonnimi" : "teonnimi",
//    "lok" : ["lok", "dat"], // locative case (forwarded to dative if missing)
// commented and left here for the purpose of example
};
for (cname in call_name_to_prop) {
    create_pgetter(cname, call_name_to_prop[cname]);
}

// ------------------------------
// Property maps to be available to all apps.
Ts.loadProps("general");
