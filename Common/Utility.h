#ifndef UTILITY_H
#define UTILITY_H

#include "type_fwd.h"

class Utility
{

public:

    static std::string get_string_from_file( const std::string& file_path );
    static std::string preferred_path( const std::string& path );

    static std::vector<std::string> get_process_list();
    static std::map<std::string, bool> get_service_list();

private:

    Utility();
    Utility( const Utility& );
    Utility& operator =( const Utility& );

};


#endif //UTILITY_H

