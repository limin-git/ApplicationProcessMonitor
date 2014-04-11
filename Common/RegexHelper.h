#pragma once
#include "type_fwd.h"


class RegexHelper
{
public:

    static bool regex_remove_all( std::string& s, const boost::regex& e );
    static bool regex_replace_all( std::string& s, const boost::regex& e, const std::string& r = "" );
    static bool regex_replace_first_only( std::string& s, const boost::regex& e, const std::string& r = "" );
    static bool regex_remove_comments( std::string& s );

private:

    RegexHelper();
};
