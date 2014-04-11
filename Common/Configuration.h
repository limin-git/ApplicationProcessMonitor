#pragma once
#include "type_fwd.h"


class Configuration
{
public:

    static Configuration& instance();

    std::string& get_configuration( const std::string& section, const std::string& key );
    KeyValueMap& get_configuration( const std::string& section );
    SectionMap& get_configuration();

private:

    Configuration( const std::string& ini_file_name = CONFIG_INI );
    static void parse_ini_file( const std::string& ini_file_name, SectionMap& section_map );

private:

    SectionMap m_section_map;
    std::string m_ini_file_name;
};
