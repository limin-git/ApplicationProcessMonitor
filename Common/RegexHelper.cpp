///////////////////////////////////////////////////////////
//  RegexHelper.cpp
//  Implementation of the Class RegexHelper
//  Created on:     28/05/2008 19:09:48
//  Original author: limin
///////////////////////////////////////////////////////////


#include "StdAfx.h"
#include "RegexHelper.h"


bool RegexHelper::regex_remove_all( std::string& s, const boost::regex& e )
{
    FUNCTION_ENTRY( "regex_remove_all" );
    FUNCTION_EXIT;
    return regex_replace_all( s, e );
}


bool RegexHelper::regex_replace_all( std::string& s, const boost::regex& e, const std::string& r )
{
    FUNCTION_ENTRY( "regex_replace_all" );

    if ( true == regex_search( s, e ) )
    {
        std::ostringstream t( std::ios::out | std::ios::binary );
        std::ostream_iterator< char, char > oi( t );
        regex_replace( oi,
                       s.begin(),
                       s.end(),
                       e,
                       r,
                       boost::match_default | boost::format_all );
        s = t.str();

        FUNCTION_EXIT;
        return true;
    }
    else
    {
        FUNCTION_EXIT;
        return false;
    }
}


bool RegexHelper::regex_replace_first_only( std::string& s, const boost::regex& e, const std::string& r )
{
    FUNCTION_ENTRY( "regex_replace_first_only" );

    if ( true == regex_search( s, e ) )
    {
        std::ostringstream t( std::ios::out | std::ios::binary );
        std::ostream_iterator< char, char > oi( t );

        regex_replace( oi,
                       s.begin(),
                       s.end(),
                       e,
                       r,
                       boost::match_default | boost::format_first_only );
        s = t.str();

        FUNCTION_EXIT;
        return true;
    }
    else
    {
        FUNCTION_EXIT;
        return false;
    }
}



