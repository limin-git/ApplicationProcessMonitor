#ifndef APPLICATION_PROCESS_MONITOR_H
#define APPLICATION_PROCESS_MONITOR_H

#include "type_fwd.h"

class ApplicationProcessMonitor
{

public:

    ApplicationProcessMonitor();

    void run();

private:

    void initialize();

    void monitor_application( const std::string& application_name, const KeyValueMap& application_configuration );

    void extract_system_information( std::vector<std::string>& process_list, std::map<std::string, bool>& service_list );

    bool is_application_running( const std::string& application_name );
    bool is_service_running( const std::string& service_name );

    bool get_application_name_from_command_line( const std::string& command_line, std::string& application_name );
    bool get_service_name_from_command_line( const std::string& command_line, std::string& service_name );
    void get_command_name_from_command_line( const std::string& command_line, std::string& command_name );
    std::string convert_configuration_command_line_to_actual_command_line( const std::string& configuration_command_line );

    unsigned int execute_command_line( const std::string& command_line );

private:

    std::vector<std::string>    m_process_list;
    std::map<std::string, bool> m_service_list;

    SectionMap m_application_configuration_map;
    KeyValueMap m_command_map;
    unsigned long m_interval_in_millisecond;
};


#endif //UTILITY_H

