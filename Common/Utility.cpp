#include "StdAfx.h"
#include "Utility.h"
#include "Configuration.h"
#include "Console.h"
#include <psapi.h>
#include <tlhelp32.h>


std::string Utility::get_string_from_file( const std::string& file_path )
{
    std::ifstream ifs( file_path.c_str() );

    if ( !ifs )
    {
        COUT_ERROR( SourceInfo, "cannot open file " << Utility::preferred_path( file_path ) << "." << std::endl );
        exit( - 1 );
    }

    return std::string( std::istreambuf_iterator<char>( ifs ), ( std::istreambuf_iterator<char>() ) );
}


std::string Utility::preferred_path( const std::string& path )
{
    std::string path_copy = path;
    boost::replace_all( path_copy, "/", "\\" );
    return path_copy;
}


std::map<std::string, bool> Utility::get_service_list()
{
    std::map<std::string, bool> service_list;

    // Open a handle to the SC Manager database.
    static SC_HANDLE schSCManager = ::OpenSCManager( NULL,                    // local machine
                                                     NULL,                    // ServicesActive database
                                                     SC_MANAGER_ALL_ACCESS);  // full access rights

    if ( NULL == schSCManager )
    {
        std::cout << "OpenSCManager failed (" << GetLastError() << ")" << std::endl;
        return service_list;
    }

    static LPENUM_SERVICE_STATUS lpServices = (LPENUM_SERVICE_STATUS)LocalAlloc( LPTR, 64 * 1024 );
    DWORD nSize = 0;
    DWORD n;
    DWORD nResumeHandle = 0;

    if ( false == ::EnumServicesStatus( schSCManager,
                                        SERVICE_WIN32,
                                        SERVICE_STATE_ALL,
                                        lpServices,
                                        64 * 1024,
                                        &nSize,
                                        &n,
                                        &nResumeHandle ) )
    {
        return service_list;
    }

    for ( DWORD i = 0; i < n; ++i )
    {
        std::string service_name = lpServices[i].lpServiceName;
        boost::to_lower( service_name );
        bool is_service_running = ( lpServices[i].ServiceStatus.dwCurrentState != SERVICE_STOPPED );
        service_list[service_name] = is_service_running;
    }

    return service_list;
}


std::set<std::string> Utility::get_process_list()
{
    std::set<std::string> process_list;

    HANDLE hProcessSnap = NULL;
    PROCESSENTRY32 pe32 = { 0 };

    hProcessSnap = ::CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );

    if ( (HANDLE)-1 == hProcessSnap )
    {
        return process_list;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if ( ::Process32First( hProcessSnap, &pe32 ) )
    {
        do
        {
            process_list.insert( boost::to_lower_copy( std::string( pe32.szExeFile ) ) );
        }
        while ( ::Process32Next( hProcessSnap, &pe32 ) );
    }

    ::CloseHandle(hProcessSnap);
    return process_list;
}


std::string Utility::get_current_time()
{
    char buff[100];
    timeb timebuffer;
    tm t;
    ftime( &timebuffer );
    localtime_s( &t, &timebuffer.time );
    sprintf_s( buff, "%02d/%02d/%02d %02d:%02d:%02d:%03d", t.tm_mday, t.tm_mon + 1, t.tm_year + 1900, t.tm_hour, t.tm_min, t.tm_sec, timebuffer.millitm );
    return buff;
}


void Utility::log_to_file( const std::string& s )
{
    static std::ofstream os( "ApplicationProcessMonitor.log" );
    os << get_current_time() << "\t" << s << std::endl;
}
