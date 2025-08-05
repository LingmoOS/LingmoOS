
/*
 * ISO-8859-1
 */

static int
iso8859_1_mbtowc (ucs4_t *pwc, const unsigned char *s, int n)
{
  unsigned char c = *s;
  *pwc = (ucs4_t) c;
  return 1;
}

static int
iso8859_1_wctomb (unsigned char *r, ucs4_t wc, int n)
{
  if (wc < 0x0100) {
    *r = wc;
    return 1;
  }
  return RET_ILSEQ;
}
