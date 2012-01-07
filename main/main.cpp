#include "StdAfx.h"
#include "Utility.h"
#include "Configuration.h"


void test_get_process();
void test_get_service_list();

void monitor_process();


int main( int argc, char** argv )
{
    std::string sleep_interval_string = Configuration::instance().get_configuration( CONFIG_SECTION_OPTION, CONFIG_KEY_INTERVAL_IN_SECONDS );
    unsigned long sleep_interval = boost::lexical_cast<unsigned long>( sleep_interval_string );

    while ( true )
    {
        try
        {
            std::cout << "." << std::endl;
            monitor_process();
        }
        catch ( std::exception& e )
        {
            COUT_WARNING( SourceInfo, "caught std::exception in (main): " << e.what() << std::endl );
        }
        catch (...)
        {
            COUT_WARNING( SourceInfo, "caught unknown exception in (main)" << std::endl );
        }

        Sleep( sleep_interval * 1000 );
    }


    return 0;
};


void test_get_process()
{
    std::vector<std::string> process_list = Utility::get_process_list();

    for ( size_t i = 0; i < process_list.size(); ++i )
    {
        std::cout << process_list[i] << std::endl;
    }
}


void test_get_service_list()
{
    std::map<std::string, bool> service_list = Utility::get_service_list();

    for ( std::map<std::string, bool>::iterator it = service_list.begin(); it != service_list.end(); ++it )
    {
        std::cout << it->first << "\t\t" << it->second << std::endl;
    }
}


bool get_application_name_from_command_line( const std::string& command_line, std::string& application_name )
{
    boost::smatch m;
    static const boost::regex application_name_regex( "(?xi) \\w+\\.exe" );
    if ( true == regex_search( command_line, m, application_name_regex ) )
    {
        application_name = m.str( 0 );

        boost::to_lower( application_name );

        return true;
    }

    return false;
}


bool get_service_name_from_command_line( const std::string& command_line, std::string& service_name )
{
    boost::smatch m;
    static const boost::regex application_name_regex( "(?xi) (start [ ] service | stop [ ] service | sc [ ] start | sc [ ] stop) \\s* (\\w+)" );
    if ( true == regex_search( command_line, m, application_name_regex ) )
    {
        service_name = m.str( 2 );

        boost::to_lower( service_name );

        return true;
    }

    return false;
}


void monitor_process()
{

    std::vector<std::string> process_list = Utility::get_process_list();
    std::map<std::string, bool> service_list = Utility::get_service_list();


    SectionMap section_map = Configuration::instance().get_configuration();

    const KeyValueMap& command_map = Configuration::instance().get_configuration( CONFIG_SECTION_COMMAND );

    section_map.erase( CONFIG_SECTION_COMMAND );

    for ( SectionMap::iterator it = section_map.begin(); it != section_map.end(); ++it )
    {
        std::string process_name = it->first;
        boost::to_lower( process_name );
        KeyValueMap& process_configuration  = it->second;

        bool is_running = ( std::find( process_list.begin(), process_list.end(), process_name ) != process_list.end() );

        for ( KeyValueMap::iterator key_it = process_configuration.begin(); key_it != process_configuration.end(); ++key_it )
        {
            const std::string& event_name = key_it->first;
            const std::string& command_line = key_it->second;
            std::string actual_command_line;

            if ( CONFIG_KEY_AFTER_START == event_name )
            {
                if ( true == is_running )
                {
                    actual_command_line = command_line;
                }
            }
            else if ( CONFIG_KEY_AFTER_STOP == event_name )
            {
                if ( false == is_running )
                {
                    actual_command_line = command_line;
                }
            }
            else if ( CONFIG_KEY_AFTER_START_OR_STOP == event_name )
            {
                actual_command_line = command_line;
            }
            else if ( CONFIG_KEY_START_ALWAYS == event_name )
            {
                actual_command_line = command_line;
            }
            else if ( CONFIG_KEY_STOP_ALWAYS == event_name )
            {
                actual_command_line = command_line;
            }
            else
            {
                NULL;
            }

            if ( true == actual_command_line.empty() )
            {
                continue;
            }

            size_t replace_length = 0;
            std::string replace_string;
            std::string to_replace_string;
            for ( KeyValueMap::const_iterator key_it = command_map.begin(); key_it != command_map.end(); ++key_it )
            {
                const std::string& command_name = key_it->first;
                const std::string& command_value = key_it->second;

                // std::cout << command_name << std::endl;

                std::string::size_type pos = actual_command_line.find( command_name );

                if ( 0 == pos )
                {
                    if ( replace_length < command_name.size() )
                    {
                        to_replace_string = command_name;
                        replace_length = command_name.size();
                        replace_string = command_value;
                    }
                }
            }

            if ( replace_length != 0 )
            {
                actual_command_line.replace( 0, replace_length, replace_string );
                boost::trim( actual_command_line );

                if ( CONFIG_KEY_START == to_replace_string )
                {
                    std::string application_name;
                    bool is_application_running = false;

                    if ( true == get_application_name_from_command_line( actual_command_line, application_name ) )
                    {
                        is_application_running = ( std::find( process_list.begin(), process_list.end(), application_name ) != process_list.end() );
                    }

                    if ( true == is_application_running )
                    {
                        actual_command_line = "";
                    }
                }
                else if ( CONFIG_KEY_STOP == to_replace_string )
                {
                    std::string application_name;
                    bool is_application_running = true;

                    if ( true == get_application_name_from_command_line( actual_command_line, application_name ) )
                    {
                        is_application_running = ( std::find( process_list.begin(), process_list.end(), application_name ) != process_list.end() );
                    }

                    if ( false == is_application_running )
                    {
                        actual_command_line = "";
                    }
                }
                else if ( CONFIG_KEY_START_SERVICE == to_replace_string )
                {
                    std::string service_name;
                    bool is_service_running = false;

                    if ( true == get_service_name_from_command_line( actual_command_line, service_name ) )
                    {
                        is_service_running = service_list[ service_name ];
                    }

                    if ( true == is_service_running )
                    {
                        actual_command_line = "";
                    }
                }
                else if ( CONFIG_KEY_STOP_SERVICE == to_replace_string )
                {
                    std::string service_name;
                    bool is_service_running = false;

                    if ( true == get_service_name_from_command_line( actual_command_line, service_name ) )
                    {
                        is_service_running = service_list[ service_name ];
                    }

                    if ( false == is_service_running )
                    {
                        actual_command_line = "";
                    }
                }
                else
                {
                    NULL;
                }
            }

            if ( false == actual_command_line.empty() )
            {
                std::cout << actual_command_line << std::endl;

                WinExec( actual_command_line.c_str(), SW_HIDE );
                // system( actual_command_line.c_str() );
            }

        }
    }

}
