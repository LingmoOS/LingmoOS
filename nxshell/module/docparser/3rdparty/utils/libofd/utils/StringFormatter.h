#ifndef __UTILS_STRINGFORMATTER_H__
#define __UTILS_STRINGFORMATTER_H__

#include <vector>
#include <cstdio>

namespace utils {

    class StringFormatter {

        public:
            struct GuardedPointer {
                GuardedPointer(StringFormatter * sf) : sf(sf) { ++(sf->buf_cnt); }
                GuardedPointer(const GuardedPointer & gp) : sf(gp.sf) { ++(sf->buf_cnt); }
                ~GuardedPointer(void) { --(sf->buf_cnt); }
                operator char* () const { return &(sf->buf.front()); }

                private:
                StringFormatter * sf;
            };

            StringFormatter() : buf_cnt(0) { buf.reserve(L_tmpnam); }
            /*
             * Important:
             * there is only one buffer, so new strings will replace old ones
             */
            GuardedPointer operator () (const char * format, ...);

        private:
            friend struct GuardedPointer;
            std::vector<char> buf;
            int buf_cnt;
    };

} //namespace utils

#endif // __UTILS_STRINGFORMATTER_H__
