#include "gbm_adaptor.h"

#ifndef SUPPORT_FULL_GBM

/* The two GBM_BO_FORMAT_[XA]RGB8888 formats alias the GBM_FORMAT_*
 * formats of the same name. We want to accept them whenever someone
 * has a GBM format, but never return them to the user. */
uint32_t gbm_format_canonicalize(uint32_t gbm_format)
{
   switch (gbm_format) {
   case GBM_BO_FORMAT_XRGB8888:
      return GBM_FORMAT_XRGB8888;
   case GBM_BO_FORMAT_ARGB8888:
      return GBM_FORMAT_ARGB8888;
   default:
      return gbm_format;
   }
}

/**
 * Returns a string representing the fourcc format name.
 *
 * \param desc Caller-provided storage for the format name string.
 * \return String containing the fourcc of the format.
 */
char *gbm_format_get_name(uint32_t gbm_format, struct gbm_format_name_desc *desc)
{
   gbm_format = gbm_format_canonicalize(gbm_format);

   desc->name[0] = gbm_format;
   desc->name[1] = gbm_format >> 8;
   desc->name[2] = gbm_format >> 16;
   desc->name[3] = gbm_format >> 24;
   desc->name[4] = 0;

   return desc->name;
}

#endif