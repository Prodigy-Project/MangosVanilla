/**
 * MaNGOS is a full featured server for World of Warcraft, supporting
 * the following clients: 1.12.x, 2.4.3, 3.3.5a, 4.3.4a and 5.4.8
 *
 * Copyright (C) 2005-2025 MaNGOS <https://www.getmangos.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * World of Warcraft, and all World of Warcraft or Warcraft art, images,
 * and lore are copyrighted by Blizzard Entertainment, Inc.
 */

#ifndef LOGGING_H
#define LOGGING_H

#include "Config.h"
#include "LogDefines.h"
#include "LogToken.h"

enum LogLevelColor
{
    BLACK         = 30,
    RED           = 31,
    GREEN         = 32,
    BROWN         = 33,
    BLUE          = 34,
    MAGENTA       = 35,
    CYAN          = 36,
    WHITE         = 37,

    YELLOW        = 93,
    RED_BOLD      = 91,
    GREEN_BOLD    = 92,
    BLUE_BOLD     = 94,
    MAGENTA_BOLD  = 95,
    CYAN_BOLD     = 96,
    WHITE_BOLD    = 97,

    LIGHTBLUE     = 94,
    GREY          = 90
};

enum LoggingLevel
{
    // main loggers
    LOG_LEVEL_ALERT     = 1 << 0,  // Immediate action required
    LOG_LEVEL_CRIT      = 1 << 1,  // Critical conditions
    LOG_LEVEL_DEBUG     = 1 << 2,  // Debug-level messages
    LOG_LEVEL_EMERG     = 1 << 3,  // System is unusable
    LOG_LEVEL_ERROR     = 1 << 4,  // Error conditions
    LOG_LEVEL_FUNC      = 1 << 5,  // Function-level (fine-grained tracing)
    LOG_LEVEL_INFO      = 1 << 6,  // Informational messages
    LOG_LEVEL_NOTICE    = 1 << 7,  // Normal but significant condition
    LOG_LEVEL_SQL       = 1 << 8,  // SQL informational messages
    LOG_LEVEL_THREAD    = 1 << 9,  // Thread-level (thread execution tracing)
    LOG_LEVEL_TRACE     = 1 << 10, // Trace-level (fine-grained debugging)
    LOG_LEVEL_WARN      = 1 << 11, // Warning conditions

    LOG_LEVEL_ALL       = (1 << 12) - 1
};

enum LogOpenMode
{
    Append,
    Overwrite
};

class Logging
{
    public:
        static Logging* instance()
        {
            static Logging instance(LOG_LEVEL_ALL);
            return &instance;
        }

        Logging(uint16 enabledLevels);

        void Initialize();
        void InitRateLimiters();
        void LogOutput(LoggingLevel level, std::string_view str, va_list args);
        inline void LogHelper(Logging* logger, LoggingLevel level, std::string_view str, ...);

        uint16 GetActiveLevels() const;
        void EnableLevel(LoggingLevel level);
        void DisableLevel(LoggingLevel level);
        void SetActiveLevels(uint16 levels);

        void outEmergency(std::string_view str, ...);
        void outAlert(std::string_view str, ...);
        void outCritical(std::string_view str, ...);
        void outError(std::string_view str, ...);
        void outWarning(std::string_view str, ...);
        void outNotice(std::string_view str, ...);
        void outInfo(std::string_view str, ...);
        void outDebug(std::string_view str, ...);
        void outTrace(std::string_view str, ...);
        void outThread(std::string_view str, ...);
        void outFunction(std::string_view str, ...);
        void outSQL(std::string_view str, ...);

        void SetLogFile(const std::string& filename, LogOpenMode mode);
        void CloseLogFile();

        inline bool IsLogFileOpen() const
        {
            return m_logFile.is_open();
        }

    private:
        std::mutex m_log;
        std::ofstream m_logFile;
        std::string m_logFolder;

        std::string token;
        std::string log_enabled;
        std::string log_disabled;

        uint16 m_activeLevels;
        std::map<LoggingLevel, LogToken> m_rateLimiters;

        inline bool IsLevelEnabled(LoggingLevel level)
        {
            return (m_activeLevels & level) != 0;
        }

        inline std::string GetTimestamp()
        {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            std::tm local { };
        #ifdef _WIN32
            localtime_s(&local, &time);
        #else
            local = *std::localtime(&time);
        #endif
            char buffer[20];
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &local);
            return buffer;
        }

        inline std::string GetLevelName(LoggingLevel level)
        {
            switch (level)
            {
                case LOG_LEVEL_EMERG:
                    return "EMERG";

                case LOG_LEVEL_ALERT:
                    return "ALERT";

                case LOG_LEVEL_CRIT:
                    return "CRIT";

                case LOG_LEVEL_ERROR:
                    return "ERROR";

                case LOG_LEVEL_WARN:
                    return "WARN";

                case LOG_LEVEL_NOTICE:
                    return "NOTICE";

                case LOG_LEVEL_INFO:
                    return "INFO";

                case LOG_LEVEL_DEBUG:
                    return "DEBUG";

                case LOG_LEVEL_TRACE:
                    return "TRACE";

                case LOG_LEVEL_THREAD:
                    return "THREAD";

                case LOG_LEVEL_FUNC:
                    return "FUNC";

                case LOG_LEVEL_SQL:
                    return "SQL";

                default:
                    return "UNKNOWN";
            }
        }

        inline uint16 GetLevelColor(LoggingLevel level)
        {
            switch (level)
            {
                case LOG_LEVEL_EMERG:
                case LOG_LEVEL_ALERT:
                case LOG_LEVEL_CRIT:
                    return RED_BOLD;

                case LOG_LEVEL_ERROR:
                    return RED;

                case LOG_LEVEL_WARN:
                    return YELLOW;

                case LOG_LEVEL_NOTICE:
                    return GREEN;

                case LOG_LEVEL_INFO:
                    return LIGHTBLUE;

                case LOG_LEVEL_DEBUG:
                    return GREY;

                case LOG_LEVEL_TRACE:
                    return CYAN;

                case LOG_LEVEL_THREAD:
                    return MAGENTA;

                case LOG_LEVEL_FUNC:
                    return BROWN;

                case LOG_LEVEL_SQL:
                    return WHITE;

                default:
                    return WHITE;
            }
        }
};

#define sLogging Logging::instance()

#endif // LOGGING_H
