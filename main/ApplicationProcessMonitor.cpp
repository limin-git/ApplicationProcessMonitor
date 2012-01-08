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
        Sleep( m_interval_in_millisecond );
        std::cout << "." << std::endl;

        extract_system_information( m_process_list, m_service_list );

        for ( SectionMap::const_iterator it = m_application_configuration_map.begin(); it != m_application_configuration_map.end(); ++it )
        {
            std::string application_name = it->first;
            boost::to_lower( application_name );

            const KeyValueMap& application_configuration  = it->second;

            monitor_application( application_name, application_configuration );
        }
    }

    FUNCTION_EXIT;
}


void ApplicationProcessMonitor::initialize()
{
    FUNCTION_ENTRY( "initialize" );

    m_application_configuration_map = Configuration::instance().get_configuration();
    m_application_configuration_map.erase( CONFIG_SECTION_COMMAND );
    m_application_configuration_map.erase( CONFIG_SECTION_OPTION );

    m_command_map = Configuration::instance().get_configuration( CONFIG_SECTION_COMMAND );

    std::string sleep_interval_string = Configuration::instance().get_configuration( CONFIG_SECTION_OPTION, CONFIG_KEY_INTERVAL_IN_SECONDS );

    try
    {
        m_interval_in_millisecond = boost::lexical_cast<unsigned long>( sleep_interval_string ) * 1000;
    }
    catch (const boost::bad_lexical_cast&)
    {
        m_interval_in_millisecond = 60 * 1000;
    }

    FUNCTION_EXIT;
}


void ApplicationProcessMonitor::extract_system_information( std::vector<std::string>& process_list, std::map<std::string, bool>& service_list )
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
    return ( std::find( m_process_list.begin(), m_process_list.end(), application_name ) != m_process_list.end() );
}


bool ApplicationProcessMonitor::is_service_running( const std::string& service_name )
{
    FUNCTION_ENTRY( "is_service_running" );
    FUNCTION_EXIT;
    return m_service_list[ service_name ];
}


bool ApplicationProcessMonitor::get_application_name_from_command_line( const std::string& command_line, std::string& application_name )
{
    FUNCTION_ENTRY( "get_application_name_from_command_line" );

    boost::smatch m;
    static const boost::regex application_name_regex( "(?xi) \\w+\\.exe" );
    if ( true == regex_search( command_line, m, application_name_regex ) )
    {
        application_name = m.str( 0 );

        boost::to_lower( application_name );

        FUNCTION_EXIT;
        return true;
    }

    FUNCTION_EXIT;
    return false;
}


bool ApplicationProcessMonitor::get_service_name_from_command_line( const std::string& command_line, std::string& service_name )
{
    FUNCTION_ENTRY( "get_service_name_from_command_line" );

    boost::smatch m;
    static const boost::regex application_name_regex( "(?xi) (start [ ] service | stop [ ] service | sc [ ] start | sc [ ] stop) \\s* (\\w+)" );
    if ( true == regex_search( command_line, m, application_name_regex ) )
    {
        service_name = m.str( 2 );

        boost::to_lower( service_name );

        FUNCTION_EXIT;
        return true;
    }

    FUNCTION_EXIT;
    return false;
}


void ApplicationProcessMonitor::get_command_name_from_command_line( const std::string& command_line, std::string& command_name )
{
    FUNCTION_ENTRY( "get_command_name_from_command_line" );

    command_name.clear();

    for ( KeyValueMap::const_iterator key_it = m_command_map.begin(); key_it != m_command_map.end(); ++key_it )
    {
        const std::string& current_command_name = key_it->first;

        std::string::size_type pos = command_line.find( current_command_name );

        if ( 0 == pos )
        {
            if ( command_name.size() < current_command_name.size() )
            {
                command_name = current_command_name;
            }
        }
    }

    FUNCTION_EXIT;
}


std::string ApplicationProcessMonitor::convert_configuration_command_line_to_actual_command_line( const std::string& configuration_command_line )
{
    FUNCTION_ENTRY( "convert_configuration_command_line_to_actual_command_line" );

    std::string command_name;
    get_command_name_from_command_line( configuration_command_line, command_name );

    if ( true == command_name.empty() )
    {
        FUNCTION_EXIT;
        return "";
    }

    if ( m_command_map.find( command_name ) == m_command_map.end() )
    {
        FUNCTION_EXIT;
        return "";
    }

    std::string actual_command_line = configuration_command_line;
    actual_command_line.replace( 0, command_name.size(), m_command_map[command_name] );
    boost::trim( actual_command_line );

    FUNCTION_EXIT;
    return actual_command_line;
}


void ApplicationProcessMonitor::monitor_application( const std::string& application_name, const KeyValueMap& application_configuration )
{
    FUNCTION_ENTRY( "monitor_application" );

    bool is_running = is_application_running( application_name );

    for ( KeyValueMap::const_iterator it = application_configuration.begin(); it != application_configuration.end(); ++it )
    {
        const std::string& condition_name = it->first;
        const std::string& configuration_command_line = it->second;

        if ( ( true == configuration_command_line.empty() ) ||
             ( CONFIG_KEY_AFTER_START == condition_name && false == is_running ) ||
             ( CONFIG_KEY_AFTER_STOP == condition_name && true == is_running ) )
        {
            continue;
        }

        std::string command_name;
        get_command_name_from_command_line( configuration_command_line, command_name );

        if ( command_name.empty() )
        {
            continue;
        }

        if ( CONFIG_KEY_START == command_name || CONFIG_KEY_STOP == command_name )
        {
            std::string configuration_application_name;

            if ( false == get_application_name_from_command_line( configuration_command_line, configuration_application_name ) )
            {
                continue;
            }

            bool is_configuration_application_running = is_application_running( configuration_application_name );

            if ( ( CONFIG_KEY_START == command_name && true == is_configuration_application_running ) ||
                 ( CONFIG_KEY_STOP == command_name && false == is_configuration_application_running ) )
            {
                continue;
            }
        }
        else if ( CONFIG_KEY_START_SERVICE == command_name || CONFIG_KEY_STOP_SERVICE == command_name )
        {
            std::string configuration_service_name;

            if ( false == get_service_name_from_command_line( configuration_command_line, configuration_service_name ) )
            {
                continue;
            }

            bool is_configuration_service_running = is_service_running( configuration_service_name );

            if ( ( CONFIG_KEY_START_SERVICE == command_name && true == is_configuration_service_running ) ||
                 ( CONFIG_KEY_STOP_SERVICE == command_name && false == is_configuration_service_running ) )
            {
                continue;
            }
        }
        else
        {
            NULL;
        }

        std::string actual_command_line = convert_configuration_command_line_to_actual_command_line( configuration_command_line );

        if ( true == actual_command_line.empty() )
        {
            continue;
        }

        execute_command_line( actual_command_line );
    }

    FUNCTION_EXIT;
}


unsigned int ApplicationProcessMonitor::execute_command_line( const std::string& command_line )
{
    FUNCTION_ENTRY( "execute_command_line" );
    FUNCTION_EXIT;
    return WinExec( command_line.c_str(), SW_HIDE );
}

