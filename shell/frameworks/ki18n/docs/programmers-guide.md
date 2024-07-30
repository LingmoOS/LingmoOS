Programmer's Guide   {#prg_guide}
==================

-   [Introduction](#intro)
-   [Writing Messages](#write_i18n)
    -   [General Messages](#gen_usage)
    -   [Specialized Messages](#spec_usage)
    -   [Placeholder Substitution](#subs_notes)
    -   [Writing Good Texts](#good_text)
    -   [Writing Good Contexts](#good_ctxt)
    -   [Extraction-Only Macros](#i18n_noop)
    -   [Messages before creation of Q*Application instance](#before_qapp)
-   [Connecting Calls to Catalogs](#link_cat)
    -   [Connecting to Catalogs in Application Code](#link_prog)
    -   [Connecting to Catalogs in Library Code](#link_lib)
    -   [Connecting to Catalogs in Non-Code Files](#link_noncode)
-   [Handling Catalog Files](#handle_cat)
    -   [Extracting Templates](#handle_extract)
    -   [Placing and Installing Catalogs](#handle_install)
    -   [Updating Catalogs](#handle_update)
-   [Semantic Markup](#kuit_markup)
    -   [Defining Tags](#kuit_def_tags)
    -   [Selecting Visual Format](#kuit_sel_fmt)
    -   [Escaping](#kuit_escape)
    -   [Predefined Tags](#kuit_tags)
    -   [Predefined Entities](#kuit_ents)
-   [Localizing Non-Text Resources](#non_text)
-   [Further References](#refs)

<a name="intro">

## Introduction

*Internationalization* (i18n) is the process of preparing
the source code such that the running program can receive and
output user-visible information in various human languages, or,
more precisely, *locales*. This is because the same language may
be spoken at several places which differ in other details of relevance
to the software internationalization (e.g. dialect, calendar, etc).
I18n is performed by the programmer.
*Localization* (l10n) is the process of adapting relevant
program resources for a particular locale.
L10n is performed by the translator, who is usually a native of the locale.

User interface text is the most prominent resource needing l10n.
In the source code, there are two general approaches
to writing i18n'd user-visible strings, hereafter called *messages*.
In the first approach, the programmer writes symbolic identifier in place
of actual message text, and then for each language there is a file,
called translation *catalog*, which contains the actual text linked
to the appropriate identifier.
For example, in the code:

~~~
QString msg = getId("out_of_disk_space");
~~~

and in the translation catalog:

~~~
out_of_disk_space "There is no space left on the disk."
~~~

In the second approach, the programmer writes actual text inside
the source code, in an agreed upon human language,
which is usually American English. Then there is a translation catalog
for every *other* language, in which English and translated text of
the message are linked. For example, in the code:

~~~
QString msg = i18n("There is no space left on the disk.");
~~~

and in the translation catalog:

~~~
msgid "There is no space left on the disk."
msgstr "Nema više prostora na disku."
~~~

One may observe that the second approach is technically a generalization
of the first approach. However, in actual use there are non-negligible
differences in the i18n/l10n workflow between these approaches,
and therefore they are treated as qualitatively different.

Both approaches -- the symbolic-identifier and the actual-text -- have
some advantages and disadvantages over each other. That is why today
both are in use, in software i18n in general. Within free software,
though, the actual-text approach is considerably more widespread,
as embodied in the GNU Gettext i18n system. Whether this is a historical
accident, or due to sensibilities of people writing free software,
is left to the interested reader to research.

The KDE Ki18n library, being built atop Gettext, provides
the actual-text approach. In the following it is described
how to use Ki18n, from the viewpoint of programmers and maintainers.
Basic instructions are split into three parts:

\li The first part deals with how to write i18n messages in the code.
    This is the largest and the most important part, since there are
    many nuances to preparing quality messages for translation.
    Section: \ref write_i18n.

\li The second part describes how to connect particular translation calls
    with particular translation catalogs, so that the system can know
    in which catalogs to search for translations.
    In particular, there are some differences here depending on whether
    the piece of code is an application or a library.
    Section: \ref link_cat.

\li The third part explains how to *extract* messages from the code,
    in order to create the translation *template*.
    The template is simply a clean translation catalog (having only English
    part of each message), which the translator copies over and fills in
    the translations for a particular language.
    Then it is shown how and where to install translation catalogs,
    and how to update existing translation catalogs to newly extracted
    templates (after some new development has been done, causing
    new messages to appear, some to be removed, and some modified).
    Unlike in the GNU Gettext manual, here there are no dependencies or
    references to a particular software build system;
    this is left to the maintainer to choose.
    Section: \ref handle_cat.

There are also two special topics:

\li Some programmers like to have more structure and consistency
    in message texts, and for them Ki18n provides
    a customizable semantic markup.
    Section: \ref kuit_markup.

\li Sometimes there are program resources other than text
    that require localization.
    Ki18n provides a generic, though rudimentary solution for such cases.
    Section: \ref non_text.


<a name="write_i18n">

## Writing Messages

Most messages can be internationalized with simple `i18n*` calls,
which are described in the section \ref gen_usage.
A few messages may require treatment with `ki18n*` calls,
and when this is needed is described in the section \ref spec_usage.
Argument substitution in messages is performed using the familiar
Qt syntax `%<number>`, but there may be some differences;
the section \ref subs_notes gives some notes about placeholders.
Finally, guidelines for writing good messages are given
in sections \ref good_text and \ref good_ctxt.

<a name="gen_usage">

### General Messages

General messages are wrapped with `i18n*` calls.
These calls are *immediate*, which means that they return
the final localized text (including substituted arguments)
as a `QString` object, that can be passed to UI widgets.

The most frequent message type, a simple text without any arguments,
is handled like this:

~~~
QString msg = i18n("Just plain info.");
~~~

The message text may contain arbitrary Unicode characters,
and the source file *must* be UTF-8 encoded.
Ki18n supports no other character encoding.

If there are some arguments to be substituted into the message,
`%<number>` placeholders are put into the text at desired positions,
and arguments are listed after the string:

~~~
QString msg = i18n("%1 has scored %2", playerName, score);
~~~

Arguments can be of any type for which there exists an overloaded
`KLocalizedString::subs` method.
Up to 9 arguments can be inserted in this fashion, due to the fact that
`i18n` calls are realized as overloaded templates.
If more than 9 arguments are needed, which is extremely rare,
a `ki18n*` call (described later) must be used.

Sometimes a short message in English is ambiguous to translators,
possibly leading to a wrong translations.
Ambiguity can be resolved by providing a context string along the text,
using the `i18nc` call. In it, the first argument is the context,
which only the translator will see, and the second argument is the text
which the user will see:

~~~
QString msg = i18nc("player name - score", "%1 - %2", playerName, score);
~~~

Section \ref good_ctxt gives a few pointers on when contexts are needed,
and what they should contain.

In messages stating how many of some kind of objects there are,
where the number of objects is inserted at run time, it is necessary
to differentiate between *plural forms* of the text.
In English there are only two forms, one for number 1 (singular) and
another form for any other number (plural).
In other languages this might be more complicated (more than two forms),
or it might be simpler (same form for all numbers).
This is handled properly by using the `i18np` plural call:

~~~
QString msg = i18np("%1 image in album %2", "%1 images in album %2",
                    numImages, albumName);
~~~

The plural form is decided by the first integer-valued argument,
which is `numImages` in this example. In rare cases when there are
two or more integer arguments, they should be ordered carefully.

In QML code, due to some limitations, always the first argument
will be treated as plural-deciding, so it should be an appropriate
number (integer or a round double); otherwise, behavior is undefined.

It is also allowed to omit the plural-deciding placeholder, for example:

~~~
QString msg = i18np("One image in album %2", "%1 images in album %2",
                    numImages, albumName);
~~~

or even:

~~~
QString msg = i18np("One image in album %2", "More images in album %2",
                    numImages, albumName);
~~~

If the code context is such that the number is always greater than 1,
the plural call must be used nevertheless.
This is because in some languages there are different plural forms
for different classes of numbers; in particular, the singular form
may be used for numbers other than 1 (e.g. those ending in 1).

If a message needs both context and plural forms, this is provided by
`i18ncp` call:

~~~
QString msg = i18ncp("file on a person", "1 file", "%1 files", numFiles);
~~~


In the basic `i18n` call (no context, no plural) it is not allowed
to put a literal string as the first argument for substitution.
In debug mode this will even trigger a static assertion,
resulting in compilation error. This serves to prevent misnamed calls:
context or plural frequently needs to be added at a later point to
a basic call, and at that moment the programmer may forget to update
the call name from `i18n` to `i18nc/p`.

Furthermore, an empty string should never be wrapped with
a basic `i18n` call (no `i18n("")`),
because in translation catalog the message with empty text
has a special meaning, and is not intended for client use.
The behavior of `i18n("")` is undefined,
and there will be some warnings in debug mode.

There is also a complement of `i18nd*` calls
(`i18nd`, `i18ndc`, `i18ndp`, `i18ndcp`),
which are not supposed to be used directly,
but as will be explained in the section \ref link_cat.

<a name="spec_usage">

### Specialized Messages

There are some situations where `i18n*` calls are not sufficient,
or are not convenient enough.
One obvious case is if more than 9 arguments need to be substituted.
Another case is if it would be easier to substitute arguments later on,
after the line with the i18n call.
For cases such as these, `ki18n*` calls can be used.
These calls are *deferred*, which means that they do not return
the final translated text as `QString`, but instead return
a `KLocalizedString` instance which needs further treatment.
Arguments are then substituted one by one using
`KLocalizedString::subs` methods, and after all arguments
have been substituted, the translation is finalized by one of
`KLocalizedString::toString` methods (which return `QString`).
For example:

~~~
KLocalizedString ks;
case (reportSource) {
    SRC_ENG: ks = ki18n("Engineering reports: %1"); break;
    SRC_HEL: ks = ki18n("Helm reports: %1"); break;
    SRC_SON: ks = ki18n("Sonar reports: %1"); break;
    default: ks = ki18n("General report: %1");
}
QString msg = ks.subs(reportText).toString();
~~~


`subs` methods do not update the `KLocalizedString` instance on which
they are invoked, but return a copy of it with one argument slot filled.
This allows to use `KLocalizedString` instances as a templates
for constructing final texts, by supplying different arguments.

Another use for deferred calls is when special formatting of arguments
is needed, like requesting the field width or number of decimals.
`subs` methods can take these formatting parameters.
In particular, arguments should not be formatted in a custom way,
because `subs` methods will also take care of proper localization
(e.g. use either dot or comma as decimal separator in numbers, etc):

~~~
// BAD (number not localized):
QString msg = i18n("Rounds: %1", myNumberFormat(n, 8));
// Good:
QString msg = ki18n("Rounds: %1").subs(n, 8).toString();
~~~


Like with `i18n`, there are context, plural, and context-plural
variants of `ki18n:`

~~~
ki18nc("No function", "None").toString();
ki18np("File found", "%1 files found").subs(n).toString();
ki18ncp("Personal file", "One file", "%1 files").subs(n).toString();
~~~


`toString` methods can be used to override the global locale.
To override only the language of the locale, `toString` can take
a list of languages for which to look up translations
(ordered by decreasing priority):

~~~
QStringList myLanguages;
...
QString msg = ki18n("Welcome").toString(myLanguages);
~~~

The section \ref link_cat describes how to specify
the translation *domain*, a canonical name for the catalog file
from which `*i18n*` calls will draw translations.
But `toString` can always be used to override the domain for a given call,
by supplying a specific domain:

~~~
QString trName = ki18n("Georgia").toString("country-names");
~~~

Relevant here is the set of `ki18nd*` calls
(`ki18nd`, `ki18ndc`, `ki18ndp`, `ki18ndcp`),
which can be used for the same purpose,
but which are not intended to be used directly.
This will also be covered in the section \ref link_cat.

<a name="dyn_ctxt">

#### Dynamic Contexts

Translators are provided with the capability to script translations,
such that the text changes based on arguments supplied at run time.
For the most part, this feature is transparent to the programmer.
However, sometimes the programmer may help in this by providing
a *dynamic* context to the message, through
`KLocalizedString::inContext` methods.
Unlike the static context, the dynamic context changes at run time;
translators have the means to fetch it and use it to script
the translation properly. An example:

~~~
KLocalizedString ks = ki18nc("%1 is user name; may have "
                             "dynamic context gender=[male,female]",
                             "%1 went offline");
if (knownUsers.contains(user) && !knownUsers[user].gender.isEmpty()) {
    ks = ks.inContext("gender", knownUsers[user].gender);
}
QString msg = ks.subs(user).toString();
~~~

Any number of dynamic contexts, with different keys, can be added like this.
Normally every message with a dynamic context should also have
a static context, like in the previous example, informing the translator
of the available dynamic context keys and possible values.
Like `subs` methods, `inContext` does not modify the parent instance,
but returns a copy of it.

<a name="before_qapp">

### Messages before creation of Q*Application instance

Fetching the translated messages only works after the Q*Application
instance has been created. So any code which will be executed before
and which handles text that should be translated has to delay
the actual lookup in the catalog (like other locale-dependent actions),
in one of two ways:
either by using statically initialized character arrays wrapped in
`kli18n*` functions for extraction, and later making the
actual `i18n*` calls (see section \ref i18n_noop);
or by using `ki18n*` calls to create `KLocalizedString` instances,
which do not perform immediate catalog lookup, and later fetching
the actual translation through their toString() methods (see
section \ref spec_usage);

One reason for this is that Gettext follows the standard C/POSIX locale
settings. By standard on the start of a program all locale categories are
fixed to the "C" locale, including the locale for the message catalog
category (LC_MESSAGES).
To use instead the locales specified by the environment variables,
the locale values in the runtime need to be set to an empty string.
This is usually done in one go for all locale categories by this call:
~~~
setlocale(LC_ALL, "");
~~~
From this point on the locale specific environment variables
"LANGUAGE", "LC_*" and "LANG" are considered for deciding which locale
to use for which category. This includes Gettext when picking the
message catalog to use.

The constructor of QCoreApplication (and thus its subclasses) does a call
like that. Which means any `i18n*` calls done _after_ the creation of the
QCoreApplication instance (or of its subclasses) will use a message catalog
based on the locale specific environment variables, as one usually expects,
But any calls _before_ will use a message catalog for the "C" locale.

<a name="subs_notes">

### Placeholder Substitution

Hopefully, most of the time `%<number>` placeholders are substituted
in the way one would intuitively expect them to be.
Nevertheless, some details about substitution are as follows.

Placeholders are substituted in one pass, so there is no need
to worry about what will happen if one of the substituted arguments
contains a placeholder, and another argument is substituted after it.

All same-numbered placeholders are substituted with the same argument.

Placeholders directly index arguments: they should be numbered from 1
upwards, without gaps in the sequence, until each argument is indexed.
Otherwise, error marks will be inserted into message at run time
(when the code is compiled in debug mode), and any invalid placeholder
will be left unsubstituted.
The exception is the plural-deciding argument in plural calls,
where it is allowed to drop its placeholder,
in either the singular or the plural text.

If none of the arguments supplied to a plural call is integer-valued,
 an error mark will be inserted into the message at run time
 (when compiled in debug mode).

Integer arguments will be by default formatted as if they denote
an amount, according to locale rules (thousands separation, etc.)
But sometimes an integer is a numerical identifier (e.g. port number),
and then it should be manually converted into `QString` beforehand
to avoid treatment as amount:

~~~
i18n("Listening on port %1.", QString::number(port));
~~~


<a name="good_text">

### Writing Good Texts

When writing message texts, sometimes it is tempting to assemble text
from pieces such as to have less repetition.
However, such shortcuts frequently cannot work for other languages,
and are almost always confusing to translators.
The first rule of writing good message texts is therefore to
*keep sentences together and clearly structured*
(or "no word puzzles"), even at the cost of some repetition.

At its basic, this rule means always to use placeholders for insertion
of run time arguments, rather than string concatenation. For example:

~~~
// BAD:
i18n("File ") + filePath + i18n(" not found.");
// Good:
i18n("File %1 not found.", filePath);
~~~

This is rather obvious, since it also results in clearer and shorter code.
But sometimes placeholders can be overused:

~~~
// BAD:
i18n("This contact is now %1.",
     isOnline ? i18n("online") : i18n("offline"));
// Good:
isOnline ? i18n("This contact is now online.")
         : i18n("This contact is now offline.");
~~~

The shorter version here is bad, because the sentence structure of
translation may need to change by more than the one word,
and also because a less thorough translator may fail to check
in which way the short messages "online" and "offline" are used.

If an otherwise long text needs to differ in only small part,
then a reasonable solution is to split it at sentence level,
but also explain the situation to translators through context:

~~~
// BAD:
i18n("Something very long here. This contact is now %1.",
     isOnline ? i18n("online") : i18n("offline"));
// Good:
i18nc("%1 one of the 'This contact...' messages below",
      "Something very long here. %1",
      isOnline ? i18n("This contact is now online.")
               : i18n("This contact is now offline."));
// BAD:
  i18n("Something very long here. ")
+ isOnline ? i18n("This contact is now online.")
           : i18n("This contact is now offline.");
~~~

The third version above is bad because, firstly, the translator
may wonder about the trailing space in the first message or
simply overlook it, and secondly, there may be some cross-dependency
between the translation of the long message and the short messages.
In general, insertions of one message into another should *always*
be accompanied by contexts, and composition-significant
leading and trailing whitespace should be avoided.

The second basic rule of writing good texts is to
*expose every user-visible text for translation*.
One should *never* make assumptions of the type
"this does not need translation" or "it will be same in every language".

One example where programmers sometimes make such assumption
are compositions without any letters:

~~~
// BAD:
QString("%1: %2").arg(albumName, playCount);
// Good:
i18nc("album name: play count", "%1: %2", albumName, playCount)
~~~

Here, in some languages the arrangement of whitespace will need to differ
(e.g. a space may be needed before the colon as well).
Letter-free compositions should also normally be equipped with context,
because, for example, separation may depend on type of arguments.

Another example of user-visible texts sometimes wrongly omitted
from i18n are proper names:

~~~
// BAD:
label1->setText(i18n("Written by:"));
label2->setText("Roofus McBane");
// Good:
label1->setText(i18n("Written by:"));
label2->setText(i18n("Roofus McBane"));
~~~

Proper names too may need localization, for example, transliteration
when the target language uses a different writing system.
This holds for proper names of people, and of anything else.

When it comes to text markup, like the HTML subset supported by
Qt rich text processing, opinions are divided on how much of it
to expose for translation. One argument goes that markup may be
confusing for translators, and that exposing it increases
the chance of syntactical errors in translation.
This is true as such, but it should be balanced with situations where,
either, the translator needs to modify the markup,
or, the markup will convey some context to translator.
For example, typographical modifiers should always be left in:

~~~
// BAD:
label->setText("<b>" + i18n("Disk almost full:") + "</b>");
// Good:
label->setText(i18n("<b>Disk almost full:</b>"));
~~~

because the target language may have different typographical standards
(e.g. CJK languages tend to avoid boldface in text body font sizes,
as it makes ideographs harder to recognize).
Especially if tags are found around internal parts of the message text,
it would be ungainly to hide them, e.g. by placeholder insertion.
But, values to tag attributes, such as links in <tt>\<a href='...'></tt>,
tags, should be inserted through placeholders
(unless it is expected that translators provide localized values).
In this example:

~~~
// Good:
i18n("<p>Preprocessing has failed.</p>"
     "<p>Please check your bracketed images stack.</p>"
     "<p>Click \"Details\" to show processing details.</p>")
~~~

`<p>` tags could be avoided by splitting this message into
one message per sentence, but this should not be done, because
paragraphs should be translated as single units of meaning.

Another important point about XML-like text markup, is to try and keep it
well-formed in XML sense on the level of standalone message. For example:

~~~
// BAD (for translation, although fine by HTML / Qt rich text):
i18n("<p>Some sentence."
     "<p>Another sentence.");
// Good:
i18n("<p>Some sentence.</p>"
     "<p>Another sentence.</p>");
~~~

Well-formedness is good because the most frequent error in translation
in presence of markup is mistyping (or miscopying) a tag.
If the original text is well-formed, a translation checker tool
can require the same of translation, and signal when that is not so.
The previous example of non-well-formedness was perhaps trivial;
in practice, non-trivial examples usually break some other rules too
(e.g. no word puzzles).

<a name="good_ctxt">

### Writing Good Contexts

The message context, given as first argument in `*i18nc` calls,
is of great help to translators. Unfortunately, to a programmer it is
not always clear when a context is needed, or what it should state.
So, the very first rule of writing good contexts is to
*listen to the translators asking for contexts*.
When taking suggestions from translators, there is no need to worry if
the proposed context will be sufficient for "all" languages.
It is fine to simply add the information that a translator into
particular language requested, and wait for translators into other
languages to maybe request some other context information as well.

Having said this, some apriori guidelines on contexts can be followed.

Since in English the form of an adjective does not change based on
the gender of the noun it modifies, properly translating messages which
are single standalone adjectives will be impossible in many languages
without a context. So, in general, every message which is a standalone
adjective should have context:

~~~
i18nc("new file", "New");
i18nc("default action", "Default");
~~~


Lists of related items typically benefit from having the same context,
since they should all be translated in the same style:

~~~
i18nc("IPv4 configuration method", "Automatic (VPN)");
i18nc("IPv4 configuration method", "Automatic (VPN) addresses only");
i18nc("IPv4 configuration method", "Automatic (PPP)");
i18nc("IPv4 configuration method", "Automatic (PPP) addresses only");
...
~~~


When there are placeholders in the text for which it is not clear,
from the text alone, what kind of argument they represent,
this should be explained in the context:

~~~
i18nc("%1 old file path, %2 new file path",
      "Cannot move '%1' to '%2'.", oldPath, newPath);
i18nc("%1 track URL, %2 artist name",
      "%1 (by %2)", trackUrl, artistName);
~~~


It is frequently suggested to state in the context the grammar category
of the message text, such as "this is a verb" or "this is a noun".
Since the grammar category of the translation does not have to be
the same as that of the original, this kind of context provides
circumstantial information at best (see the section \ref uimark_ctxt
for what translators may use it to draw some real information about),
and is worthless at worst.
Also, due to relative absence of declension in English grammar,
different programmers may have different opinion on the grammar category:
the menu title "View", is it a verb or a noun?

<a name="uimark_ctxt">

#### User Interface Markers

In the same way there exists a HIG (Human Interface Guidelines) document
for the programmers to follow, translators should establish HIG-like
convention for their language concerning the forms of UI text.
Therefore, for a proper translation, the translator will need too know
not only what does the message mean, but also where it figures in the UI.
E.g. is the message a button label, a menu title, a tooltip, etc.

To this end a convention has been developed among KDE translators,
which programmers can use to succinctly describe UI usage of messages.
In this convention, the context string starts with an *UI marker*
of the form `@<major>:<minor>`, and may be followed by any other
usual context information, separated with a single space:

~~~
i18nc("@action:inmenu create new file", "New");
~~~


The major and minor component of the UI marker are not arbitrary,
but are drawn from the following table.
For each component, the superscript states the Ki18n release
when the component was introduced.
<table>
<tr><th>Major</th><th>Minor</th><th>Description</th></tr>
<tr><td><b>\@action</b><sup>5.0</sup></td><td></td>
    <td>Labels of clickable widgets which cause an action
        to be performed.</td></tr>
<tr><td></td><td>:button<sup>5.0</sup></td>
    <td>Push buttons in windows and dialogs.</td></tr>
<tr><td></td><td>:inmenu<sup>5.0</sup></td>
    <td>Menu entries that perform an action
        (as opposed e.g. to being checked).</td></tr>
<tr><td></td><td>:intoolbar<sup>5.0</sup></td>
    <td>Toolbar buttons.</td></tr>
<tr><td><b>\@title</b><sup>5.0</sup></td><td></td>
    <td>Text that is the title of a major UI element or
        a widget container.</td></tr>
<tr><td></td><td>:window<sup>5.0</sup></td>
    <td>Title of a window or a (dockable) view/pane.</td></tr>
<tr><td></td><td>:menu<sup>5.0</sup></td>
    <td>Menu title.</td></tr>
<tr><td></td><td>:tab<sup>5.0</sup></td>
    <td>Tab name.</td></tr>
<tr><td></td><td>:group<sup>5.0</sup></td>
    <td>Title to a group of widgets, like a group of checkboxes or
        radio buttons.</td>
<tr><td></td><td>:column<sup>5.0</sup></td>
    <td>Column name in a table header,
        e.g. in a table view widget.</td></tr>
<tr><td></td><td>:row<sup>5.0</sup></td>
    <td>Row name in a table.</td></tr>
<tr><td><b>\@option</b><sup>5.0</sup></td><td></td>
    <td>Labels of option selection widgets, which can be enable/disabled
        or selected between.</td></tr>
<tr><td></td><td>:check<sup>5.0</sup></td>
    <td>Checkbox label, also a checkable menu entry.</td></tr>
<tr><td></td><td>:radio<sup>5.0</sup></td>
    <td>Radio button label.</td></tr>
<tr><td><b>\@label</b><sup>5.0</sup></td><td></td>
    <td>Various widget labels which are not covered by any of
        \@action, \@title, or \@option.</td></tr>
<tr><td></td><td>:slider<sup>5.0</sup></td>
    <td>Slider label.</td></tr>
<tr><td></td><td>:spinbox<sup>5.0</sup></td>
    <td>Spinbox label.</td></tr>
<tr><td></td><td>:listbox<sup>5.0</sup></td>
    <td>Label to a list box or combo box.</td></tr>
<tr><td></td><td>:textbox<sup>5.0</sup></td>
    <td>Label to a text box or text edit field.</td></tr>
<tr><td></td><td>:chooser<sup>5.0</sup></td>
    <td>Label to any special-purpose chooser widget,
        like color chooser, font chooser, etc.</td></tr>
<tr><td><b>\@item</b><sup>5.0</sup></td><td></td>
    <td>Strings that are items from a range of possibilities or
        properties of a given type.</td></tr>
<tr><td></td><td>:inmenu<sup>5.0</sup></td>
    <td>Item presented in menu (e.g. sort ordering,
        encoding name, etc).</td></tr>
<tr><td></td><td>:inlistbox<sup>5.0</sup></td>
    <td>Item presented in a list or combo box.</td></tr>
<tr><td></td><td>:intable<sup>5.0</sup></td>
    <td>Item presented in a table cell.</td></tr>
<tr><td></td><td>:inrange<sup>5.0</sup></td>
    <td>End range labels, e.g. on sliders.</td></tr>
<tr><td></td><td>:intext<sup>5.0</sup></td>
    <td>Words and short phrases which are inserted into
        a larger piece of text.</td></tr>
<tr><td></td><td>:valuesuffix<sup>5.46</sup></td>
    <td>Suffix appended to a value, including any spacing
        (e.g. in a spinbox).</td></tr>
<tr><td><b>\@info</b><sup>5.0</sup></td><td></td>
    <td>Any transient information for the user.</td></tr>
<tr><td></td><td>:tooltip<sup>5.0</sup></td>
    <td>Expanded formulation of the widget's label,
        usually appearing automatically when the pointer
        hovers over the widget.</td></tr>
<tr><td></td><td>:whatsthis<sup>5.0</sup></td>
    <td>Longer description of a widget's purpose and behavior,
        usually manually called up by the user.</td></tr>
<tr><td></td><td>:placeholder<sup>5.46</sup></td>
    <td>A hint what input is expected in an input field,
        shown in the place of the input where there is none yet.</td></tr>
<tr><td></td><td>:status<sup>5.0</sup></td>
    <td>A piece of text displayed in application's status view
        (e.g in the status bar).</td></tr>
<tr><td></td><td>:progress<sup>5.0</sup></td>
    <td>Text describing the current step or state of an operation,
        possibly periodically updating.</td></tr>
<tr><td></td><td>:usagetip<sup>5.0</sup></td>
    <td>A tip that comes up to inform the user about
        a certain possibility in a given context,
        e.g. a "tip of the day" on application startup.<br/>
        Deprecated synonym: :tipoftheday.</td></tr>
<tr><td></td><td>:credit<sup>5.0</sup></td>
    <td>Contributor names and their contributions,
        e.g. in the about dialog.</td></tr>
<tr><td></td><td>:shell<sup>5.0</sup></td>
    <td>A note, warning or error sent to application's text output stream
        (stdout, stderr) rather than shown in the UI.</td></tr>
</table>
If none of the minor components apply to a given message,
a major component can be used standalone.
For example, this would happen with a library-provided list of items
without any immediate UI context (e.g. language or country names),
where the appropriate UI marker would be just `@item`.

One way to extend the context after the UI marker,
which is simple for the programmer yet can be very helpful for translators,
is simply to add the text of the (technically or logically) parent widget.
For example, if the action "New" is in the menu "File", then:

~~~
i18nc("@action:inmenu File", "New")
~~~

Or, if the item "Left" is found in the list box with
the label "Vertical alignment":

~~~
i18nc("@item:inlistbox Vertical alignment", "Left")
~~~


<a name="nocpp_ctxt">

#### Adding Contexts in Non-C++ files

When Qt Designer is used to build the user interface,
the \em -tr option of `uic` should be used to pass UI strings
through Ki18n's `tr2i18n` function (see also \ref link_ui).
This function is the equivalent of `i18n` or `i18nc`,
based on whether the second argument is null or not.
If a string in the `.ui` file has the attribute `comment=`,
its value will be automatically used as the context argument.
(In Qt Designer, this is the "disambiguation" property of a string.)
Alternatively, strings can be passed to Ki18n's `tr2xi18n` function
(see \ref kuit_markup).

In KXmlGui (`.rc`) and KConfigXT (`.kcfg`) files,
contexts can be added through `context=` attributes to
`text`, `label`, and `whatsthis` tags.

<a name="ctxt_extc">

#### Disambiguation Context vs. Extracted Comment

The GNU Gettext system actually defines *two* types of context
for translators. The type discussed so far, the first argument
of `*i18nc*` calls, is called *disambiguation context*.
The other type of context is *extracted comment*.
In Ki18n, this other type of context is written as a code comment,
in the line just preceding the message text,
and starting with *i18n:* keyword:

~~~
// i18n: This is a short text containing all letter of the alphabet,
// e.g. for testing fonts. Translate with a sentence which can
// serve the same purpose in your language.
i18n("The quick brown fox jumps over the lazy dog");
~~~


There are two main differences between disambiguation contexts and
extracted comments.

The first difference is that extracted comments
*do not separate messages* in the translation catalog.
For example, such two messages equipped with extracted comments:

~~~
// i18n: new file
QString msg1 = i18n("New");
// i18n: new folder
QString msg2 = i18n("New");
~~~

will show up in the translation catalog as a single message
with aggregate comments:

~~~
#. i18n: new file
#. i18n: new folder
msgid "New"
msgstr ""
~~~

The same two messages equipped with disambiguation contexts:

~~~
QString msg1 = i18nc("new file", "New");
QString msg2 = i18nc("new folder", "New");
~~~

will show up in the translation catalog as two messages:

~~~
msgctxt "new file"
msgid "New"
msgstr ""

msgctxt "new folder"
msgid "New"
msgstr ""
~~~


The second difference is that a change in extracted comment
*does not invalidate the existing translation*, i.e.
it will not force translators to revisit the message and revise
the translation (see the section \ref handle_update for details
on how this invalidation happens with disambiguation contexts).
Thus, for example, if there is a "message freeze"
before the next release of a piece of software, during which
programmers must not modify messages so that translators can
thoroughly complete their work, it is allowed to modify
the extracted comment, but it is not allowed to modify
the disambiguation context.

The Gettext manual suggests to use extracted comments as
the default way of providing context for translators,
and to use disambiguation contexts only when message separation
is necessary. However, we (the people in the KDE community)
suggest the opposite -- to use disambiguation context by default,
and extracted comments only in special cases.
This because there are two dangers associated with extracted comments:
one is that programmer may fail to properly judge when two messages
should be made separate for translation (and having to judge that
all the time is a burden in the first place), and the other is that
when the context change is such that translators really should revisit
the message, they would not get any automatic signal about that.
The message freeze advantage of extracted comments has not been
observed to be very important.

One special case when to an use extracted comment is when the context
is a multi-sentence text explaining the purpose of the message,
and the context is unlikely to change (in its meaning).
Another case is when the context lists a fixed set
of possible translations ("meta-messages", by which translators
influence some aspect of text formatting), which may expand in
the future, as new possibilities are introduced at request of
translators into new languages.

<a name="i18n_noop">

### Extraction-Only Functions

Sometimes it is convenient only to mark a message for extraction
(into the catalog template, as described in \ref handle_extract),
and to place the actual `i18n` call somewhere else.
A typical case of this are global static initializers,
where only POD types can be safely used.
For this purpose `kli18n*` functions are provided.

The `kli18n` function is the counterpart to `*i18n` calls,
and it is used like this:

~~~
typedef struct
{
    const KLazyLocalizedString description;
    const char *regExp;
} term;

static const term items[] = {
    {kli18n("any character"), "."},
    {kli18n("start of line"), "^"},
    {kli18n("end of line"), "$"},
    {kli18n("repeats, zero or more times"), "*"},
    ...
};

...

void populatePatternMenu (QMenu *menu)
{
    for (uint i = 0; i < sizeof(items) / sizeof(items[0]); i++) {
        menu->addAction(new RegExpAction(menu,
                                         // ...actual i18n call here.
                                         items[i].description.toString(),
                                         items[i].regExp));
    }
}
~~~


The `kli18nc` macro is the counterpart to `*i18nc` calls:

~~~
typedef struct
{
    const KLazyLocalizedString abbrev;
} unitDef;
static const unitDef units[] = {
    {kli18nc("unit for 2^10 bytes", "KiB")},
    {kli18nc("unit for 2^20 bytes", "MiB")},
    ...
}
...
    QString unitAbbrev = units[i].abbrev.toString();
~~~

Variants for singular/plural and with or without markup exist as well.
Unlike the `I18N_NOOP` macros they replace, mixing different variants
in the same message table is possible, e.g. to add a context to a few
messages when necessary.


<a name="link_cat">

## Connecting Calls to Catalogs

Every `i18n` call must look for translations in exactly
one translation catalog for a given language of translation.
For this purpose, a group of catalogs which have the same source text
but translations into different languages, is identified by
a unique canonical name, called the *domain*.
Therefore, every `i18n` call must be connected to a domain.
This connection is established differently for applications
and libraries, though the difference is only for convenience: if desired,
the more verbose library method can be used for application code as well.

<a name="link_prog">

### Connecting to Catalogs in Application Code

All `*i18n*` calls in an application can be connected
to a single domain by calling the static
`KLocalizedString::setApplicationDomain` method with
the domain as the argument:

~~~
#include <klocalizedstring.h>

...

int main (int argc, char *argv[])
{
    ...
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("fooapp"));
    ...
}
~~~

This call must be made in the code before any `i18n` call takes place,
and right after creating the instance of `QCoreApplication` or one
of its subclasses. `ki18n` calls can still be made before, but the
respective `KLocalizedString::toString()` has to be delayed to after that.

This is all there is to connecting calls and catalogs application's
C++ source files. However, there may also be some non-code files
that need connecting, and how to do this is some typical non-code
files is described in the section \ref link_noncode.

<a name="link_lib">

### Connecting to Catalogs in Library Code

`i18n` calls in libraries must be strictly connected to library's
own translation domain no matter how the library is used,
and this should be fully transparent to the library's client.
In particular, if the client does not use Ki18n for internationalization
of its own messages, library translation must still work as expected.
Therefore, in library code, the call-domain connection is established
in this way:

~~~
#define TRANSLATION_DOMAIN "foolib"
#include <klocalizedstring.h>

...

void some_library_function ()
{
    ...
    QString msg = i18n("Greetings from Foolib!");
    ...
}
~~~

The definition of `TRANSLATION_DOMAIN` triggers
the domain-specialization macro in `klocalizedstring.h`.
It routes all `*i18n*` calls to their `*i18nd*` counterparts,
which take the domain as their first argument.
The `i18n` call from this example will thus expand into:

~~~
QString msg = i18nd("foolib", "Greetings from Foolib!");
~~~


It is possible to use `*i18nd*` calls explicitly,
but there should be no need for that.
If there are any messages that should draw translations
from a special domain, it is better style-wise to use the
`ki18n("...").toString(domain)` construct.

Definition of `TRANSLATION_DOMAIN` can be put into
a private header file of the library, so that it does not have
to be repeated at multiple locations.
If there are some `i18n` calls in a *public* header file,
definition of `TRANSLATION_DOMAIN` would propagate
into and affect the application client code that uses Ki18n too.
This is prevented by adding the following lines somewhere
after the last `i18n` call in the public header:

~~~
#undef TRANSLATION_DOMAIN
#include <klocalizedstring.h>
~~~

This will undefine all expansions of `*i18n*` into `*i18nd*`,
leaving the client's environment clean.
If instead the public header contains only `kli18n*` function calls,
defining `TRANSLATION_DOMAIN` is unnecessary in the first place,
since actual `i18n` calls happen somewhere else.

<a name="link_noncode">

### Connecting to Catalogs in Non-Code Files

Both KDE applications and libraries can include some non-code files
which contain messages that need to be connected to a translation domain.
This can be the same domain where messages from C++ code are found,
or another domain, whatever seems more appropriate.
In principle, each type of non-code file requires its own connection
mechanism, and here it is explained how this works for
typical types of non-code files found in KDE sources.

It is assumed in the following that all messages in the non-code file
are connected to the single domain. In other words, the connection
is specified on the file level rather than on the message level.

<a name="link_ui">

#### Qt Designer (.ui) files

First, to have UI strings from `.ui` file passed through Ki18n,
`uic` is run with `-tr tr2i18n`. This will replace all
native Qt `tr` calls with Ki18n's `tr2i18n` calls
in the resulting header file.
Then, the generated header file needs to be post-processed
to fix empty messages and include `klocalizedstring.h`.
At this point, the `TRANSLATION_DOMAIN` can be defined just like
in static C++ files.

If CMake is used as the build system, a macro that performs
all of the above is provided (`ki18n_wrap_ui`).
Otherwise, one could use a shell snippet such as this:

~~~
domain=fooapp
uifile=fooconfpage.ui
uihfile=$uifile.h
uic -tr tr2i18n $uifile -o $uihfile
sed -i 's/tr2i18n("")/QString()/g' $uihfile
sed -i 's/tr2i18n("", "")/QString()/g' $uihfile
sed -i "1i\#define TRANSLATION_DOMAIN \"$domain\"\n#include <klocalizedstring.h>" $uihfile
~~~


If strings contain KUIT markup (section \ref kuit_markup),
`tr2i18n` in the lines above should be replaced with `tr2xi18n`.

<a name="link_rc">

#### KXmlGui (.rc) files

Since `.rc` files are interpreted at runtime,
the translation domain connection is established simply
by adding the `translationDomain` attribute to the top element:

~~~
<!DOCTYPE gui SYSTEM "kpartgui.dtd">
<gui name="foolib_part" version="55" translationDomain="foolib">
...
~~~

If the `.rc` file belongs to application rather than library source,
it is not necessary to set `translationDomain`. If not set,
translations will be looked up in the domain set with
`KLocalizedString::setApplicationDomain` call in the code.

If strings contain KUIT markup (section \ref kuit_markup),
additionally the attribute `translationMarkup="true"` should be set.

<a name="link_kcfg">

#### KConfigXT (.kcfg) files

Instructions for building the configuration code from a `.kcfg` file
are contained in the `.kcfgc` file of the same base name;
`kconfig_compiler` is invoked with both files as arguments.
Then, the domain connection is established simply by adding
the `TranslationSystem` and `TranslationDomain` fields in
the `.kcfgc` file, to select Ki18n as the translation system
and the appropriate translation domain:

~~~
File=foolib.kcfg
...
TranslationSystem=kde
TranslationDomain=foolib
~~~

If the `.kcfg` file is part of an application rather than a library,
the `TranslationDomain` field can be omitted in order
to have messages looked up in the domain set by
`KLocalizedString::setApplicationDomain` call in the code.

If strings contain KUIT markup (section \ref kuit_markup),
additionally the field `TranslationMarkup=true` should be set.

<a name="handle_cat">

## Handling Catalog Files

For translators to start working, one or more translation catalog files
should be prepared, based on the `i18n` calls in the source code.
The procedure to do this is called *extraction* of messages.
Extraction produces empty catalog files, called *templates*.
These files are in the PO format, and have `.pot` extension.
Section \ref handle_extract explains how to perform extraction.

Once templates are ready, a translators make copies of them,
with `.po` extension, and start filling them out with translations
into respective languages.
When translation is done, the translated catalog is committed
into the source code repository.
The build system is set up to install translated catalogs.
Section \ref handle_install provides necessary steps for this.

After some development has passed, the source repository will
contain many translated catalogs which are out of date with
respect to latest catalog templates.
Of course, translators do not have to start translating from scratch,
but there are specialized tools to carry over as much of existing
translation as possible, so that only new and modified texts
need to be considered.
Section \ref handle_update shows how this is done.

A usual application or a library has one translation catalog,
but there can be more if there is higher modularity of the source.
The following subsections refer to a single catalog wherever
the extension to case with multiple catalogs is obvious,
and mention multiple catalogs only where necessary.

<a name="handle_extract">

### Extracting Templates

The primary tool for collecting texts from `i18n` calls and
writing out the catalog template is `xgettext`,
from the official Gettext tools package.
`xgettext` supports many programming languages and sublanguage
environments, among which naturally C++ and Ki18n specifically.

The extraction process from source code files is thus simple:
`xgettext` runs with appropriate options over all files,
and it writes out the catalog template.
For the moment masking the complete list of options as `$EXTOPTS`,
`xgettext` can be run for example like this at the top of Fooapp
source to create the catalog template `fooapp.pot`:

~~~
find -name \*.cpp -o -name \*.h -o -name \*.qml | sort \
| xargs xgettext $EXTOPTS -o fooapp.pot
~~~

Or, a list of source files that should be extracted from can be
assembled separately and fed to `xgettext:`

~~~
# ...create sources.list...
xgettext $EXTOPTS -f sources.list -o fooapp.pot
~~~

One may want to assemble the list of source files by hand
or semi-automatically in order to prioritize the order of translation
(messages from most important files appearing first in the catalog),
to exclude some portions of the source tree from extraction, and so on.

`$EXTOPTS` that cover everything from Ki18n, and some generalities,
should look like this:

~~~
--c++ --kde \
--from-code=UTF-8 \
-c i18n \
-ki18n:1 -ki18nc:1c,2 -ki18np:1,2 -ki18ncp:1c,2,3 \
-kki18n:1 -kki18nc:1c,2 -kki18np:1,2 -kki18ncp:1c,2,3 \
-kkli18n:1 -kkli18nc:1c,2 -kkli18np:1,2 -kkli18ncp:1c,2,3 \
-kI18N_NOOP:1 -kI18NC_NOOP:1c,2 \
--copyright-holder=<author-of-original-text> \
--msgid-bugs-address=<where-to-report-errors-in-original-text>
~~~

`--c++ --kde` options tell `xgettext` that source files
are C++ with Ki18n. `--from-code=UTF-8` specifies the encoding
of source files to be UTF-8, which must be so for Ki18n.
`-c i18n` states that comments for extraction start with given
keyword (`// i18n: ...`).
The series of `-k` options informs `xgettext` of all possible
translation call names and which of their arguments to extract.
Finally, options `--copyright-holder` and
`--msgid-bugs-address` automatically write
the corresponding information into the catalog at proper place.
If there are semantic markup calls in the code (section \ref kuit_markup),
the following `-k` options should be added as well:

~~~
-kxi18n:1 -kxi18nc:1c,2 -kxi18np:1,2 -kxi18ncp:1c,2,3 \
-kkxi18n:1 -kkxi18nc:1c,2 -kkxi18np:1,2 -kkxi18ncp:1c,2,3 \
-kklxi18n:1 -kklxi18nc:1c,2 -kklxi18np:1,2 -kklxi18ncp:1c,2,3 \
~~~


`xgettext` unfortunately cannot be directly used to extract messages
from the usual XML files appearing in Qt and KDE sources --
Designer (`.ui`), KXmlGui (`.rc`) and KConfigXT (`.kcfg`) files.
Therefore the `kdesdk` package provides the `extractrc` script,
which extracts XML messages as dummy `i18n` calls into a dummy C++ file,
This file can then be included into the list of files for `xgettext` run.
The usual invocation of `extractrc` is:

~~~
find -name \*.ui -o -name \*.rc -o -name \*.kcfg | sort \
| extractrc > rc.cpp
# ...run xgettext with rc.cpp included in source files...
rm rc.cpp
~~~


If the catalog being extracted is an application catalog,
i.e. given as `KLocalizedString::setApplicationDomain` in the code,
it should contain two meta-messages for translation credits
which will be shown by `KAboutApplicationDialog`.
These messages are also written as dummy `i18n` calls,
usually into the same dummy C++ file with XML messages,
with the following context and text:

~~~
echo 'i18nc("NAME OF TRANSLATORS", "Your names");' >> rc.cpp
echo 'i18nc("EMAIL OF TRANSLATORS", "Your emails");' >> rc.cpp
~~~


The extraction command sequence can be written down as a small script,
to be run periodically by the maintainer,
or it can be integrated into the build system.

For the code residing in the official KDE repositories
a special form of the extraction script is mandated.
This enables automatic overnight template extraction and feeding
into dedicated translation section of KDE repositories.
Details can be found at
<a href="http://techbase.kde.org/Development/Tutorials/Localization/i18n_Build_Systems">KDE Techbase</a>.

<a name="handle_install">

### Placing and Installing Catalogs

For an application or a library which uses a single catalog,
the usual organization of catalogs in the source tree is this:

~~~
fooapp/
    src/
    doc/
    po/
        fooapp.pot
        aa.po
        bb.po
        cc.po
        ...
~~~

Here translated catalog files are named by their language codes
(`aa`, `bb`, `cc`...).
In case of multiple catalogs, one directory per catalog can be created
under the `po/` directory:

~~~
fooapp/
    src/
    lib
    doc/
    po/
        fooapp/
            fooapp.pot
            aa.po
            bb.po
            ...
        foolib/
            foolib.pot
            aa.po
            bb.po
            ...
~~~

An alternative organization is to have one directory per language,
and name catalog files by the translation domain.
In multiple catalog situation this would look like:

~~~
fooapp/
    src/
    lib
    doc/
    po/
        templates/
            fooapp.pot
            foolib.pot
        aa/
            fooapp.po
            foolib.po
        bb/
            fooapp.po
            foolib.po
        ...
~~~


Catalog templates are fully derived files, and therefore
some maintainers do not like to keep them inside the repository.
In that case, at least the tarball should contain the templates
(i.e. they should be generated at packaging time),
so that translators have somewhere to get them from.
Another possibility is to upload the template to
a *translation hub* (such as
<a href="https://www.transifex.com/">Transifex</a>),
which translators can use to upload translated catalogs back,
and usually for some other features as well (assignment, review, etc).

If the code resides in an official KDE repository, neither templates
nor translated catalogs are kept inside the source tree.
Instead, translated catalogs are fetched from an appropriate place
when tarball is made, using a script provided for that purpose.
Details can be found at
<a href="http://techbase.kde.org/Development/Tutorials/Localization/i18n_Build_Systems">KDE Techbase</a>.

No matter how the catalog files are named and organized inside
the distribution tarball, they must be installed in exactly one way.
The base name of the installed catalog must be their translation domain,
and if the package is installed into `$PREFIX`, the installed directory
tree must look like as follows:

~~~
$PREFIX/
    share/
        locales/
            aa/
                LC_MESSAGES/
                    fooapp.mo
                    foolib.mo
            bb/
                LC_MESSAGES/
                    fooapp.mo
                    foolib.mo
            ...
~~~

Given that these directories are shared with other packages
in the same prefix, by Gettext convention translation domains
must be unique (like package names are).

MO files are the compiled version of PO files, and they are produced
using Gettext's `msgfmt` command:

~~~
msgfmt $SRCDIR/po/fooapp/aa.po -o $BUILDDIR/mo/aa/fooapp.mo
~~~

Compilation and installation of catalogs should naturally be integrated
into the build system. In case <a href="http://www.cmake.org/">CMake</a>
is used, KDE provides CMake macros for this purpose.

<a name="handle_transcript">

#### Placing and Installing Scripting Modules

Since Ki18n provides a run-time scripting capability for translators,
some translators may also write the scripting module corresponding
to the catalog.
A scripting module is a directory named like the translation domain,
and at least one JavaScript file inside, also with the same base name
as the translation domain.

Scripting modules can be placed like this in the source tree:

~~~
fooapp/
    po/
        fooapp.pot
        aa.po
        aa/
            fooapp.js
            ...
        bb.po
        ...
~~~

or in the per-language directory variant:

~~~
fooapp/
    po/
        templates/
            fooapp.pot
        aa/
            fooapp.po
            fooapp/
                fooapp.js
                ...
        bb/
            fooapp.po
        ...
~~~


The installation location for scripting modules is like that for catalogs,
only using `LC_SCRIPTS/` directory instead of `LC_MESSAGES/`:

~~~
$PREFIX/
    share/
        locales/
            aa/
                LC_SCRIPTS/
                    fooapp/
                        fooapp.js
                        ...
~~~


When a translator inquires about adding a scripting module,
or sends one in, the maintainer should check with the translator
if perhaps the functions provided by the module are more widely
applicable. If that is the case, they should rather become part of
Ki18n's own scripting module, because then they will be accessible
to all Ki18n-based translations in the given language.

<a name="handle_update">

### Updating Catalogs

When new catalog template is extracted after some development
has been done, existing translation should be updated against it.
This is called *merging* with template, and it is performed
by Gettext's `msgmerge` command.
There are some merging options that can be examined here,
but generally the best invocation of `msgmerge` is this:

~~~
msgmerge --update --backup=none --previous aa.po fooapp.pot
~~~

Options `--update --backup=none` mean to update the catalog
in place, and not to make a backup file. Option `--previous`
puts some additional information into every modified message,
that translation editing tools can use to show to the translator
all changes in the text.
This command should be run once for every existing translation catalog.

One thing to keep in mind is that a change in the context string
of a message in the code (i.e. the first argument to `*i18nc*` calls),
including adding or removing one, will also register as
a modified message in the merged catalog.
This will require that translators revisit it, which is exactly as
intended: if the context has changed, especially if it was added,
some changes in translation may be needed.
However, this means that when a "message freeze" is declared so that
translators can complete updating translations without disruption,
contexts fall under same rules as text.

There are a few possibilities for who and when should perform merging.
For example, the maintainer can write a script that at the same time
extracts the template and merges all catalogs, and run it periodically,
committing updated catalogs to the repository for translators to pick up.
This could even be integrated into the build system.
Some maintainers do not like committing automatic changes,
and instead expect translators to run the extraction-merging script
for the language they maintain, update the translation,
and commit only that updated catalog. This solution is cleaner with
respect to repository history, but it may burden translators.

When operating in an official KDE repository, maintainers do not
have to deal with merging at all. The server-side automation which
automatically extracts templates and provides them to translators,
also performs merging. So the maintainer is left only to pick up
whatever are the latest catalogs when making a tarball.


<a name="kuit_markup">

## Semantic Markup

When composing user-interface text, some programmers ponder about
the typographical conventions to use in certain contexts.
For example, when a file name is inserted into the text,
some typographical solutions that can be used are:

~~~
i18n("Cannot open %1 for reading.", filePath);
i18n("Cannot open '%1' for reading.", filePath);
i18n("Cannot open \"%1\" for reading.", filePath);
~~~

For the Qt widgets that have rich text capability,
exposed as subset of HTML tags, additional solutions include:

~~~
i18n("Cannot open <b>%1</b> for reading.", filePath);
i18n("Cannot open <i>%1</i> for reading.", filePath);
i18n("Cannot open <tt>%1</tt> for reading.", filePath);
~~~

The problem here is not so much to decide on one solution, as it is
to follow it consistently through time and between contributors.
One may also want to use two solutions, one in places where only
plain text is allowed, and another where rich text is available.
Wouldn't it then be easier to write everywhere:

~~~
xi18n("Cannot open <filename>%1</filename> for reading.", filePath);
~~~

and have it automatically resolve into plain or rich text according to
the UI context, using formatting patterns defined at one place?
This approach is called *semantic* markup, because the author
marks parts of the text according to what they *represent*.

Ki18n implements such a semantic markup,
called KUIT (KDE User Interface Text).
It is accessed through the series of `xi18n*` and `kxi18n*` calls,
which are the KUIT-aware counterparts of `i18n*` and `ki18n*` calls.
Ordinary and KUIT-aware calls can be freely mixed within
a given body of code.

KUIT defines a number of semantic tags that are frequently of use,
as listed in the section \ref kuit_tags.
But, KUIT also allows the programmer to define custom tags,
as well as to change visual formatting patterns for predefined tags.
This capability should lessen one important issue of semantic markups:
when the author is forced to choose between several tags
none of which exactly fits the desired meaning;
with KUIT, the author can simply define a custom tag in that case.

<a name="kuit_def_tags">

### Defining Tags

Tags are defined and redefined per translation domain, so that changes
will not affect markup resolution in any other domain.
Changes are performed through the `KuitSetup` object associated
with the domain, as returned by the `Kuit::setupForDomain` method.
A tag is defined (or redefined) by defining (or redefining)
its formatting patterns, with one call to `KuitSetup::setTagPattern`
for each desired combination of tag name, attribute names,
and visual format.
Here is an example of defining the tag `<player>`,
which has an optional attribute `color=`,
on the domain `foogame`:

~~~
KuitSetup *ks = Kuit::setupForDomain("foogame");
QString tagName;
QStringList attribNames;

tagName = "player";
attribNames.clear()
ks->setTagPattern(tagName, attribNames, Kuit::PlainText,
                  ki18nc("tag-format-pattern <player> plain",
                         "'%1'"));
ks->setTagPattern(tagName, attribNames, Kuit::RichText,
                  ki18nc("tag-format-pattern <player> rich",
                         "<b>%1</b>"));
attribNames.append("color");
ks->setTagPattern(tagName, attribNames, Kuit::RichText,
                  ki18nc("tag-format-pattern <player color= > rich",
                         "<font color='%2'><b>%1</b></font>"));
~~~

The first two `setTagPattern` calls set up resolution of
`<player>` without attributes, into plain and rich text.
The third call sets up resolution for `<player color="...">`,
but only into rich text; since a plain text pattern is not defined
for this tag-attribute combination, it will fall back to basic
`<player>` plain text pattern.
A fallback is always defined, the elementary fallback being a no-op,
where tag is simply removed.
Formatting patterns must be wrapped for translation too,
since translators may need to tweak them;
*ordinary* (not markup-aware) `ki18nc` calls must be used here,
since patterns themselves are not KUIT markup.
The `%1` placeholder in the pattern will be replaced by
the text wrapped with the tag, and `%2` and upwards with
attribute values, in the order of appearance in attribute names list.

If a simple substitution pattern is insufficient for formatting,
additionally a formatting function of type `Kuit::TagFormatter`
can be given. The result of this function is substituted into the pattern;
alternatively an empty pattern can be given (as `KLocalizedString()`),
in which case the result is used directly, no substitution is performed.
The formatting function also receives the current element path,
so that the resolution can depend on the markup tree context if needed.
Anything in the function that may need translator input
should be appropriately exposed through `i18nc*` or `ki18nc*` calls.

In the section \ref kuit_tags it is stated that every KUIT tag
is classified either as a phrase tag or as a structuring tag,
and explained what that means for processing.
A newly defined tag is by default a phrase tag;
method `KuitSetup::setTagClass` can be used to change its class:

~~~
tagName = "list2";
ks->setTagPattern(tagName, ...);
ks->setTagClass(tagName, Kuit::StructTag);
~~~


In a library, changes to the KUIT setup may need to be private,
applicable only in library's own domain, but they may also need
to be public, applicable in clients' domains.
For changes that should be public, the library should define
a public function which takes the domain as the argument
and performs all the changes on that domain:

~~~
void updateKuitSetup (const char *domain)
{
    KuitSetup *ks = Kuit::setupForDomain(domain);
    ....
}
~~~

The client code should then call this function in its initialization.

<a name="kuit_sel_fmt">

### Selecting Visual Format

The target visual format for any given `xi18n` call can be selected
in two ways.

The primary way is by UI markers in the message context,
which were described in the section \ref uimark_ctxt.
Every `@<major>:<minor>` marker combination has
a default target visual format assigned, as follows:
<table>
<tr><th>UI Marker</th>
    <th>Visual Format</th></tr>
<tr><td>(none)</td>
    <td>plain</td></tr>
<tr><td>\@action:\<any\></td>
    <td>plain</td></tr>
<tr><td>\@title:\<any\></td>
    <td>plain</td></tr>
<tr><td>\@option:\<any\></td>
    <td>plain</td></tr>
<tr><td>\@label:\<any\></td>
    <td>plain</td></tr>
<tr><td>\@item:\<any\></td>
     <td>plain</td></tr>
<tr><td>\@info, \@info:tooltip, \@info:whatsthis, \@info:usagetip</td>
    <td>rich</td></tr>
<tr><td>\@info:status, \@info:progress, \@info:credit</td>
    <td>plain</td></tr>
<tr><td>\@info:shell</td>
    <td>term</td></tr>
</tr>
</table>
Target visual formats associated with UI markers can be changed using
the `KUITSetup::setFormatForMarker` method:

~~~
KuitSetup *ks = Kuit::setupForDomain("fooapp");
// Set standalone @info (no minor component) to plain text:
ks->setFormatForMarker("@info", Kuit::PlainText);
// Set @info:tooltip to rich text:
ks->setFormatForMarker("@info:tooltip", Kuit::RichText);
// Set all @info:<minor> to plain text:
ks->setFormatForMarker("@info:", Kuit::PlainText);
~~~


The second way to select the visual format is by using a `kxi18n*` call,
and passing the format type to the `toString` method:

~~~
kxi18nc("@info", "Logging paused.").toString(Kuit::PlainText);
~~~

This will override any format implied by the UI marker if present.

If a library is making modifications in visual format association
with UI markers, and these changes should be available to clients,
the same approach as in the section \ref kuit_def_tags should be used.

<a name="kuit_escape">

### Escaping

While for `*i18n*` calls it was advised to keep each message text
well-formed by itself with respect to Qt rich text markup,
for `*xi18n*` calls well-formedness is mandatory.
This means that markup-significant characters in plain-looking text
need to be escaped, using standard XML entities:

~~~
xi18n("Installed Fooapp too old, need release &gt;= 2.1.8.");
xi18n("Set &lt;themeId&gt; as theme on startup");
~~~

The exception is the ampersand (&) character, which in XML denotes
the start of an entity, but in Qt denotes the accelerator marker.
Therefore it is necessary to escape ampersand as `&amp;`
only when it is in position which would result in valid entity syntax:

~~~
// Escaping not needed because not in entity-like position:
xi18n("Remove &all entries");
// Escaping not needed for the same reason wrt. markup,
// but ampersand doubled to escape it wrt. Qt accelerator marker:
xi18n("Look && Feel");
// Escaping wrt. markup necessary:
xi18n("Delete &amp;everything; see if I care!");
~~~

The example for necessary escaping above is rather artificial,
because in practice it is unlikely for ampersand to appear
in entity-like position while not actually starting an entity.

To assure the validity of markup when arguments are inserted into
`xi18n`-wrapped text, all markup-significant characters
in string arguments are automatically escaped.
For example, this works as expected:

~~~
QString filePath("assignment03-<yourname>.txt");
QString msg1 = kxi18n("Delete <filename>%1</filename>?", filePath)
                     .toString(Kuit::PlainText);
// msg1 == "Delete 'assignment03-<yourname>.txt'?"
QString msg2 = kxi18n("Delete <filename>%1</filename>?", filePath)
                     .toString(Kuit::RichText);
// msg2 == "Delete `assignment03-&lt;yourname&gt;.txt`?"
~~~

But, how then to compose a text where the arguments too should
contain some KUIT markup? This is done by using non-finalization
`kxi18n*` call to translate the argument text, and passing
the returned `KLocalizedString` object directly as argument:

~~~
KLocalizedString stateDesc = kxi18n(
    "On <emphasis>indefinite</emphasis> hold.");
QString msg = xi18nc("@info",
                     "<para>Task state:</para>"
                     "<para>%1</para>", stateDesc);
// msg == "<p>Task state:</p>"
//        "<p>On <i>indefinite</i> hold.</p>"
~~~

If the argument and the text have different visual formats implied by
their UI markers, the outermost format overrides inner formats.

<a name="kuit_tags">

### Predefined Tags

All KUIT tags belong to one of the two classes:
- phrase tags, which describe parts of sentences or whole sentences
  inserted into running text;
- structuring tags, which split text into paragraph-level blocks.

A text without any structuring tags is considered equivalent of
one paragraph or sub-paragraph sentence or phrase.
If at least one structuring tag appears in the text,
then the text is considered multi-paragraph,
and no content may appear outside of structuring tags.
For example:

~~~
// Good:
i18nc("@info",
      "You can configure the history sidebar here.");
// BAD:
xi18nc("@info",
       "<title>History Sidebar</title>"
       "You can configure the history sidebar here.");
// Good:
xi18nc("@info",
       "<title>History Sidebar</title>"
       "<para>You can configure the history sidebar here.</para>");
~~~


The current set of predefined tags is presented below.
For each tag the following information is stated:
the tag name (in superscript: Ki18n release of first appearance),
available attributes (in superscript: \em * if mandatory,
Ki18n release of first appearance),
admissible subtags, and description.

<table>
<tr><th>
Phrase Tags
</th></tr>
<tr><td>
<b>\<application\></b><sup>5.0</sup><br/>
Name of an application.

~~~
xi18nc("@action:inmenu",
       "Open with <application>%1</application>", appName);
~~~

</td></tr>
<tr><td>
<b>\<bcode\></b><sup>5.0</sup><br/>
Line-breaking body of code, for short code or output listings.

~~~
xi18nc("@info:whatsthis",
       "You can try the following snippet:<bcode>"
       "\\begin{equation}\n"
       "  C_{x_i} = \\frac{C_z^2}{e \\pi \\lambda}\n"
       "\\end{equation}\n"
       "</bcode>");
~~~

</td></tr>
<tr><td>
<b>\<command\></b><sup>5.0</sup><br/>
<i>Attributes:</i> section=<sup>5.0</sup><br/>
Name of a shell command, system call, signal, etc.
Its man section can be given with the `section=` attribute.

~~~
xi18nc("@info",
       "This will call <command>%1</command> internally.", cmdName);
xi18nc("@info",
       "Consult the man entry of "
       "<command section='1'>%1</command>", cmdName);
~~~

</td></tr>
<tr><td>
<b>\<email\></b><sup>5.0</sup><br/>
<i>Attributes:</i> address=<sup>5.0</sup><br/>
Email address.
Without attributes, the tag text is the address.
If the address is explicitly given with the `address=` attribute,
the tag text is the name or description attached to the address.
In rich text, the phrase will be hyperlinked.

~~~
xi18nc("@info",
       "Send bug reports to <email>%1</email>.", emailNull);
xi18nc("@info",
       "Send praises to <email address='%1'>the author</email>.", emailMy);
~~~

</td></tr>
<tr><td>
<b>\<emphasis\></b><sup>5.0</sup><br/>
<i>Attributes:</i> strong=<sup>5.0</sup><br/>
Emphasized word or phrase in the text.
For strong emphasis, attribute `strong=`
can be set to `[1|true|yes]`.
</td></tr>
<tr><td>
<b>\<envar\></b><sup>5.0</sup><br/>
Environment variable.
A `$`-sign will be prepended automatically in formatted text.

~~~
xi18nc("@info",
       "Assure that <envar>PATH</envar> is properly set.");
~~~

</td></tr>
<tr><td>
<b>\<filename\></b><sup>5.0</sup><br/>
<i>Subtags:</i> \<envar\>, \<placeholder\><br/>
File or folder name or path.
Slash (/) should be used as path separator, and it will be converted
into native separator for the underlying platform.

~~~
xi18nc("@info", "Cannot read <filename>%1</filename>.", filePath);
xi18nc("@info",
       "<filename><envar>HOME</envar>/.foorc</filename> "
       "does not exist.");
~~~
</td></tr>
<tr><td>
<b>\<icode\></b><sup>5.0</sup><br/>
<i>Subtags:</i> \<envar\>, \<placeholder\><br/>
Inline code, like a shell command line.

~~~
xi18nc("@info:tooltip",
       "Execute <icode>svn merge</icode> on selected revisions.");
~~~

</td></tr>
<tr><td>
<b>\<interface\></b><sup>5.0</sup><br/>
Path to a graphical user interface element.
If a path of UI elements is needed,
elements should be separated with `|` or `->`,
which will be converted into the canonical separator.

~~~
xi18nc("@info:whatsthis",
       "If you make a mistake, click "
       "<interface>Reset</interface> to start again.");
xi18nc("@info:whatsthis",
       "The line colors can be changed under "
       "<interface>Settings->Visuals</interface>.");
~~~

</td></tr>
<tr><td>
<b>\<link\></b><sup>5.0</sup><br/>
<i>Attributes:</i> url=<sup>5.0</sup><br/>
Link to a URL-addressable resource.
Without attributes, the tag text is the URL;
alternatively, the URL can be given by `url=` attribute,
and then the text serves as description.
Separate URL and description are preferred if applicable.
The phrase will be hyperlinked in rich text.

~~~
xi18nc("@info:tooltip",
       "Go to <link>%1</link> website.", urlKDE);
xi18nc("@info:tooltip",
       "Go to the <link url='%1'>KDE website</link>.", urlKDE);
~~~

</td></tr>
<tr><td>
<b>\<message\></b><sup>5.0</sup><br/>
<i>Subtags:</i> all phrase tags<br/>
An external message inserted into the text.

~~~
xi18nc("@info",
       "The fortune cookie says: <message>%1</message>", cookieText);
~~~

</td></tr>
<tr><td>
<b>\<nl\></b><sup>5.0</sup><br/>
Line break.

~~~
xi18nc("@info",
       "The server replied:<nl/>"
       "<message>%1</message>", serverReply);
~~~

</td></tr>
<tr><td>
<b>\<note\></b><sup>5.0</sup><br/>
<i>Attributes:</i> label=<sup>5.0</sup><br/>
<i>Subtags:</i> all phrase tags<br/>
The sentence is a side note related to the topic.
Prefix "Note:" will be added automatically;
another prefix can be set with attribute `label=`.

~~~
xi18nc("@info",
       "Probably the best known of all duck species is the Mallard. "
       "It breeds throughout the temperate areas around the world. "
       "<note>Most domestic ducks are derived from Mallard.</note>");
~~~

</tr></td>
<tr><td>
<b>\<placeholder\></b><sup>5.0</sup><br/>
A placeholder text.
It could be something which the user should replace with actual text,
or a generic item in a list.

~~~
xi18nc("@info",
       "Replace <placeholder>name</placeholder> with your name.");
xi18nc("@item:inlistbox",
       "<placeholder>All images</placeholder>");
~~~

</td></tr>
<tr><td>
<b>\<shortcut\></b><sup>5.0</sup><br/>
Combination of keyboard keys to press.
Key names should be separated by "+" or "-",
and the shortcut will be converted into canonical form.

~~~
xi18nc("@info:whatsthis",
       "Cycle through layouts using <shortcut>Alt+Space</shortcut>.");
~~~

</td></tr>
<tr><td>
<b>\<warning\></b><sup>5.0</sup><br/>
<i>Attributes:</i> label=<sup>5.0</sup><br/>
<i>Subtags:</i> all phrase tags<br/>
The sentence is a warning.
Prefix "Warning:" will be added automatically;
another prefix can be set with attribute `label=`.

~~~
xi18nc("@info",
       "Really delete this key? "
       "<warning>This cannot be undone.</warning>");
~~~

</tr></td>
<tr><th>
Structuring Tags
</th></tr>
<tr><td>
<b>\<item\></b><sup>5.0</sup><br/>
<i>Subtags:</i> all phrase tags<br/>
A list item.
</tr></td>
<tr><td>
<b>\<list\></b><sup>5.0</sup><br/>
<i>Subtags:</i> \<item\><br/>
List of items.
List is considered an element of the paragraph,
so `<list>` tags must be inside `<para>` tags.
</tr></td>
<tr><td>
<b>\<para\></b><sup>5.0</sup><br/>
<i>Subtags:</i> all phrase tags, \<list\><br/>
One paragraph of text.
</tr></td>
<tr><td>
<b>\<subtitle\></b><sup>5.0</sup><br/>
<i>Subtags:</i> all phrase tags<br/>
The subtitle of the text. Must come after `<title>`.
</tr></td>
<tr><td>
<b>\<title\></b><sup>5.0</sup><br/>
<i>Subtags:</i> all phrase tags<br/>
The title of the text.
Must be the first tag in the text if present.
</tr></td>
</table>

The criteria for adding new tags to the predefined set,
particularly new phrase tags, are not very strict.
If the tag is clearly useful to a class of applications,
of which more than one are known to use Ki18n,
it is reasonable to add it here.
Adding synonymous tag names is also fine, where one finds that
the original name is not sufficiently discoverable,
or that it is too verbose for the given frequency of use.

<a name="kuit_ents">

### Predefined Entities

KUIT defines a fixed set of XML entities, which means that unlike tags,
entities cannot be added to, nor their character expansions changed.
The standard XML entities are:
<table>
<tr><th>Entity</th><th>Expansion</th></tr>
<tr><td><b>\&lt;</b></td><td>less-than (`<`)</td></tr>
<tr><td><b>\&gt;</b></td><td>greater-than (`>`)</td></tr>
<tr><td><b>\&amp;</b></td><td>ampersand (`&`)</td></tr>
<tr><td><b>\&apos;</b></td><td>single quote (`'`)</td></tr>
<tr><td><b>\&quot;</b></td><td>double quote (`"`)</td></tr>
</table>
The `&amp;apos;` and `&amp;quot;` are really needed only
within attribute values (and even there they can be avoided
by choosing the opposite quote sign for quoting the attribute value).

Additional entities are (Ki18n release where they were introduced
given in superscript):
<table>
<tr><th>Entity</th><th>Expansion</th></tr>
<tr>
<td><b>\&nbsp;</b><sup>5.0</sup></td>
<td></td>non-breaking space (`&nbsp;`)
</tr>
</table>

The reason for not allowing custom entities
can be demonstrated by the following example.
An application programmer may think of defining the entity
`&amp;appname;`, which would be used everywhere in text in place of
the actual application name. In that way, seemingly, it would be easy
to play with various names or spellings without disrupting translations.
The problem, however, is that in many languages the structure of
the sentence depends on the grammar properties of the particular name
(e.g. its grammar gender or number), and conversely, the name may
need modification according to sentence structure (e.g. by grammar case).
Thus, custom entities are not allowed because
they are misused too easily.
If something *really* needs to be inserted verbatim into text,
argument substitution is always at hand.


<a name="non_text">

## Localizing Non-Text Resources

It sometimes happens that a non-textual application resource
needs localization. The most frequent example are images
that contain some text, like splash screens.
Ki18n also provides a rudimentary facility for this situation,
the `KLocalizedString::localizedFilePath` static method.
When called with a resource file path as the argument,
this method will check what are the active languages,
and look if there exists a localized version of the resource at path
`<original-parent-dir>/l10n/<language>/<original-basename>`.
For example, if the active language is `aa`, and a candidate image
for localization is installed as:

~~~
$PREFIX/share/fooapp/splash.png
~~~

then a call to

~~~
QString splashPath = QStandardPaths::locate(
    QStandardPaths::GenericDataLocation, "splash.png");
splashPath = KLocalizedString::localizedFilePath(splashPath);
~~~

will check if there exist the file

~~~
$PREFIX/share/fooapp/l10n/aa/splash.png
~~~

and return that path if it does, or else the original path.

Some KDE libraries will call `KLocalizedString::localizedFilePath`
on their own behind the scene, for resources that may need localization
but whose paths are not directly manipulated in application sources.
An example here are icons handled through `KIcon` class,
which are referred to in the code only by the icon name.


<a name="refs">

## Further References

For details about the format of translation catalogs (PO)
and various Gettext tools, the first stop is the
<a href="http://www.gnu.org/software/gettext/manual/gettext.html">
Gettext manual</a>.

<a href="http://techbase.kde.org/">KDE Techbase</a> contains a
<a href="http://techbase.kde.org/Development/Tutorials/Localization">
series of tutorials</a> on preparing the KDE code for localization,
and on the internationalization process in general.

