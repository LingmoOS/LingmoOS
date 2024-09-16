// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cupsppd.h"
#include "cupsmodule.h"

#include <ctype.h>
#include <iconv.h>
#include <stdbool.h>
#include <stdexcept>

using namespace std;

/////////////////////////
// Encoding conversion //
/////////////////////////

static int ppd_encoding_is_utf8(PPD *ppd)
{
    const char *lang_encoding = nullptr, *from_encoding = nullptr;
    iconv_t cdf, cdt;
    if (ppd->conv_from != nullptr)
        return 0;

    lang_encoding = ppd->ppd->lang_encoding;
    if (lang_encoding && !strcasecmp(lang_encoding, "UTF-8"))
        return 1;

    if (lang_encoding && !strcasecmp(lang_encoding, "ISOLatin1"))
        from_encoding = "ISO-8859-1";
    else if (lang_encoding && !strcasecmp(lang_encoding, "ISOLatin2"))
        from_encoding = "ISO-8859-2";
    else if (lang_encoding && !strcasecmp(lang_encoding, "ISOLatin5"))
        from_encoding = "ISO-8859-5";
    else if (lang_encoding && !strcasecmp(lang_encoding, "JIS83-RKSJ"))
        from_encoding = "SHIFT-JIS";
    else if (lang_encoding && !strcasecmp(lang_encoding, "MacStandard"))
        from_encoding = "MACINTOSH";
    else if (lang_encoding && !strcasecmp(lang_encoding, "WindowsANSI"))
        from_encoding = "WINDOWS-1252";
    else // Guess
        from_encoding = "ISO-8859-1";

    cdf = iconv_open("UTF-8", from_encoding);
    if (cdf == (iconv_t) -1)
        cdf = iconv_open("UTF-8", "ISO-8859-1");

    cdt = iconv_open(from_encoding, "UTF-8");
    if (cdt == (iconv_t) -1)
        cdt = iconv_open("ISO-8859-1", "UTF-8");

    ppd->conv_from = (iconv_t *)malloc(sizeof(iconv_t));
    *ppd->conv_from = cdf;

    ppd->conv_to = (iconv_t *)malloc(sizeof(iconv_t));
    *ppd->conv_to = cdt;

    return 0;
}

static char *utf8_to_ppd_encoding(PPD *ppd, const char *inbuf)
{
    char *outbuf = nullptr, *ret = nullptr;
    size_t len, outsize, outbytesleft;
    iconv_t cdt;

    if (ppd_encoding_is_utf8(ppd))
        return strdup(inbuf);

    cdt = *ppd->conv_to;

    // Reset to initial state
    iconv(cdt, nullptr, nullptr, nullptr, nullptr);
    len = strlen(inbuf);
    outsize = 1 + 6 * len;
    outbytesleft = outsize - 1;
    ret = outbuf = (char *)malloc(outsize);
    if (iconv(cdt, (char **)&inbuf, &len, &outbuf, &outbytesleft) == (size_t) -1) {
        free(outbuf);
        return nullptr;
    }
    *outbuf = '\0';

    return ret;
}

/////////
// PPD //
/////////

PPD::PPD(void)
{
    ppd = nullptr;
    file = nullptr;
    conv_from = nullptr;
    conv_to = nullptr;
}

void PPD::load(const char *filename)
{
    /*打开文件之前先确保关闭，可能重复打开*/
    if (this->file) {
        fclose(this->file);
        this->file = nullptr;
    }
    if (this->ppd) {
        ppdClose(this->ppd);
        this->ppd = nullptr;
    }
    if (this->conv_from)
        iconv_close(*this->conv_from);
    if (this->conv_to)
        iconv_close(*this->conv_to);

    this->file = fopen(filename, "r");
    if (!this->file) {
        throw runtime_error("fopen failed");
    }

    debugprintf("+ PPD %p %s (fd %d)\n", this, filename, fileno(this->file));
    this->ppd = ppdOpenFile(filename);
    if (!this->ppd) {
        fclose(this->file);
        this->file = nullptr;
        throw runtime_error("ppdOpenFile failed");
    }
    this->conv_from = this->conv_to = nullptr;
}

PPD::~PPD(void)
{
    if (this->file) {
        debugprintf("- PPD %p (fd %d)\n", this, fileno(this->file));
        fclose(this->file);
    } else {
        debugprintf("- PPD %p (no fd)\n", this);
    }

    if (this->ppd)
        ppdClose(this->ppd);
    if (this->conv_from)
        iconv_close(*this->conv_from);
    if (this->conv_to)
        iconv_close(*this->conv_to);
}

/////////
// PPD // METHODS
/////////

void PPD::localize(void)
{
    if (!ppdLocalize(this->ppd))
        return; // OK

    throw runtime_error(string_format("%d", errno));
}

string PPD::localizeIPPReason(const char *reason, const char *scheme)
{
    string ret;
    char *buffer = nullptr;
    const size_t bufsize = 1024;

    buffer = (char *)malloc(bufsize);
    if (ppdLocalizeIPPReason(this->ppd, reason, scheme, buffer, bufsize)) {
        ret = string(buffer);
    }

    free(buffer);
    return ret;
}

string PPD::localizeMarkerName(const char *name)
{
#if CUPS_VERSION_MAJOR > 1 || (CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR >= 4)
    string ret;
    const char *lname = nullptr;

    lname = ppdLocalizeMarkerName(this->ppd, name);
    if (lname != nullptr) {
        ret = string(lname);
    }

    return ret;
#else /* earlier than CUPS 1.4 */
    throw runtime_error("Operation not supported - recompile against CUPS 1.4 or later");
#endif /* CUPS 1.4 */
}

void PPD::markDefaults(void)
{
    ppdMarkDefaults(this->ppd);
}

int PPD::markOption(const char *name, const char *value)
{
    int conflicts;
    char *encname = nullptr, *encvalue = nullptr;

    encname = utf8_to_ppd_encoding(this, name);
    if (!encname) {
        throw runtime_error(string_format("%d", errno));
    }

    encvalue = utf8_to_ppd_encoding(this, value);
    if (!encvalue) {
        throw runtime_error(string_format("%d", errno));
    }

    conflicts = ppdMarkOption(this->ppd, encname, encvalue);
    if (encname)
        free(encname);
    if (encvalue)
        free(encvalue);

    return conflicts;
}

int PPD::conflicts(void)
{
    return ppdConflicts(this->ppd);
}

Option PPD::findOption(const char *option)
{
    Option ret;
    ppd_option_t *opt = nullptr;

    opt = ppdFindOption(this->ppd, option);
    if (opt) {
        ret.option = opt;
        ret.ppd = this;
    }

    return ret;
}

Attribute PPD::findAttr(const char *name, const char *spec)
{
    Attribute ret;
    ppd_attr_t *attr = nullptr;

    attr = ppdFindAttr(this->ppd, name, spec);
    if (attr) {
        ret.attribute = attr;
        ret.ppd = this;
    }

    return ret;
}

Attribute PPD::findNextAttr(const char *name, const char *spec)
{
    Attribute ret;
    ppd_attr_t *attr = nullptr;

    attr = ppdFindNextAttr(this->ppd, name, spec);
    if (attr) {
        ret.attribute = attr;
        ret.ppd = this;
    }

    return ret;
}

static int nondefaults_are_marked(ppd_group_t *g)
{
    ppd_option_t *o = nullptr;
    int oi;
    for (oi = 0, o = g->options; oi < g->num_options; oi++, o++) {
        ppd_choice_t *c = nullptr;
        int ci;
        for (ci = 0, c = o->choices; ci < o->num_choices; ci++, c++) {
            if (c->marked) {
                if (strcmp(c->choice, o->defchoice))
                    return 1;
                break;
            }
        }
    }

    return 0;
}

int PPD::nondefaultsMarked(void)
{
    int nondefaults_marked = 0;
    ppd_group_t *g = nullptr;
    int gi;
    for (gi = 0, g = this->ppd->groups;
            gi < this->ppd->num_groups && !nondefaults_marked; gi++, g++) {
        ppd_group_t *sg = nullptr;
        int sgi;
        if (nondefaults_are_marked(g)) {
            nondefaults_marked = 1;
            break;
        }

        for (sgi = 0, sg = g->subgroups;
                sgi < g->num_subgroups; sgi++, sg++) {
            if (nondefaults_are_marked(sg)) {
                nondefaults_marked = 1;
                break;
            }
        }
    }

    return nondefaults_marked;
}

#if !(_POSIX_C_SOURCE >= 200809L || _XOPEN_SOURCE >= 700)
/*
 * A rudimentary emulation of getline() for systems that dont support it
 * natively.  Since this is used for PPD file reading, it assumes (possibly
 * falsely) that BUFSIZ is big enough.
 */
ssize_t getline(char **line, size_t *linelen, FILE *fp)
{
    if (*linelen == 0) {
        *linelen = BUFSIZ;
        *line = malloc(*linelen);
    }

    memset(*line, 0, *linelen);
    fgets(*line, *linelen, fp);

    return (strlen(*line));
}
#endif

/*
 * emit marked options by returning a string.
 */
string PPD::emitString(ppd_section_t section, float min_order)
{
    char *emitted = nullptr;
    string ret;
    emitted = ppdEmitString(this->ppd, section, min_order);
    if (emitted) {
        ret = string(emitted);
        free(emitted);
    }

    return ret;
}

/*
 * emit marked options by writing to a file object.
 */
void PPD::emitFile(FILE *f, ppd_section_t section)
{
    if (!ppdEmit(this->ppd, f, section))
        return;

    throw runtime_error(string_format("%d", errno));
}

/*
 * emit marked options after order dependency by writing to a file object.
 */
void PPD::emitAfterOrder(FILE *f, ppd_section_t section, int limit, float min_order)
{
    if (!ppdEmitAfterOrder(this->ppd, f, section, limit, min_order))
        return;

    throw runtime_error(string_format("%d", errno));
}

/*
 * emit marked options by writing to a file descriptor.
 */
void PPD::emitFd(int fd, ppd_section_t section)
{
    if (!ppdEmitFd(this->ppd, fd, section))
        return;

    throw runtime_error(string_format("%d", errno));
}

/*
 * emit JCL options by writing to a file object.
 */
void PPD::emitJCL(FILE *f, int job_id, const char *user, const char *title)
{
    if (!ppdEmitJCL(this->ppd, f, job_id, user, title))
        return;

    throw runtime_error(string_format("%d", errno));
}

/*
 * emit JCL end by writing to a file object.
 */
void PPD::emitJCLEnd(FILE *f)
{
    if (!ppdEmitJCLEnd(this->ppd, f))
        return;

    throw runtime_error(string_format("%d", errno));
}

void PPD::writeFd(int fd)
{
    char *line = nullptr;
    size_t linelen = 0;
    FILE *out = nullptr;
    int dfd;

    dfd = dup(fd);
    if (dfd == -1)
        throw runtime_error(string_format("%d", errno));

    out = fdopen(dfd, "w");
    if (!out)
        throw runtime_error(string_format("%d", errno));

    rewind(this->file);
    while (!feof(this->file)) {
        int written = 0;
        ssize_t got = getline(&line, &linelen, this->file);
        if (got == -1)
            break;

        if (!strncmp(line, "*Default", 8)) {
            char *keyword = nullptr;
            char *start = line + 8;
            char *end = nullptr;
            ppd_choice_t *choice = nullptr;
            for (end = start; *end; end++) {
                if (isspace(*end) || *end == ':')
                    break;
            }
            keyword = (char *)calloc(1, (end - start) + 1);
            strncpy(keyword, start, end - start);
            choice = ppdFindMarkedChoice(this->ppd, keyword);

            // Treat PageRegion, PaperDimension and ImageableArea specially:
            // if not marked, use PageSize option.
            if (!choice && (!strcmp(keyword, "PageRegion") || !strcmp(keyword, "PaperDimension") || !strcmp(keyword, "ImageableArea"))) {
                choice = ppdFindMarkedChoice(this->ppd, "PageSize");
            }

            if (choice) {
                fprintf(out, "*Default%s: %s", keyword, choice->choice);
                if (strchr(end, '\r'))
                    fputs("\r", out);
                fputs("\n", out);
                written = 1;
            }
            if (keyword)
                free(keyword);
        }

        if (!written)
            fputs(line, out);
    }

    fclose(out);
    if (line)
        free(line);
}

/////////
// PPD // ATTRIBUTES
/////////

vector<Constraint> PPD::getConstraints(void)
{
    Constraint cns;
    vector<Constraint> ret;
    ppd_const_t *c = nullptr;
    int i;
    for (i = 0, c = this->ppd->consts; i < this->ppd->num_consts; i++, c++) {
        cns.constraint = c;
        cns.ppd = this;
        ret.push_back(cns);
    }

    return ret;
}

vector<Attribute> PPD::getAttributes(void)
{
    Attribute as;
    vector<Attribute> ret;
    int i;
    for (i = 0; i < this->ppd->num_attrs; i++) {
        as.attribute = this->ppd->attrs[i];
        as.ppd = this;
        ret.push_back(as);
    }

    return ret;
}

vector<Group> PPD::getOptionGroups(void)
{
    vector<Group> ret;
    Group grp;
    ppd_group_t *group = nullptr;
    int i;

    for (i = 0, group = this->ppd->groups; i < this->ppd->num_groups; i++, group++) {
        grp.group = group;
        grp.ppd = this;
        ret.push_back(grp);
    }

    return ret;
}

////////////
// Option // ATTRIBUTES
////////////

bool Option::getConflicted(void)
{
    return (!this->option || this->option->conflicted);
}

string Option::getKeyword(void)
{
    string ret;
    if (this->option) {
        ret = string(this->option->keyword);
    }

    return ret;
}

string Option::getDefchoice(void)
{
    string ret;
    if (this->option) {
        ret = string(this->option->defchoice);
    }

    return ret;
}

string Option::getText(void)
{
    string ret;
    if (this->option) {
        ret = string(this->option->text);
    }

    return ret;
}

int Option::getUI(void)
{
    if (!this->option) {
        return 0;
    }
    return this->option->ui;
}

vector<map<string, string>> Option::getChoices(void)
{
    vector<map<string, string>> choices;
    ppd_choice_t *choice = nullptr;
    bool defchoice_seen = false;
    int i;

    if (!this->option)
        return choices;

    for (i = 0, choice = this->option->choices; i < this->option->num_choices; i++, choice++) {
        map<string, string> choice_dict;

        choice_dict["choice"] = choice->choice;
        choice_dict["text"] = choice->text;
        choice_dict["marked"] = choice->marked;

        choices.push_back(choice_dict);
        if (!strcmp(choice->choice, this->option->defchoice))
            defchoice_seen = true;
    }

    if (!defchoice_seen) {
        // This PPD option has a default choice that isn't one of the choices.
        // This really happens.
        map<string, string> choice_dict;
        const char *defchoice = this->option->defchoice;

        choice_dict["choice"] = defchoice;
        choice_dict["text"] = defchoice;
        choices.push_back(choice_dict);
    }

    return choices;
}

///////////
// Group // ATTRIBUTES
///////////

string Group::getText(void)
{
    string ret;
    if (this->group) {
        ret = string(this->group->text);
    }

    return ret;
}

string Group::getName(void)
{
    string ret;
    if (this->group) {
        ret = string(this->group->name);
    }

    return ret;
}

vector<Option> Group::getOptions(void)
{
    vector<Option> options;
    Option opt;
    ppd_option_t *option = nullptr;
    int i;

    if (!this->group)
        return options;

    for (i = 0, option = this->group->options; i < this->group->num_options; i++, option++) {
        opt.option = option;
        opt.ppd = this->ppd;
        options.push_back(opt);
    }

    return options;
}

vector<Group> Group::getSubgroups(void)
{
    vector<Group> subgroups;
    Group grp;
    ppd_group_t *subgroup = nullptr;
    int i;

    if (!this->group)
        return subgroups;

    for (i = 0, subgroup = this->group->subgroups; i < this->group->num_subgroups; i++, subgroup++) {
        grp.group = subgroup;
        grp.ppd = this->ppd;
        subgroups.push_back(grp);
    }

    return subgroups;
}

////////////////
// Constraint // ATTRIBUTES
////////////////

string Constraint::getOption1(void)
{
    string ret;
    if (this->constraint) {
        ret = this->constraint->option1;
    }

    return ret;
}

string Constraint::getChoice1(void)
{
    string ret;
    if (this->constraint) {
        ret = this->constraint->choice1;
    }

    return ret;
}

string Constraint::getOption2(void)
{
    string ret;
    if (this->constraint) {
        ret = this->constraint->option2;
    }

    return ret;
}

string Constraint::getChoice2(void)
{
    string ret;
    if (this->constraint) {
        ret = this->constraint->choice2;
    }

    return ret;
}

///////////////
// Attribute // ATTRIBUTES
///////////////

string Attribute::getName(void)
{
    string ret;
    if (this->attribute) {
        ret = this->attribute->name;
    }

    return ret;
}

string Attribute::getSpec(void)
{
    string ret;
    if (this->attribute) {
        ret = this->attribute->spec;
    }

    return ret;
}

string Attribute::getText(void)
{
    string ret;
    if (this->attribute) {
        ret = this->attribute->text;
    }

    return ret;
}

string Attribute::getValue(void)
{
    string ret;
    if (this->attribute) {
        ret = this->attribute->value;
    }

    return ret;
}
