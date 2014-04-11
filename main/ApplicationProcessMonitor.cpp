#include "StdAfx.h"
#include "ApplicationProcessMonitor.h"
#include "Utility.h"
#include "Configuration.h"


ApplicationProcessMonitor::ApplicationProcessMonitor()
    : m_interval_in_millisecond( 0 )
{
    FUNCTION_ENTRY( "ApplicationProcessMonitor" );

    initialize();

    FUNCTION_EXIT;
}


void ApplicationProcessMonitor::run()
{
    FUNCTION_ENTRY( "run" );

    while ( true )
    {
        extract_system_information( m_process_list, m_service_list );

        for ( std::map< std::string, std::vector<ApplicationConfiguration> >::iterator it = m_application_configuration_map.begin(); it != m_application_configuration_map.end(); ++it )
        {
            const std::string& application_name = it->first;
            const std::vector<ApplicationConfiguration>& application_configuration_list  = it->second;

            monitor_application( application_name, application_configuration_list );
        }

        ::Sleep( m_interval_in_millisecond );
        std::cout << "." << std::flush;
    }

    FUNCTION_EXIT;
}


void ApplicationProcessMonitor::initialize()
{
    FUNCTION_ENTRY( "initialize" );

    m_command_map[ "start" ] = "";
    m_command_map[ "stop" ] = "TASKKILL /F /IM ";
    m_command_map[ "start_service" ] = "SC START ";
    m_command_map[ "stop_service" ] = "SC STOP ";

    m_command_type_map[ "start" ] = START;
    m_command_type_map[ "stop" ] = STOP;
    m_command_type_map[ "start_service" ] = START_SERVICE;
    m_command_type_map[ "stop_service" ] = STOP_SERVICE;

    m_condition_type_map[ CONFIG_KEY_AFTER_START ] = AFTER_START;
    m_condition_type_map[ CONFIG_KEY_AFTER_STOP ] = AFTER_STOP;
    m_condition_type_map[ "*" ] = ANY;

    SectionMap section_map = Configuration::instance().get_configuration();
    section_map.erase( CONFIG_SECTION_OPTION );

    for ( SectionMap::iterator it = section_map.begin(); it != section_map.end(); ++it )
    {
        const std::string& application_name = boost::to_lower_copy( it->first );
        const KeyValueMap& configuration_map = it->second;

        for ( KeyValueMap::const_iterator key_it = configuration_map.begin(); key_it != configuration_map.end(); ++key_it )
        {
            const std::string& condition_type = key_it->first;
            const std::string& command_line = key_it->second;

            ApplicationConfiguration application_configuration;

            boost::smatch m;
            static const boost::regex configuration_regex( "(?x) ([\\w]+) \\s+ (.+)" );

            if ( true == regex_match( command_line, m, configuration_regex ) )
            {
                const std::string& command_type = m.str(1);
                const std::string& command = m.str(2);

                application_configuration.m_command_type = m_command_type_map[command_type];
                application_configuration.m_condition_type = m_condition_type_map[condition_type];

                if ( START == application_configuration.m_command_type || STOP == application_configuration.m_command_type )
                {
                    application_configuration.m_application_name = get_application_name_from_command_line( command );
                }
                else if ( START_SERVICE == application_configuration.m_command_type || STOP_SERVICE == application_configuration.m_command_type )
                {
                    application_configuration.m_service_name = boost::to_lower_copy( command );
                }

                application_configuration.m_command_line = m_command_map[command_type] + command;
                m_application_configuration_map[application_name].push_back( application_configuration );
            }
            else
            {
                std::cout << "error: bad configuration:" << "application: " << application_name << " condition type:" << condition_type << " command line: " << command_line << std::endl;
            }
        }
    }

    try
    {
        const std::string& sleep_interval_string = Configuration::instance().get_configuration( CONFIG_SECTION_OPTION, CONFIG_KEY_INTERVAL_IN_SECONDS );
        m_interval_in_millisecond = boost::lexical_cast<unsigned long>( sleep_interval_string ) * 1000;
    }
    catch (const boost::bad_lexical_cast&)
    {
        m_interval_in_millisecond = 60 * 1000;
    }

    FUNCTION_EXIT;
}


void ApplicationProcessMonitor::monitor_application( const std::string& application_name, const std::vector<ApplicationConfiguration>& application_configuration_list )
{
    FUNCTION_ENTRY( "monitor_application" );

    for ( size_t i = 0; i < application_configuration_list.size(); ++i )
    {
        const ApplicationConfiguration& application_configuration = application_configuration_list[i];

        if ( "*" != application_name )
        {
            bool is_running = is_application_running( application_name );

            if ( ( true == application_configuration.m_command_line.empty() ) ||
                 ( AFTER_START == application_configuration.m_condition_type && false == is_running ) ||
                 ( AFTER_STOP == application_configuration.m_condition_type && true == is_running ) )
            {
                continue;
            }
        }

        if ( START == application_configuration.m_command_type || STOP == application_configuration.m_command_type )
        {
            bool is_configuration_application_running = is_application_running( application_configuration.m_application_name );

            if ( ( START == application_configuration.m_command_type && true == is_configuration_application_running ) ||
                 ( STOP == application_configuration.m_command_type && false == is_configuration_application_running ) )
            {
                continue;
            }
        }
        else if ( START_SERVICE == application_configuration.m_command_type || STOP_SERVICE == application_configuration.m_command_type )
        {
            bool is_configuration_service_running = is_service_running( application_configuration.m_service_name );

            if ( ( START_SERVICE == application_configuration.m_command_type && true == is_configuration_service_running ) ||
                 ( STOP_SERVICE == application_configuration.m_command_type && false == is_configuration_service_running ) )
            {
                continue;
            }
        }

        unsigned int result = execute_command_line( application_configuration.m_command_line );

        if ( result <= 31 )
        {
            if ( 0 == result )
            {
                std::cout << "failed to execute command line: " << application_configuration.m_command_line << ", reason: out of memory." << std::endl;
            }
            else if ( ERROR_BAD_FORMAT == result )
            {
                std::cout << "failed to execute command line: " << application_configuration.m_command_line << ", reason: The .exe file is invalid." << std::endl;
            }
            else if ( ERROR_FILE_NOT_FOUND == result )
            {
                std::cout << "failed to execute command line: " << application_configuration.m_command_line << ", reason: The specified file was not found." << std::endl;
            }
            else if ( ERROR_PATH_NOT_FOUND == result )
            {
                std::cout << "failed to execute command line: " << application_configuration.m_command_line << ", reason: The specified path was not found." << std::endl;
            }
        }
    }

    FUNCTION_EXIT;
}


void ApplicationProcessMonitor::extract_system_information( std::set<std::string>& process_list, std::map<std::string, bool>& service_list )
{
    FUNCTION_ENTRY( "extract_system_information" );

    // m_process_list = Utility::get_process_list();
    m_process_list = Utility::get_process_list_2();
    m_service_list = Utility::get_service_list();

    FUNCTION_EXIT;
}


bool ApplicationProcessMonitor::is_application_running( const std::string& application_name )
{
    FUNCTION_ENTRY( "is_application_running" );
    FUNCTION_EXIT;
    return m_process_list.find( application_name ) != m_process_list.end();
}


bool ApplicationProcessMonitor::is_service_running( const std::string& service_name )
{
    FUNCTION_ENTRY( "is_service_running" );
    FUNCTION_EXIT;
    return m_service_list[ service_name ];
}


std::string ApplicationProcessMonitor::get_application_name_from_command_line( const std::string& command_line )
{
    boost::filesystem::path app_path( command_line );
    std::string application_name = boost::to_lower_copy( app_path.filename().string() );
    boost::replace_all( application_name, "\"", "" );
    return application_name;
}


unsigned int ApplicationProcessMonitor::execute_command_line( const std::string& command_line )
{
    FUNCTION_ENTRY( "execute_command_line" );

    Utility::log_to_file( command_line );
    
    FUNCTION_EXIT;
    return ::WinExec( command_line.c_str(), SW_HIDE );
}
