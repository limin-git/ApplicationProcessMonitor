///////////////////////////////////////////////////////////
//  RegexHelper.h
//  Implementation of the Class RegexHelper
//  Created on:     28/05/2008 19:09:48
//  Original author: limin
///////////////////////////////////////////////////////////

#if !defined(REGEX_HELPER_H)
#define REGEX_HELPER_H

#include "type_fwd.h"


/**
 * @author limin
 * @version 1.0
 * @created28/05/2008 19:09:48
 */
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

#endif // !defined(REGEX_HELPER_H)
