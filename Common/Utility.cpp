#include "StdAfx.h"
#include "Utility.h"
#include "Configuration.h"
#include "Console.h"
#include <psapi.h>
#include <tlhelp32.h>


std::string Utility::get_string_from_file( const std::string& file_path )
{
    FUNCTION_ENTRY( "get_string_from_file" );

    std::ifstream ifs( file_path.c_str() );

    if ( !ifs )
    {
        COUT_ERROR( SourceInfo, "cannot open file " << Utility::preferred_path( file_path ) << "." << std::endl );

        system( "pause" );

        FUNCTION_EXIT;
        exit( - 1 );
    }

    FUNCTION_EXIT;
    return std::string( std::istreambuf_iterator< char >( ifs ), ( std::istreambuf_iterator< char >() ) );
}


std::string Utility::preferred_path( const std::string& path )
{
    FUNCTION_ENTRY( "preferred_path" );

    std::string path_copy = path;
    boost::replace_all( path_copy, "/", "\\" );

    FUNCTION_EXIT;
    return path_copy;
}


std::vector<std::string> Utility::get_process_list()
{
    FUNCTION_ENTRY( "get_process_list" );

    //TODO:
    // current version only enumerate processes of current user, and can not enumerate system process

    std::vector<std::string> process_list;

    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;

    if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
    {
        FUNCTION_EXIT;
        return process_list;
    }

    // Calculate how many process identifiers were returned.

    cProcesses = cbNeeded / sizeof(DWORD);

    // Print the name and process identifier for each process.

    for ( i = 0; i < cProcesses; i++ )
    {
        char szProcessName[MAX_PATH] = "<unknown>";

        // Get a handle to the process.

        HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i] );

        // Get the process name.

        if ( NULL != hProcess )
        {
            HMODULE hMod;
            DWORD cbNeeded;

            if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), &cbNeeded) )
            {
                GetModuleBaseName( hProcess, hMod, szProcessName, sizeof(szProcessName)/sizeof(TCHAR) );

                boost::to_lower( szProcessName );

                process_list.push_back( szProcessName );
            }
        }

        CloseHandle( hProcess );
    }

    FUNCTION_EXIT;
    return process_list;
}


std::map<std::string, bool> Utility::get_service_list()
{
    FUNCTION_ENTRY( "get_service_list" );

    std::map<std::string, bool> service_list;

    // Open a handle to the SC Manager database.
    static SC_HANDLE schSCManager = OpenSCManager( NULL,                    // local machine
                                                   NULL,                    // ServicesActive database
                                                   SC_MANAGER_ALL_ACCESS);  // full access rights

    if ( NULL == schSCManager )
    {
        std::cout << "OpenSCManager failed (" << GetLastError() << ")" << std::endl;

        FUNCTION_EXIT;
        return service_list;
    }

    static LPENUM_SERVICE_STATUS lpServices = (LPENUM_SERVICE_STATUS)LocalAlloc( LPTR, 64 * 1024 );
    DWORD nSize = 0;
    DWORD n;
    DWORD nResumeHandle = 0;

    if ( false == EnumServicesStatus( schSCManager,
                                      SERVICE_WIN32,
                                      SERVICE_STATE_ALL,
                                      lpServices,
                                      64 * 1024,
                                      &nSize,
                                      &n,
                                      &nResumeHandle ) )
    {
        FUNCTION_EXIT;
        return service_list;
    }

    for ( DWORD i = 0; i < n; ++i )
    {
        std::string service_name = lpServices[i].lpServiceName;

        boost::to_lower( service_name );

        bool is_service_running = ( lpServices[i].ServiceStatus.dwCurrentState != SERVICE_STOPPED );

        service_list[service_name] = is_service_running;
    }

    FUNCTION_EXIT;
    return service_list;
}


std::vector<std::string> Utility::get_process_list_2()
{
    FUNCTION_ENTRY( "get_process_list_2" );

    std::vector<std::string> process_list;

    HANDLE hProcessSnap = NULL;
    PROCESSENTRY32 pe32 = { 0 };

    hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );

    if ( (HANDLE)-1 == hProcessSnap )
    {
        FUNCTION_EXIT;
        return process_list;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if ( Process32First( hProcessSnap, &pe32 ) )
    {
        do
        {
            std::string exec_file = pe32.szExeFile;
            boost::to_lower( exec_file );
            process_list.push_back( exec_file );
        }
        while ( Process32Next( hProcessSnap, &pe32 ) );
    }

    CloseHandle(hProcessSnap);

    FUNCTION_EXIT;
    return process_list;
}

