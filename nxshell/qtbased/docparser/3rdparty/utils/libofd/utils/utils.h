#ifndef __UTILS_UTILS_H__
#define __UTILS_UTILS_H__

#include <memory>
#include <tuple>
#include <string>
#include <vector>
#include <sstream>
// for PRIu64
#include <inttypes.h>
#include <math.h>

namespace utils{

    class XMLWriter;
    class XMLElement;
    typedef std::shared_ptr<XMLElement> XMLElementPtr;
    class Zip;
    typedef std::shared_ptr<Zip> ZipPtr;
    
    static const double EPS = 1e-6;

    typedef std::vector<uint64_t> IDArray;
    typedef std::vector<int> IntArray;
    typedef std::vector<double> DoubleArray;
    typedef std::vector<std::string> StringArray;

    std::vector<std::string> SplitString(const std::string& content);

    void SetStringStreamPrecision(std::stringstream &ss, int precision);

    template<typename T, typename... Ts>
    std::unique_ptr<T> make_unique(Ts&&... params){
        return std::unique_ptr<T>(new T(std::forward<Ts>(params)...));
    }


    std::tuple<char*, size_t, bool> ReadFileData(const std::string &filename);
    bool WriteFileData(const std::string &filename, const char *data, size_t dataSize); 

    static inline bool equal(double x, double y) { return fabs(x-y) <= EPS; }

    // IO

    bool FileExist(const std::string &fileName); 
    bool MkdirIfNotExist(const std::string &dirName); 

}

#ifndef likely
#if defined(__GNUC__)
#define likely(x) __builtin_expect((x),1)
#else
#define likely(x) (x)
#endif
#endif


#ifndef unlikely
#if defined(__GNUC__)
#define unlikely(x) __builtin_expect((x),0)
#else
#define unlikely(x) (x)
#endif
#endif

#endif // __UTILS_UTILS_H__
