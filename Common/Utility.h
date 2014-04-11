#pragma once
#include "type_fwd.h"

class Utility
{
public:

    static std::string get_string_from_file( const std::string& file_path );
    static std::string preferred_path( const std::string& path );
    static std::string get_current_time();
    static void log_to_file( const std::string& s );

    static std::set<std::string> get_process_list();
    static std::map<std::string, bool> get_service_list();

private:

    Utility();
};
