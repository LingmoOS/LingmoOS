// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HAVE_CUPSPPD_H
#define HAVE_CUPSPPD_H

#include <cups/ppd.h>
#include <iconv.h>
#include <vector>
#include <string>
#include <map>

class PPD;

class Option
{
public:
    Option() {}
    ~Option() {}

    bool getConflicted(void);
    std::string getKeyword(void);
    std::string getDefchoice(void);
    std::string getText(void);

    // @return: one of these UI types:
    // PPD_UI_BOOLEAN,          /* True or False option */
    // PPD_UI_PICKONE,          /* Pick one from a list */
    // PPD_UI_PICKMANY          /* Pick zero or more from a list */
    int getUI(void);

    // @return: each map is a choice with the key is one of these attributes:
    //          "choice",
    //          "text",
    //          "marked",
    //
    std::vector<std::map<std::string, std::string>> getChoices(void);

    ppd_option_t *option = nullptr;
    PPD *ppd = nullptr;
};

class Constraint
{
public:
    Constraint() {}
    ~Constraint() {}
    std::string getOption1(void);
    std::string getChoice1(void);
    std::string getOption2(void);
    std::string getChoice2(void);

    ppd_const_t *constraint = nullptr;
    PPD *ppd = nullptr;
};

class Group
{
public:
    Group() {}
    ~Group() {}
    std::string getText(void);
    std::string getName(void);
    std::vector<Option> getOptions(void);
    std::vector<Group> getSubgroups(void);

    ppd_group_t *group = nullptr;
    PPD *ppd = nullptr;
};

class Attribute
{
public:
    Attribute() {}
    ~Attribute() {}

    std::string getName(void);
    std::string getSpec(void);
    std::string getText(void);
    std::string getValue(void);

    ppd_attr_t *attribute = nullptr;
    PPD *ppd = nullptr;
};

class PPD
{
public:
    PPD(void);
    ~PPD(void);

    // @param filename: must be not null!
    // @exception: runtime_error
    void load(const char *filename);

    void init(const char *filename);
    void localize(void);

    // @param reason: must be not null!
    // @param scheme: URI scheme, optional.
    // @exception: none
    std::string localizeIPPReason(const char *reason, const char *scheme);

    // @param name: must be not null!
    // @exception: runtime_error
    std::string localizeMarkerName(const char *name);

    // @description: Mark/select all default options in the PPD file.
    void markDefaults(void);

    // @description: Mark/select an options in the PPD file.
    // @param name: must be not null!
    // @param value: must be not null!
    // @return: Number of conflicts
    // @exception: runtime_error
    int markOption(const char *name, const char *value);

    // @description: Check to see if there are any conflicts among the marked option choices.
    // @return: Number of conflicts found. The returned value is the same as returned by markOption.
    int conflicts(void);

    // @param option: must be not null!
    // @exception: none
    // @note: every Option has a pointer point to current PPD object,
    //        so must destroy Option before destroy PPD object.
    Option findOption(const char *option);

    // @param name: must be not null!
    // @param spec: optional, see getSpec method
    // @exception: none
    // @note: every Attribute has a pointer point to current PPD object,
    //        so must destroy Attribute before destroy PPD object.
    Attribute findAttr(const char *name, const char *spec);

    // @param name: must be not null!
    // @param spec: optional, see getSpec method
    // @exception: none
    // @note: every Attribute has a pointer point to current PPD object,
    //        so must destroy Attribute before destroy PPD object.
    Attribute findNextAttr(const char *name, const char *spec);

    // @description: refer to ppd_choice_t.marked field.
    // @return: Number of non-default marked
    // @exception: none
    int nondefaultsMarked(void);

    // @description: Get a string containing the code for marked options.
    // @param section: can be one of these value:
    //                  PPD_ORDER_ANY,      /* Option code can be anywhere in the file */
    //                  PPD_ORDER_DOCUMENT, /* ... must be in the DocumentSetup section */
    //                  PPD_ORDER_EXIT,     /* ... must be sent prior to the document */
    //                  PPD_ORDER_JCL,      /* ... must be sent as a JCL command */
    //                  PPD_ORDER_PAGE,     /* ... must be in the PageSetup section */
    //                  PPD_ORDER_PROLOG    /* ... must be in the Prolog section */
    // @param min_order: When @min_order > 0.0, this method only includes options
    //                   whose OrderDependency value is greater than or equal to "min_order".
    //                   Otherwise, all options in the specified section are included in the
    //                   returned string.
    std::string emitString(ppd_section_t section, float min_order);

    void emitFile(FILE *f, ppd_section_t section);

    // @param limit: 0 or 1, if set to 1, use @min_order; else same as emitFile
    void emitAfterOrder(FILE *f, ppd_section_t section, int limit, float min_order);

    void emitFd(int fd, ppd_section_t section);
    void emitJCL(FILE *f, int job_id, const char *user, const char *title);
    void emitJCLEnd(FILE *f);

    // @description: Write all default option into a new PPD file specified by fd.
    //               Usually called after markOption.
    // @exception: runtime_error
    void writeFd(int fd);

    // @note: every Constraint has a pointer point to current PPD object,
    //        so must destroy Constraint before destroy PPD object.
    // @exception: none
    std::vector<Constraint> getConstraints(void);

    // @note: every Attribute has a pointer point to current PPD object,
    //        so must destroy Attribute before destroy PPD object.
    // @exception: none
    std::vector<Attribute> getAttributes(void);

    // @note: every Group has a pointer point to current PPD object,
    //        so must destroy Group before destroy PPD object.
    // @exception: none
    std::vector<Group> getOptionGroups(void);

public:
    ppd_file_t *ppd;
    FILE *file;
    iconv_t *conv_from;
    iconv_t *conv_to;
};

#endif /* HAVE_CUPSPPD_H */
