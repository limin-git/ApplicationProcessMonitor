#pragma once
////////////////////////[ Macros ]//////////////////////////////////////////////////
typedef std::multimap< std::string, std::string > KeyValueMap;
typedef std::string Section;
typedef std::map< Section, KeyValueMap > SectionMap;

#define CONFIG_INI ".\\ApplicationProcessMonitor.ini"

#define CONFIG_SECTION_OPTION                                         "Option"
#define CONFIG_KEY_INTERVAL_IN_SECONDS                                "IntervalInSeconds"

#define CONFIG_KEY_AFTER_START                                        "After Start"
#define CONFIG_KEY_AFTER_STOP                                         "After Stop"

#define SourceInfo __FILE__ << ":" << __LINE__ << " "
#define COUT_ERROR( source_info, msg )      std::cout << source_info << JadedHoboConsole::fg_red << "error:" << JadedHoboConsole::default_color << " " << msg
#define COUT_WARNING( source_info, msg )    std::cout << source_info << JadedHoboConsole::fg_yellow << "warning:" << JadedHoboConsole::default_color << " " << msg
#define COUT_DEBUG( source_info, msg )      std::cout << std::endl; std::cout << source_info << "DEBUG: " << msg; first_cout_char = 0
#define COUT_INFO( source_info, msg )       std::cout << msg
#define COUT_RED( source_info, msg )        std::cout << JadedHoboConsole::fg_red << msg << JadedHoboConsole::default_color
#define COUT_BLUE( source_info, msg )       std::cout << JadedHoboConsole::fg_blue << msg << JadedHoboConsole::default_color
#define COUT_GREEN( source_info, msg )      std::cout << JadedHoboConsole::fg_green << msg << JadedHoboConsole::default_color
