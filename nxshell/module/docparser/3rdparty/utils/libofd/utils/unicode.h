#ifndef __UTILS_UNICODE_H__
#define __UTILS_UNICODE_H__

int enc_unicode_to_utf8_one(unsigned long unic, unsigned char *pOutput, int outSize);  

int enc_utf8_to_unicode_one(const unsigned char* pInput, unsigned long *Unic);


#endif // __UTILS_UNICODE_H__
