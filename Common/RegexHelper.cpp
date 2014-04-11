#include "StdAfx.h"
#include "RegexHelper.h"


bool RegexHelper::regex_remove_all( std::string& s, const boost::regex& e )
{
    return regex_replace_all( s, e );
}


bool RegexHelper::regex_replace_all( std::string& s, const boost::regex& e, const std::string& r )
{
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
        return true;
    }

    return false;
}


bool RegexHelper::regex_replace_first_only( std::string& s, const boost::regex& e, const std::string& r )
{
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
        return true;
    }

    return false;
}
