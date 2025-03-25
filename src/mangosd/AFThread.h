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

#ifndef ANTIFREEZE_THREAD_H
#define ANTIFREEZE_THREAD_H

#include <thread>
#include <atomic>
#include <cstdint>
#include <chrono>

class AntiFreezeThread
{
public:
    explicit AntiFreezeThread(uint32_t delay);
    ~AntiFreezeThread();                      // Destructor to handle cleanup

    void start();                             // Start the anti-freeze thread
    void stop();                              // Graceful stop mechanism

private:
    void run();                               // Thread execution logic

    std::thread antifreezeThread;            // Standard thread for anti-freeze logic
    std::atomic<bool> isRunning { false };    // Flag to control thread running state

    std::atomic<uint32_t> m_loops { 0 };      // Thread-safe loop counters
    std::atomic<uint32_t> m_lastchange { 0 };
    std::atomic<uint32_t> w_loops { 0 };
    std::atomic<uint32_t> w_lastchange { 0 };

    uint32_t delaytime;                      // Delay time in milliseconds
};

#endif // ANTIFREEZE_THREAD_H
