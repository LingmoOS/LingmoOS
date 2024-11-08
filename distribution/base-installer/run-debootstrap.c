#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include <cdebconf/debconfclient.h>
#include <debian-installer.h>

#include "waypoints.h"

#define DEBCONF_BASE          "base-installer/debootstrap/"

struct debconfclient *debconf = NULL;
int progress_start_position = 0;

/* args = read_arg_lines("EA: ", ifp, &arg_count, &line); */
char **
read_arg_lines(const char *prefix, FILE *ifp, int *arg_count, char **final_line,
	       int *llen)
{
    static char **args = NULL;
    static int arg_max = 0;
    size_t dummy = 0;

    if (args == NULL)
    {
        arg_max = 4;
        args = malloc(sizeof(char *) * arg_max);
    }
    *arg_count = 0;
    while (1)
    {
        *final_line = NULL;
        if ((*llen = getline(final_line, &dummy, ifp)) <= 0)
        {
            return NULL;
        }
        (*final_line)[*llen-1] = 0;
        if (strstr(*final_line, prefix) == *final_line)
        {
            if (*arg_count >= arg_max) {
                arg_max += 4;
                args = realloc(args, sizeof(char *) * arg_max);
            }
            args[(*arg_count)++] = strdup(*final_line+strlen(prefix));
            /* we got arguments. */
        }
        else
            break;
    }
    return args;
}

char *
n_sprintf(char *fmt, int arg_count, char **args)
{
    char *ret;

    switch (arg_count)
    {
        case 0: ret = strdup(fmt); break;
        case 1: asprintf(&ret, fmt, args[0]); break;
        case 2: asprintf(&ret, fmt, args[0], args[1]); break;
        case 3: asprintf(&ret, fmt, args[0], args[1], args[2]); break;
        default: ret = NULL; break;
    }
    return ret;
}

void
n_subst(char *template, int arg_count, char **args)
{
    int i;

    for (i = 0; i < arg_count; i++)
    {
        debconf->commandf(debconf, "SUBST %s SUBST%d %s", template, i, args[i]);
    }
}

/* changes in 'code' */
char *
find_template(const char *prefix, char *code)
{
    char *p;

    for (p = code; *p; p++)
        *p = tolower(*p);
    asprintf(&p, DEBCONF_BASE "%s/%s", prefix, code);
    if (debconf_metaget(debconf, p, "description") == 0)
        return p;
    else
    {
        free(p);
        return NULL;
    }
}

int get_progress_start_position (void) {
    const char *progress_env = getenv("PB_PROGRESS");
    if (progress_env)
        return atoi(progress_env);
    else
        return 0;
}

/* Calculate progress bar location, starting at
 * previous waypoint, and advancing the percent of
 * the current section that corresponds to the percent
 * of the debootstrap progress indicator. */
void set_progress (int current_section, int phigh, int plow) {
    float section_fraction;
    int section_span, prev_waypoint, percent;

    prev_waypoint = waypoints[current_section].startpercent;
    if (current_section > 0)
        section_span = waypoints[current_section].endpercent - prev_waypoint;
    else
        section_span = 0;

    if (phigh > 0)
        section_fraction = (float) plow / (float) phigh;
    else
        section_fraction = 0;
    if (section_fraction > 1)
        section_fraction = 1;
			
    percent = prev_waypoint + (section_span * section_fraction);

#if 0
    fprintf(stderr, "waypoint: %s (%i); prev endpercent %i; span: %i; fraction: %.9f (%i / %i); percent: %i\n",
            waypoints[current_section].progress_id,
            current_section, prev_waypoint, section_span, 
            section_fraction, plow, phigh, percent);
#endif

    debconf_progress_set(debconf, progress_start_position + percent);
}

/*
 * Copied from boot-floppies/utilities/dbootstrap/extract_base.c
 * and modified to use cdebconf progress bars
 */
static int
exec_debootstrap(char **argv){
    char **args = NULL;
    int arg_count;
    int from_db[2]; /* 0=read, 1=write */
    FILE *ifp;
    pid_t pid;
    int status, rv;
    char *ptr, *line, *template, *section_text = NULL;
    int llen;
    size_t dummy = 0;
    int current_section = 0;
    int child_exit = 0;

    pipe(from_db);

    if ((pid = fork()) == 0)
    {
        close(from_db[0]);

        if (dup2(from_db[1], 3) == -1)
            perror("dup2");
        close(from_db[1]);

        if (freopen("/dev/null", "r", stdin) == NULL)
            perror("freopen");

        setenv("PERL_BADLANG", "0", 1);
        /* These are needed to hack around a hack (!) in update-inetd
         * and to not confuse debconf's postinst */
        unsetenv("DEBIAN_HAS_FRONTEND");
        unsetenv("DEBIAN_FRONTEND");
        unsetenv("DEBCONF_FRONTEND");
        unsetenv("DEBCONF_REDIR");
        if (execv(argv[0], argv) != 0)
            perror("execv");
        return -1;
    }
    else if (pid == -1)
    {
        perror("fork");
        return -1;
    }

    progress_start_position = get_progress_start_position();

    close(from_db[1]);

    if ((ifp = fdopen(from_db[0], "r")) == NULL) {
        perror("fdopen");
        return -1;
    }

    line = NULL;
    llen = getline(&line, &dummy, ifp);
    while (!child_exit && llen > 0)
    {
        line[llen-1] = 0;

	/* fprintf(stderr, "got line: %s\n", line); */
	
        ptr = line;
        switch (ptr[0])
        {
            case 'E':
                {
                    ptr += 3;
                    /* ptr now contains the identifier of the error. */
                    template = find_template("error", ptr);
                    args = read_arg_lines("EA: ", ifp, &arg_count, &line,
					  &llen);
                    if (args == NULL)
                    {
                        child_exit = 1;
                        break;
                    }
                    if (template != NULL)
                    {
                        n_subst(template, arg_count, args);
                        debconf_input(debconf, "critical", template);
                        debconf_go(debconf);
                    }
                    else if (strstr(line, "EF:") == line)
                    {
                        ptr = n_sprintf(line+4, arg_count, args);
                        if (ptr == NULL)
                            return -1;
                        /* fallback error message */
                        debconf_subst(debconf, DEBCONF_BASE "fallback-error",
				      "ERROR", ptr);
                        debconf_input(debconf, "critical",
				      DEBCONF_BASE "fallback-error");
                        debconf_go(debconf);
                        free(ptr);
                    }
                    else
                    {
                        /* err, don't really know what to do here... there
                         * should always be a fallback... */
                    }
                    return -1;
                }
            case 'W':
                {
                    ptr += 3;
                    /* ptr now contains the identifier of the warning */
                    template = find_template("warning", ptr);

                    /* fprintf(stderr, "warning template: %s\n", template); */
		    
                    args = read_arg_lines("WA: ", ifp, &arg_count, &line,
					  &llen);
                    if (args == NULL)
                    {
                        child_exit = 1;
                        break;
                    }
                    if (template != NULL)
                    {
			/* It's hard to choose whether to display a warning
			 * as an error/informational template or as a
			 * progress item. Currently a progress item seems
			 * to fit best with how debootstrap uses warnings
			 * that we care about. */
                        n_subst(template, arg_count, args);
                        debconf_progress_info(debconf, template);
                    }
                    else if (strstr(line, "WF:") == line)
                    {
                        ptr = n_sprintf(line+4, arg_count, args);
                        if (ptr == NULL)
                            return -1;
                        /* Fallback warning message. Unlike the above,
			 * display this as an error, since it could be
			 * arbitrarily bad. */
                        debconf_subst(debconf, DEBCONF_BASE "fallback-warning",
				      "INFO", ptr);
			debconf_subst(debconf, DEBCONF_BASE "fallback-warning",
			              "SECTION", section_text);
                        debconf_input(debconf, "critical",
				      DEBCONF_BASE "fallback-warning");
                        debconf_go(debconf);
                        free(ptr);
                    }
                    else
                    {
                        /* err, don't really know what to do here... there
                         * should always be a fallback... */
                    }

		    break;
                }
            case 'P':
                {
                    int plow = 0, phigh = 0;
                    char what[1024] = "";
		    char *section_template;

                    sscanf(line+3, "%d %d %s", &plow, &phigh, what);
                    if (what[0])
		    {
			int i;
			for (i = 0; waypoints[i].progress_id != NULL; i++)
			{
			    if (strcmp(waypoints[i].progress_id, what) == 0)
			        {
                                    set_progress(i, phigh, plow);

				    /* Get the description of the section
				     * template for this waypoint. */
				    if (current_section == i)
					    break; /* optimisation */
			            current_section = i;
			            section_template = find_template("section", what);
			            if (section_template)
				    {
                                            if (! debconf_metaget(debconf, section_template, "description"))
					    {
                                                free(section_text);
                                                section_text = strdup(debconf->value);
				            }
                                            free(section_template);
			            }

			            break;
				}
			}
		    }
		    
                    args = read_arg_lines("PA: ", ifp, &arg_count, &line,
					  &llen);
                    if (args == NULL)
                    {
                        child_exit = 1;
                        break;
                    }
                    if (strstr(line, "PF:") == line)
		    {
                        /* Does not currently need to do anything;
			 * the implementation of debootstrap could change
			 * though.. */
                    }
		    else
			continue;
		    
                    break;
                }
            case 'I':
                {
                    ptr += 3;
                    /* ptr now contains the identifier of the info */
                    template = find_template("info", ptr);

                    /* fprintf(stderr, "info template: %s\n", template); */
		    
                    if (strcmp(ptr, "basesuccess") == 0 && template != NULL)
                    {
                        /* all done */
                        child_exit = 1;
                        break;
                    }
                    args = read_arg_lines("IA: ", ifp, &arg_count, &line,
					  &llen);
                    if (args == NULL)
                    {
                        child_exit = 1;
                        break;
                    }
                    if (template != NULL)
                    {
                        n_subst(template, arg_count, args);
			debconf_subst(debconf, template,
			              "SECTION", section_text);
                        debconf_progress_info(debconf, template);
                    }
                    else if (strstr(line, "IF:") == line)
                    {
                        ptr = n_sprintf(line+4, arg_count, args);
                        if (ptr == NULL)
                            return -1;
                        /* fallback info message */
                        debconf_subst(debconf, DEBCONF_BASE "fallback-info",
				      "INFO", ptr);
			debconf_subst(debconf, DEBCONF_BASE "fallback-info",
			              "SECTION", section_text);
                        debconf_progress_info(debconf,
					      DEBCONF_BASE "fallback-info");
                        free(ptr);
                    }
                    else
                    {
                        /* err, don't really know what to do here... there
                         * should always be a fallback... */
                    }

		    break;
                }
        }

	if (child_exit)
	    break;

        line = NULL;
	llen = getline(&line, &dummy, ifp);
    }

    if (waitpid(pid, &status, 0) != -1 && (WIFEXITED(status) != 0))
    {
        rv = WEXITSTATUS(status);
        if (rv != 0)
        {
            debconf->commandf(debconf, "SUBST %serror-exitcode EXITCODE %d",
			      DEBCONF_BASE, rv);
            debconf_input(debconf, "critical", DEBCONF_BASE "error-exitcode");
            debconf_go(debconf);
        }
        return rv;
    }
    else
    {
        kill(SIGKILL, pid);
        debconf_input(debconf, "critical", DEBCONF_BASE "error-abnormal");
        debconf_go(debconf);
        return 1;
    }
}

int
main(int argc, char *argv[])
{
    char **args;
    int i;

    di_system_init("run-debootstrap");
    debconf = debconfclient_new();
    args = (char **)malloc(sizeof(char *) * (argc + 1));
    args[0] = "/usr/sbin/debootstrap";
    printf("%s", args[0]);
    for (i = 1; i < argc; i++) {
        args[i] = argv[i];
        printf(" %s", argv[i]);
    }
    printf("\n");
    fflush(stdout);
    args[argc] = NULL;
    return exec_debootstrap(args);
}
