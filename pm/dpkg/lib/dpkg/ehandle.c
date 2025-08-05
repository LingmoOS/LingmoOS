/*
 * libdpkg - Debian packaging suite library routines
 * ehandle.c - error handling
 *
 * Copyright © 1994,1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2008-2014 Guillem Jover <guillem@debian.org>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <compat.h>

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/macros.h>
#include <dpkg/i18n.h>
#include <dpkg/progname.h>
#include <dpkg/color.h>
#include <dpkg/ehandle.h>

/* Incremented when we do some kind of generally necessary operation,
 * so that loops &c know to quit if we take an error exit. Decremented
 * again afterwards. */
volatile int onerr_abort = 0;

#define NCALLS 2

struct cleanup_entry {
  struct cleanup_entry *next;
  struct {
    int mask;
    void (*call)(int argc, void **argv);
  } calls[NCALLS];
  int cpmask, cpvalue;
  int argc;
  void *argv[1];
};

struct error_context {
  struct error_context *next;

  enum {
    HANDLER_TYPE_FUNC,
    HANDLER_TYPE_JUMP,
  } handler_type;

  union {
    error_handler_func *func;
    jmp_buf *jump;
  } handler;

  struct {
    error_printer_func *func;
    const void *data;
  } printer;

  struct cleanup_entry *cleanups;

  char *errmsg;
};

static struct error_context *volatile econtext = NULL;

/**
 * Emergency variables.
 *
 * These are used when the system is out of resources, and we want to be able
 * to proceed anyway at least to the point of a controlled shutdown.
 */
static struct {
  struct cleanup_entry ce;
  void *args[20];

  /**
   * Emergency error message buffer.
   *
   * The size is estimated using the following heuristic:
   * - 6x255 For inserted strings (%.255s &c in fmt; and %s with limited
   *         length arg).
   * - 1x255 For constant text.
   * - 1x255 For strerror().
   * - And the total doubled just in case.
   */
  char errmsg[4096];
} emergency;

/**
 * Default fatal error handler.
 *
 * This handler performs all error unwinding for the current context, and
 * terminates the program with an error exit code.
 */
void
catch_fatal_error(void)
{
  pop_error_context(ehflag_bombout);
  exit(2);
}

void
print_fatal_error(const char *emsg, const void *data)
{
  fprintf(stderr, "%s%s:%s %s%s:%s %s\n",
          color_get(COLOR_PROG), dpkg_get_progname(), color_reset(),
          color_get(COLOR_ERROR), _("error"), color_reset(), emsg);
}

static void
print_abort_error(const char *etype, const char *emsg)
{
  fprintf(stderr, _("%s%s%s: %s%s:%s\n %s\n"),
          color_get(COLOR_PROG), dpkg_get_progname(), color_reset(),
          color_get(COLOR_ERROR), etype, color_reset(), emsg);
}

static struct error_context *
error_context_new(void)
{
  struct error_context *necp;

  necp = malloc(sizeof(*necp));
  if (!necp)
    ohshite(_("out of memory for new error context"));
  necp->next= econtext;
  necp->cleanups= NULL;
  necp->errmsg = NULL;
  econtext= necp;

  return necp;
}

static void
set_error_printer(struct error_context *ec, error_printer_func *func,
                  const void *data)
{
  ec->printer.func = func;
  ec->printer.data = data;
}

static void
set_func_handler(struct error_context *ec, error_handler_func *func)
{
  ec->handler_type = HANDLER_TYPE_FUNC;
  ec->handler.func = func;
}

static void
set_jump_handler(struct error_context *ec, jmp_buf *jump)
{
  ec->handler_type = HANDLER_TYPE_JUMP;
  ec->handler.jump = jump;
}

static void
error_context_errmsg_free(struct error_context *ec)
{
  if (ec->errmsg != emergency.errmsg)
    free(ec->errmsg);
  ec->errmsg = NULL;
}

static void
error_context_errmsg_set(struct error_context *ec, char *errmsg)
{
  error_context_errmsg_free(ec);
  ec->errmsg = errmsg;
}

static int DPKG_ATTR_VPRINTF(1)
error_context_errmsg_format(const char *fmt, va_list args)
{
  va_list args_copy;
  char *errmsg = NULL;
  int rc;

  va_copy(args_copy, args);
  rc = vasprintf(&errmsg, fmt, args_copy);
  va_end(args_copy);

  /* If the message was constructed successfully, at least we have some
   * error message, which is better than nothing. */
  if (rc >= 0)
    error_context_errmsg_set(econtext, errmsg);

  if (rc < 0) {
    /* If there was any error, just use the emergency error message buffer,
     * even if it ends up being truncated, at least we will have a big part
     * of the problem. */
    rc = vsnprintf(emergency.errmsg, sizeof(emergency.errmsg), fmt, args);

    /* Return failure only if we get truncated. */
    if (rc >= (int)sizeof(emergency.errmsg))
      rc = -1;

    error_context_errmsg_set(econtext, emergency.errmsg);
  }

  return rc;
}

void
push_error_context_func(error_handler_func *handler,
                        error_printer_func *printer,
                        const void *printer_data)
{
  struct error_context *ec;

  ec = error_context_new();
  set_error_printer(ec, printer, printer_data);
  set_func_handler(ec, handler);
  onerr_abort = 0;
}

void
push_error_context_jump(jmp_buf *jumper,
                        error_printer_func *printer,
                        const void *printer_data)
{
  struct error_context *ec;

  ec = error_context_new();
  set_error_printer(ec, printer, printer_data);
  set_jump_handler(ec, jumper);
  onerr_abort = 0;
}

void
push_error_context(void)
{
  push_error_context_func(catch_fatal_error, print_fatal_error, NULL);
}

static void
print_cleanup_error(const char *emsg, const void *data)
{
  print_abort_error(_("error while cleaning up"), emsg);
}

static void
run_cleanups(struct error_context *econ, int flagsetin)
{
  static volatile int preventrecurse= 0;
  struct cleanup_entry *volatile cep;
  struct cleanup_entry *ncep;
  struct error_context recurserr, *oldecontext;
  jmp_buf recurse_jump;
  volatile int i, flagset;

  if (econ->printer.func)
    econ->printer.func(econ->errmsg, econ->printer.data);

  if (++preventrecurse > 3) {
    onerr_abort++;
    print_cleanup_error(_("too many nested errors during error recovery"), NULL);
    flagset= 0;
  } else {
    flagset= flagsetin;
  }
  cep= econ->cleanups;
  oldecontext= econtext;
  while (cep) {
    for (i=0; i<NCALLS; i++) {
      if (cep->calls[i].call && cep->calls[i].mask & flagset) {
        if (setjmp(recurse_jump)) {
          run_cleanups(&recurserr, ehflag_bombout | ehflag_recursiveerror);
          error_context_errmsg_free(&recurserr);
        } else {
          memset(&recurserr, 0, sizeof(recurserr));
          set_error_printer(&recurserr, print_cleanup_error, NULL);
          set_jump_handler(&recurserr, &recurse_jump);
          econtext= &recurserr;
          cep->calls[i].call(cep->argc,cep->argv);
        }
        econtext= oldecontext;
      }
    }
    flagset &= cep->cpmask;
    flagset |= cep->cpvalue;
    ncep= cep->next;
    if (cep != &emergency.ce) free(cep);
    cep= ncep;
  }
  preventrecurse--;
}

/**
 * Push an error cleanup checkpoint.
 *
 * This will arrange that when pop_error_context() is called, all previous
 * cleanups will be executed with
 *   flagset = (original_flagset & mask) | value
 * where original_flagset is the argument to pop_error_context() (as
 * modified by any checkpoint which was pushed later).
 */
void push_checkpoint(int mask, int value) {
  struct cleanup_entry *cep;
  int i;

  cep = malloc(sizeof(*cep) + sizeof(void *));
  if (cep == NULL) {
    onerr_abort++;
    ohshite(_("out of memory for new cleanup entry"));
  }

  for (i=0; i<NCALLS; i++) { cep->calls[i].call=NULL; cep->calls[i].mask=0; }
  cep->cpmask= mask; cep->cpvalue= value;
  cep->argc= 0; cep->argv[0]= NULL;
  cep->next= econtext->cleanups;
  econtext->cleanups= cep;
}

static void
cleanup_entry_new(void (*call1)(int argc, void **argv), int mask1,
                  void (*call2)(int argc, void **argv), int mask2,
                  size_t nargs, va_list vargs)
{
  struct cleanup_entry *cep;
  void **argv;
  int e = 0;
  va_list args;

  onerr_abort++;

  cep = malloc(sizeof(*cep) + sizeof(void *) * (nargs + 1));
  if (!cep) {
    if (nargs > array_count(emergency.args))
      ohshite(_("out of memory for new cleanup entry with many arguments"));
    e= errno; cep= &emergency.ce;
  }
  cep->calls[0].call= call1; cep->calls[0].mask= mask1;
  cep->calls[1].call= call2; cep->calls[1].mask= mask2;
  cep->cpmask=~0; cep->cpvalue=0; cep->argc= nargs;

  va_copy(args, vargs);
  argv = cep->argv;
  while (nargs-- > 0)
    *argv++ = va_arg(args, void *);
  *argv++ = NULL;
  va_end(args);
  cep->next= econtext->cleanups;
  econtext->cleanups= cep;
  if (cep == &emergency.ce) {
    errno = e;
    ohshite(_("out of memory for new cleanup entry"));
  }

  onerr_abort--;
}

void
push_cleanup(void (*call)(int argc, void **argv), int mask,
             size_t nargs, ...)
{
  va_list args;

  va_start(args, nargs);
  cleanup_entry_new(call, mask, NULL, 0, nargs, args);
  va_end(args);
}

void
push_cleanup_fallback(void (*call1)(int argc, void **argv), int mask1,
                      void (*call2)(int argc, void **argv), int mask2,
                      size_t nargs, ...)
{
  va_list args;

  va_start(args, nargs);
  cleanup_entry_new(call1, mask1, call2, mask2, nargs, args);
  va_end(args);
}

void pop_cleanup(int flagset) {
  struct cleanup_entry *cep;
  int i;

  cep= econtext->cleanups;
  econtext->cleanups= cep->next;
  for (i=0; i<NCALLS; i++) {
    if (cep->calls[i].call && cep->calls[i].mask & flagset)
      cep->calls[i].call(cep->argc,cep->argv);
  }
  if (cep != &emergency.ce) free(cep);
}

/**
 * Unwind the current error context by running its registered cleanups.
 */
void
pop_error_context(int flagset)
{
  struct error_context *tecp;

  tecp = econtext;
  econtext = tecp->next;

  /* If we are cleaning up normally, do not print anything. */
  if (flagset & ehflag_normaltidy)
    set_error_printer(tecp, NULL, NULL);
  run_cleanups(tecp, flagset);

  error_context_errmsg_free(tecp);
  free(tecp);
}

static void DPKG_ATTR_NORET
run_error_handler(void)
{
  if (onerr_abort) {
    /* We arrived here due to a fatal error from which we cannot recover,
     * and trying to do so would most probably get us here again. That's
     * why we will not try to do any error unwinding either. We'll just
     * abort. Hopefully the user can fix the situation (out of disk, out
     * of memory, etc). */
    print_abort_error(_("unrecoverable fatal error, aborting"), econtext->errmsg);
    error_context_errmsg_free(econtext);
    exit(2);
  }

  if (econtext == NULL) {
    print_abort_error(_("outside error context, aborting"),
                      _("an error occurred with no error handling in place"));
    exit(2);
  } else if (econtext->handler_type == HANDLER_TYPE_FUNC) {
    econtext->handler.func();
    internerr("error handler returned unexpectedly!");
  } else if (econtext->handler_type == HANDLER_TYPE_JUMP) {
    longjmp(*econtext->handler.jump, 1);
  } else {
    internerr("unknown error handler type %d!", econtext->handler_type);
  }
}

void ohshit(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  error_context_errmsg_format(fmt, args);
  va_end(args);

  run_error_handler();
}

void
ohshitv(const char *fmt, va_list args)
{
  error_context_errmsg_format(fmt, args);

  run_error_handler();
}

void ohshite(const char *fmt, ...) {
  int e, rc;
  va_list args;

  e=errno;

  /* First we construct the formatted message. */
  va_start(args, fmt);
  rc = error_context_errmsg_format(fmt, args);
  va_end(args);

  /* Then if there was no error we append the string for errno. Otherwise
   * we just use the emergency error message buffer, and ignore the errno
   * value, as we will probably have no space left anyway. */
  if (rc > 0) {
    char *errmsg = NULL;

    rc = asprintf(&errmsg, "%s: %s", econtext->errmsg, strerror(e));
    if (rc > 0)
      error_context_errmsg_set(econtext, errmsg);
  }

  run_error_handler();
}

void
do_internerr(const char *file, int line, const char *func, const char *fmt, ...)
{
  va_list args;

  va_start(args, fmt);
  error_context_errmsg_format(fmt, args);
  va_end(args);

  fprintf(stderr, "%s%s:%s:%d:%s:%s %s%s:%s %s\n", color_get(COLOR_PROG),
          dpkg_get_progname(), file, line, func, color_reset(),
          color_get(COLOR_ERROR), _("internal error"), color_reset(),
          econtext->errmsg);

  error_context_errmsg_free(econtext);

  abort();
}
