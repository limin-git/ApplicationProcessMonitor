#pragma once
#include "type_fwd.h"


class ApplicationProcessMonitor
{
    enum EConditionType{ ANY, AFTER_START, AFTER_STOP };
    enum ECommandType{ START, STOP, START_SERVICE, STOP_SERVICE };

    struct ApplicationConfiguration
    {
        std::string m_application_name;
        std::string m_service_name;
        EConditionType m_condition_type;
        ECommandType m_command_type;
        std::string m_command_line;
    };

public:

    ApplicationProcessMonitor();

    void run();

private:

    void initialize();
    void monitor_application( const std::string& application_name, const std::vector<ApplicationConfiguration>& application_configuration_list );
    void extract_system_information();
    bool is_application_running( const std::string& application_name );
    bool is_service_running( const std::string& service_name );
    bool is_service_exist( const std::string& service_name );
    std::string get_application_name_from_command_line( const std::string& command_line );
    unsigned int execute_command_line( const std::string& command_line );

private:

    unsigned long m_interval_in_millisecond;

    std::set<std::string>    m_process_list;
    std::map<std::string, bool> m_service_list;

    std::map< std::string, std::vector<ApplicationConfiguration> > m_application_configuration_map;

    std::map<std::string, std::string>    m_command_map;
    std::map<std::string, ECommandType>   m_command_type_map;
    std::map<std::string, EConditionType> m_condition_type_map;
};
