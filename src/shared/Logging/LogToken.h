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

#ifndef LOGTOKEN_H
#define LOGTOKEN_H

class LogToken
{
    public:
        LogToken() : m_capacity(0), m_tokens(0), m_refillRate(0.0),
            m_lastRefill(std::chrono::steady_clock::now()) { }

        LogToken(size_t capacity, double refillRatePerSecond) : m_capacity(capacity), m_tokens(capacity),
            m_refillRate(refillRatePerSecond), m_lastRefill(std::chrono::steady_clock::now()) { }

        LogToken(LogToken&& other) noexcept : m_capacity(other.m_capacity), m_tokens(other.m_tokens),
            m_refillRate(other.m_refillRate), m_lastRefill(other.m_lastRefill)
        {
            other.m_capacity = 0;
            other.m_tokens = 0;
            other.m_refillRate = 0.0;
        }

        LogToken& operator=(LogToken&& other) noexcept
        {
            if (this != &other)
            {
                m_capacity = other.m_capacity;
                m_tokens = other.m_tokens;
                m_refillRate = other.m_refillRate;
                m_lastRefill = other.m_lastRefill;

                other.m_capacity = 0;
                other.m_tokens = 0;
                other.m_refillRate = 0.0;
            }

            return *this;
        }

        inline bool consume(double tokens)
        {
            refill();

            if (m_tokens >= tokens)
            {
                m_tokens -= tokens;
                return true;
            }

            return false;
        }

    private:
        size_t m_capacity;
        double m_tokens;
        double m_refillRate;

        std::chrono::steady_clock::time_point m_lastRefill;

        inline void refill()
        {
            auto now = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed = now - m_lastRefill;

            double refillAmount = elapsed.count() * m_refillRate;
            m_tokens = std::min<double>(m_capacity, m_tokens + refillAmount);

            m_lastRefill = now;
        }
};

#endif // LOGTOKEN_H
