#include <string>
#include <uuid/uuid.h>

static std::string UUID_FORMAT36 = "%X%X%X%X-%X%X-%X%X-%X%X-%X%X%X%X%X%X";
static std::string UUID_FORMAT32 = "%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X";

std::string generate_uuid(const std::string &uuidFormat){
    uuid_t id;
    uuid_generate((unsigned char *)&id); 

    char buf[33];
    sprintf(buf, uuidFormat.c_str(), 
            id[0], id[1], id[2], id[3],
            id[4], id[5], id[6], id[7], id[8], id[9],
            id[10], id[11], id[12], id[13], id[14], id[15]);
    buf[32] = '\0';

    return std::string(buf);
}

std::string generate_uuid32(){
    return generate_uuid(UUID_FORMAT32);
}

std::string generate_uuid36(){
    return generate_uuid(UUID_FORMAT36);
}
