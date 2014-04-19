#include "StdAfx.h"
#include "ApplicationProcessMonitor.h"
#include "Utility.h"
#include "Configuration.h"


ApplicationProcessMonitor::ApplicationProcessMonitor()
    : m_interval_in_millisecond( 0 )
{
    initialize();
}


void ApplicationProcessMonitor::run()
{
    while ( true )
    {
        extract_system_information();

        for ( std::map< std::string, std::vector<ApplicationConfiguration> >::iterator it = m_application_configuration_map.begin(); it != m_application_configuration_map.end(); ++it )
        {
            const std::string& application_name = it->first;
            const std::vector<ApplicationConfiguration>& application_configuration_list  = it->second;

            monitor_application( application_name, application_configuration_list );
        }

        ::Sleep( m_interval_in_millisecond );
        std::cout << "." << std::flush;
    }
}


void ApplicationProcessMonitor::initialize()
{
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

    SectionMap& section_map = Configuration::instance().get_configuration();
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
}


void ApplicationProcessMonitor::monitor_application( const std::string& application_name, const std::vector<ApplicationConfiguration>& application_configuration_list )
{
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
            if ( false == is_service_exist( application_configuration.m_service_name ) )
            {
                continue;
            }

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
}


void ApplicationProcessMonitor::extract_system_information()
{
    m_process_list = Utility::get_process_list();
    m_service_list = Utility::get_service_list();
}


bool ApplicationProcessMonitor::is_application_running( const std::string& application_name )
{
    return m_process_list.find( application_name ) != m_process_list.end();
}


bool ApplicationProcessMonitor::is_service_running( const std::string& service_name )
{
    std::map<std::string, bool>::iterator find_it = m_service_list.find( service_name );

    if ( find_it != m_service_list.end() )
    {
        return find_it->second;
    }

    return false;
}


bool ApplicationProcessMonitor::is_service_exist( const std::string& service_name )
{
    return ( m_service_list.find( service_name ) != m_service_list.end() );
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
    Utility::log_to_file( command_line );
    return ::WinExec( command_line.c_str(), SW_HIDE );
}



/////////////////////////////////////////////////////////////////////////////





struct IAppStateExp
{
    virtual ~IAppStateExp() {}
    virtual bool isTrue() = 0;
};

typedef boost::shared_ptr<IAppStateExp> IAppStateExpPtr;



struct IApplicationObserver
{
    virtual void state_changed( bool is_running ) = 0;
};

typedef std::vector<IApplicationObserver*> IApplicationObserverList;

struct IApplicationSubject
{
    virtual void add_application_observer( IApplicationObserver* observer, const std::string& application_name ) = 0;
};


struct AppIsRunning : IAppStateExp, IApplicationObserver
{
    AppIsRunning( const std::string& application_name )
        : m_application_name( application_name )
    {
        // TODO: register observer
    }

    virtual bool isTrue()
    {
        return m_is_true;
    }

    virtual void state_changed( bool is_running )
    {
        m_is_true = ( true == is_running );
    }

    bool m_is_true;
    std::string m_application_name;
};


struct AppIsStopped : IAppStateExp, IApplicationObserver
{
    AppIsStopped( const std::string& application_name )
        : m_application_name( application_name )
    {
        // TODO: register observer
    }

    virtual bool isTrue()
    {
        return m_is_true;
    }

    virtual void state_changed( bool is_running )
    {
        m_is_true = ( false == is_running );
    }

    bool m_is_true;
    std::string m_application_name;
};


struct AppStateAndExp : IAppStateExp
{
    AppStateAndExp( IAppStateExp* left, IAppStateExp* right )
        : m_left( left ),
          m_right( right )
    {
    }

    virtual bool isTrue()
    {
        return ( m_left->isTrue() && m_right->isTrue() );
    }

    IAppStateExp* m_left;
    IAppStateExp* m_right;
};





struct ConditionedCommand
{
    ConditionedCommand( const std::string& command, IAppStateExpPtr exp )
        : m_command( command ),
          m_exp( exp ),
          m_is_error( false )
    {
    }

    void do_command()
    {
        if ( false == m_is_error && true == m_exp->isTrue() )
        {
            UINT ret = ::WinExec( m_command.c_str(), SW_HIDE );

            if ( ret <= 31 )
            {
                m_is_error = true;

                if ( 0 == ret )
                {
                    std::cout << "failed to execute command line: " << m_command << ", reason: out of memory." << std::endl;
                }
                else if ( ERROR_BAD_FORMAT == ret )
                {
                    std::cout << "failed to execute command line: " << m_command << ", reason: The .exe file is invalid." << std::endl;
                }
                else if ( ERROR_FILE_NOT_FOUND == ret )
                {
                    std::cout << "failed to execute command line: " << m_command << ", reason: The specified file was not found." << std::endl;
                }
                else if ( ERROR_PATH_NOT_FOUND == ret )
                {
                    std::cout << "failed to execute command line: " << m_command << ", reason: The specified path was not found." << std::endl;
                }
            }
        }
    }

    bool m_is_error;
    std::string m_command;
    IAppStateExpPtr m_exp;
};

typedef boost::shared_ptr<ConditionedCommand> ConditionedCommandPtr;
typedef std::vector<ConditionedCommandPtr> ConditionedCommandPtrList;



struct ITaskMgrObserver
{
    virtual void task_manager_begin() = 0;
    virtual void task_manager_end() = 0;
};






struct ApplicationMonitor
{
    void add_command( ConditionedCommandPtr command )
    {
        m_commands.push_back( command );
    }

    virtual void task_manager_begin()
    {
    }

    virtual void task_manager_end()
    {
        for ( size_t i = 0; i < m_commands.size(); ++i )
        {
            m_commands[i]->do_command();
        }
    }

    ConditionedCommandPtrList m_commands;
};



struct Application
{
    void add_observer( IApplicationObserver* observer )
    {
        m_observers.push_back( observer );
    }

    void update( bool is_running )
    {
        if ( m_is_running != is_running )
        {
            m_is_running = is_running;
            m_is_need_notify = true;
        }

        m_is_updated = true;
    }

    bool is_updated()
    {
        return m_is_updated;
    }

    void update_reset()
    {
        m_is_updated = false;
        m_is_need_notify = false;
    }

    void notify_for_the_first_time()
    {
        for ( size_t i = 0; i < m_observers.size(); ++i )
        {
            m_observers[i]->state_changed( m_is_running );
        }
    }

    void notify()
    {
        for ( size_t i = 0; i < m_observers.size(); ++i )
        {
            if ( m_is_need_notify )
            {
                m_observers[i]->state_changed( m_is_running );
            }
        }
    }

    std::string m_name;
    bool m_is_running;

    bool m_is_updated;
    bool m_is_need_notify;

    IApplicationObserverList m_observers;
};






struct TaskManager : IApplicationSubject
{
    static TaskManager& instance()
    {
        static TaskManager m_instance;
        return m_instance;
    }

    void add_observer( ITaskMgrObserver* observer )
    {
        m_observer = observer;
    }

    virtual void add_application_observer( IApplicationObserver* observer, const std::string& application_name )
    {
        m_application_map[application_name].add_observer( observer );
    }

    void update()
    {
        const std::set<std::string>& process_list = Utility::get_process_list();

        for ( std::map<std::string, Application>::iterator it = m_application_map.begin(); it != m_application_map.end(); ++it )
        {
            it->second.update_reset();
        }

        for ( std::set<std::string>::const_iterator it = process_list.begin(); it != process_list.end(); ++it )
        {
            const std::string& application_name = *it;
            std::map<std::string, Application>::iterator find_it = m_application_map.find( application_name );

            if ( find_it != m_application_map.end() )
            {
                find_it->second.update( true );
            }
        }

        for ( std::map<std::string, Application>::iterator it = m_application_map.begin(); it != m_application_map.end(); ++it )
        {
            if ( false == it->second.is_updated() )
            {
                it->second.update( false );
            }
        }
    }

    void notify()
    {
        for ( std::map<std::string, Application>::iterator it = m_application_map.begin(); it != m_application_map.end(); ++it )
        {
            it->second.notify();
        }
    }

    void run()
    {
        while ( true )
        {
            m_observer->task_manager_begin();

            update();
            notify();
         
            m_observer->task_manager_end();

            ::Sleep( m_interval );
        }
    }

private:

    size_t m_interval;
    std::map<std::string, Application> m_application_map;
    ITaskMgrObserver* m_observer;
};


