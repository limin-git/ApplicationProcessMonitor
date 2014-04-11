#include "StdAfx.h"
#include "Configuration.h"
#include "Utility.h"
#include "RegexHelper.h"


Configuration& Configuration::instance()
{
    FUNCTION_ENTRY( "instance" );

    static Configuration configuration;

    FUNCTION_EXIT;
    return configuration;
}


Configuration::Configuration( const std::string& ini_file_name /* = CONFIG_INI */ )
    : m_ini_file_name( ini_file_name )
{
    FUNCTION_ENTRY( "Configuration" );

    parse_ini_file( ini_file_name, m_section_map );

    FUNCTION_EXIT;
}


void Configuration::parse_ini_file( const std::string& ini_file_name, SectionMap& section_map )
{
    FUNCTION_ENTRY( "parse_ini_file" );

    std::string s = Utility::get_string_from_file( ini_file_name );

    if ( s.empty() )
    {
        COUT_ERROR( SourceInfo, "the file is empty: " << ini_file_name << std::endl );

        system( "pause" );

        FUNCTION_EXIT;
        exit( - 1 );
    }

    const char* comment_regex_str =
        "(?x)"
        "("
        "  ( //[^\\n]* $ )       | "   // cpp comment
        "  ( /\\* .*? \\*/ )     | "   // c comment
        "  ( \\# [^\\n]* $ ) "         // shell comment
        ")"
        "";

    RegexHelper::regex_remove_all( s, boost::regex( comment_regex_str ) );                                  // remove any comment
    RegexHelper::regex_remove_all( s, boost::regex( "^[ \\t]*\\n" ) );                                      // remove empty lines
    RegexHelper::regex_replace_all( s, boost::regex( "(?x) ^[ \\t]* ([^\\n]*?) [ \\t]*(?=\\n)" ), "$1" );   // remove leading/tailing spaces

    const char* section_regex_str =
        "(?x)"
        "^\\["                                   // starts with [
        "    ( [^\\[]+ ) "                       // $1, section
        "\\]$"                                   // ends with ]
        "(.*?) "                                 // $2, key-value list
        "(?= ( ^ [ \\t]* \\[ ) | \\z ) "         // lookahead, not starts with [
        "";

    static const boost::regex section_regex( section_regex_str );
    boost::sregex_iterator it( s.begin(), s.end(), section_regex );
    boost::sregex_iterator end;

    for ( ; it != end; ++it )
    {
        const std::string& section = it->str(1);
        const std::string& key_value_list_string = it->str(2);

        {
            const char* key_value_pair_regex_str =
                "(?x)"
                "^\\s* ( [^=\\n]+ ) "                //$1, key
                " \\s* = \\s* ( [^\\n]+? ) \\s* $"   //$2, value
                "";

            static const boost::regex key_value_regex( key_value_pair_regex_str );
            boost::sregex_iterator key_value_it( key_value_list_string.begin(), key_value_list_string.end(), key_value_regex );
            boost::sregex_iterator end;

            for ( ; key_value_it != end; ++key_value_it )
            {
                const std::string& key = key_value_it->str(1);
                const std::string& value = key_value_it->str(2);
                section_map[ section ].insert( std::make_pair( key, value ) );
            }
        }
    }

    FUNCTION_EXIT;
}


const std::string& Configuration::get_configuration( const std::string& section, const std::string& key ) const
{
    FUNCTION_ENTRY( "get_configuration" );

    const KeyValueMap& key_value_map = get_configuration( section );

    if ( false == key_value_map.empty() )
    {
        KeyValueMap::const_iterator find_it = key_value_map.find( key );

        if ( find_it != key_value_map.end() )
        {
            FUNCTION_EXIT;
            return find_it->second;
        }
    }

    static const std::string empty_string;

    FUNCTION_EXIT;
    return empty_string;
}


const KeyValueMap& Configuration::get_configuration( const std::string& section ) const
{
    FUNCTION_ENTRY( "get_configuration" );

    SectionMap ::const_iterator find_it = m_section_map.find( section );

    if ( find_it != m_section_map.end() )
    {
        FUNCTION_EXIT;
        return find_it->second;
    }

    static const KeyValueMap empty_key_value_map;

    FUNCTION_EXIT;
    return empty_key_value_map;
}


const SectionMap& Configuration::get_configuration() const
{
    FUNCTION_ENTRY( "get_configuration" );
    FUNCTION_EXIT;
    return m_section_map;
}

