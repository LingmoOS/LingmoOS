#include <sys/stat.h> /* mkdir() */
#include <unistd.h> /* write(), close(), access() */
#include <sstream>
#include <iterator>
#include <iostream>
#include <fstream>
#include "utils.h"

namespace utils{

    std::vector<std::string> SplitString(const std::string& content){
        std::istringstream iss(content);
        std::vector<std::string> tokens{std::istream_iterator<std::string>{iss}, 
            std::istream_iterator<std::string>{}};
        return tokens;
    }

    void SetStringStreamPrecision(std::stringstream &ss, int precision){
        ss.setf(std::ios::fixed, std::ios::floatfield); \
            ss.precision(precision);
    }



    std::tuple<char*, size_t, bool> ReadFileData(const std::string &filename){
        bool ok = false;
        char *fontData = nullptr;
        size_t fontDataSize = 0;

        std::ifstream ifile(filename, std::ios::binary);

        ifile.seekg(0, std::ios::end);
        fontDataSize = ifile.tellg();
//        LOG(INFO) << "filename: " << filename << " len: " << fontDataSize;

        fontData = new char[fontDataSize];
        ifile.seekg(0, std::ios::beg);
        ifile.read(fontData, fontDataSize);

        ok = true;

        ifile.close();

        return std::make_tuple(fontData, fontDataSize, ok);
    }


    bool WriteFileData(const std::string &filename, const char *data, size_t dataSize){
        bool ok = true;

        std::ofstream ofile;
        ofile.open(filename, std::ios::binary | std::ios::trunc);
        ofile.write(data, dataSize);
        ofile.close();

        return ok;
    }

    bool FileExist(const std::string &fileName) {
        if ( access(fileName.c_str(), F_OK) == 0 ){
            return true;
        } else {
            return false;
        }
    }

    bool MkdirIfNotExist(const std::string &dirName) {
        if ( !FileExist(dirName) ) {
            return mkdir(dirName.c_str(), 0750);
        } 
        return true;
    }

}


