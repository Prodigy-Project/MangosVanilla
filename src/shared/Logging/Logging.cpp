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

#include "Logging.h"

Logging::Logging(uint16 enabledLevels) : m_activeLevels(enabledLevels)
{
    Initialize();
    InitRateLimiters();
}

void Logging::Initialize()
{
    // set the default logging levels
    SetActiveLevels(LOG_LEVEL_DEBUG | LOG_LEVEL_ERROR | LOG_LEVEL_INFO | LOG_LEVEL_NOTICE | LOG_LEVEL_SQL | LOG_LEVEL_WARN);

    m_logFolder = sConfig.GetStringDefault("LogsDir", "");
    //m_logFile = sConfig.GetStringDefault("LogFile", "");

    log_enabled = sConfig.GetStringDefault("Log.Enabled", "");
    log_disabled = sConfig.GetStringDefault("Log.Disabled", "");

    // todo: move log file naming to configuration
    SetLogFile(m_logFolder + "world_server.log", LogOpenMode::Overwrite);

    if (!m_logFolder.empty() && m_logFolder.back() != '/' && m_logFolder.back() != '\\')
    {
        m_logFolder += '/';
    }

    std::unordered_map<std::string, LoggingLevel> level_map =
    {
        { "EMERG",  LOG_LEVEL_EMERG },
        { "ALERT",  LOG_LEVEL_ALERT },
        { "CRIT",   LOG_LEVEL_CRIT },
        { "ERROR",  LOG_LEVEL_ERROR },
        { "WARN",   LOG_LEVEL_WARN },
        { "NOTICE", LOG_LEVEL_NOTICE },
        { "INFO",   LOG_LEVEL_INFO },
        { "DEBUG",  LOG_LEVEL_DEBUG },
        { "TRACE",  LOG_LEVEL_TRACE },
        { "THREAD", LOG_LEVEL_THREAD },
        { "FUNC",   LOG_LEVEL_FUNC },
        { "SQL",    LOG_LEVEL_SQL }
    };

    std::stringstream enabled(log_enabled);
    while (std::getline(enabled, token, ','))
    {
        std::transform(token.begin(), token.end(), token.begin(), ::toupper);
        if (level_map.find(token) != level_map.end())
        {
            EnableLevel(level_map[token]);
        }
    }

    std::stringstream disabled(log_disabled);
    while (std::getline(disabled, token, ','))
    {
        std::transform(token.begin(), token.end(), token.begin(), ::toupper);
        if (level_map.find(token) != level_map.end())
        {
            DisableLevel(level_map[token]);
        }
    }

    for (const auto& [name, level] : level_map)
    {
        if (m_rateLimiters.find(level) == m_rateLimiters.end())
        {
            m_rateLimiters[level] = LogToken(250, 10); // capacity, refillrate
        }
    }
}

void Logging::InitRateLimiters()
{
    // todo: move token capacity and refill to configuration
    m_rateLimiters.emplace(LOG_LEVEL_EMERG,  LogToken(50, 1));
    m_rateLimiters.emplace(LOG_LEVEL_ALERT,  LogToken(75, 2));
    m_rateLimiters.emplace(LOG_LEVEL_ERROR,  LogToken(100, 5));
    m_rateLimiters.emplace(LOG_LEVEL_WARN,   LogToken(150, 8));
    m_rateLimiters.emplace(LOG_LEVEL_INFO,   LogToken(200, 10));
    m_rateLimiters.emplace(LOG_LEVEL_DEBUG,  LogToken(250, 15));
    m_rateLimiters.emplace(LOG_LEVEL_TRACE,  LogToken(300, 20));
    m_rateLimiters.emplace(LOG_LEVEL_FUNC,   LogToken(300, 20));
    m_rateLimiters.emplace(LOG_LEVEL_THREAD, LogToken(300, 20));
    m_rateLimiters.emplace(LOG_LEVEL_SQL,    LogToken(250, 20));
}

void Logging::EnableLevel(LoggingLevel level)
{
    m_activeLevels |= level;
}

void Logging::DisableLevel(LoggingLevel level)
{
    m_activeLevels &= ~level;
}

void Logging::SetActiveLevels(uint16 levels)
{
    m_activeLevels = levels;
}

uint16 Logging::GetActiveLevels() const
{
    return m_activeLevels;
}

void Logging::SetLogFile(const std::string& filename, LogOpenMode mode)
{
    std::lock_guard<std::mutex> lock(m_log);

    if (m_logFile.is_open())
    {
        m_logFile.close();
    }

    std::ios_base::openmode open_mode = std::ios::out;
    switch (mode)
    {
        case LogOpenMode::Append:
            open_mode |= std::ios::app;
            break;

        case LogOpenMode::Overwrite:
            open_mode |= std::ios::trunc;
            break;

        default:
            fprintf(stderr, "Logging: Unknown file open mode. Defaulting to append.\n");
            open_mode |= std::ios::app;
            break;
    }

    m_logFile.open(filename, open_mode);

    if (!m_logFile)
    {
        fprintf(stderr, "Logging: Failed to open log file: %s\n", filename.c_str());
    }
}

void Logging::CloseLogFile()
{
    std::lock_guard<std::mutex> lock(m_log);

    if (m_logFile.is_open())
    {
        m_logFile.close();
    }
}

void Logging::LogOutput(LoggingLevel level, std::string_view str, va_list args)
{
    // thread-safe guard
    std::lock_guard<std::mutex> lock(m_log);

    std::string levelName = GetLevelName(level);
    uint16 color = GetLevelColor(level);
    std::string timestamp = GetTimestamp();

    // buffer to hold the formatted message
    char messageBuffer[1024];

    // use vsnprintf to handle the variadic arguments
    vsnprintf(messageBuffer, sizeof(messageBuffer), str.data(), args);

    // print the formatted message
    printf("[%s]\x1b[%dm [%s] %s\x1b[0m\n", timestamp.c_str(), color, levelName.c_str(), messageBuffer);

    if (m_logFile.is_open())
    {
        m_logFile << "[" << timestamp << "] [" << levelName << "] " << messageBuffer << std::endl;
    }
}

inline void Logging::LogHelper(Logging* logger, LoggingLevel level, std::string_view str, ...)
{
    if (!logger->IsLevelEnabled(level))
        return;

    if (!logger->m_rateLimiters[level].consume(1.0))
        return;

    va_list args;
    va_start(args, str);
    logger->LogOutput(level, str, args);
    va_end(args);
}

void Logging::outEmergency(std::string_view str, ...)
{
    LogHelper(this, LOG_LEVEL_EMERG, str);
}

void Logging::outAlert(std::string_view str, ...)
{
    LogHelper(this, LOG_LEVEL_ALERT, str);
}

void Logging::outCritical(std::string_view str, ...)
{
    LogHelper(this, LOG_LEVEL_CRIT, str);
}

void Logging::outError(std::string_view str, ...)
{
    LogHelper(this, LOG_LEVEL_ERROR, str);
}

void Logging::outWarning(std::string_view str, ...)
{
    LogHelper(this, LOG_LEVEL_WARN, str);
}

void Logging::outNotice(std::string_view str, ...)
{
    LogHelper(this, LOG_LEVEL_NOTICE, str);
}

void Logging::outInfo(std::string_view str, ...)
{
    LogHelper(this, LOG_LEVEL_INFO, str);
}

void Logging::outDebug(std::string_view str, ...)
{
    LogHelper(this, LOG_LEVEL_DEBUG, str);
}

void Logging::outTrace(std::string_view str, ...)
{
    LogHelper(this, LOG_LEVEL_TRACE, str);
}

void Logging::outThread(std::string_view str, ...)
{
    LogHelper(this, LOG_LEVEL_THREAD, str);
}

void Logging::outFunction(std::string_view str, ...)
{
    LogHelper(this, LOG_LEVEL_FUNC, str);
}

void Logging::outSQL(std::string_view str, ...)
{
    LogHelper(this, LOG_LEVEL_SQL, str);
}
