// Følgjande funksjonar er for å få rette klokkeslett i den uklare klokka.
//
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
Ts.setcall("eigenskap", setMsgstrProps);
// NOTE: You can replace "properties" in the line above with any UTF-8 string,
// e.g. one in your language so that it blends nicely inside POs.



// Nokre generelle tekstfunksjonar.

// Til store bokstavar
function rop (str) {
      return str.toUpperCase();
}

// Til små bokstavar
function kviskra (str) {
      return str.toLowerCase();
}

// Til liten forbokstav
function litenfor (str) {
      return Ts.toLowerFirst(str);
}

// Til stor forbokstav
function storfor (str) {
      return Ts.toUpperFirst(str);
}

// Transcript-integrering for tekstfunksjonar
Ts.setcall("rop", rop);
Ts.setcall("kviskra", kviskra);
Ts.setcall("litenfor", litenfor);
Ts.setcall("storfor", storfor);


// Handtering av namn på skjermelement
Ts.loadProps("skjermelement");

// Formatering av namn på skjermelement
// Bunden form av namnet.
function bunden(str) {
      var svar = Ts.getProp(str, "bunden");
      if (!svar) {
        throw Ts.fallback();
      }
      return svar;
}

// Tilhøyrande pronomen (denne, dette, desse)
function pron(str) {
      var svar = Ts.getProp(str,"pron");
      if (!svar) {
        throw Ts.fallback();
      }
      return svar;
}

// Ev. dobbeltbestemming («Set opp *den* uklare klokka»),
// men ikkje gje feil om «dobbelbest» ikkje er definert.
function dobbelbest(str) {
      var svar = Ts.getProp(str,"dobbelbest");
      if (!svar)
        return "";
      else
        return svar+" ";
}

Ts.setcall("bunden", bunden);
Ts.setcall("pron", pron);
Ts.setcall("dobbelbest", dobbelbest);

