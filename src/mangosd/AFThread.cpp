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

#include "AFThread.h"
#include "Log.h"

#include <iostream>
#include <thread>
#include <stdexcept>
#include <chrono>

AntiFreezeThread::AntiFreezeThread(uint32_t delay) : delaytime(delay) { }

AntiFreezeThread::~AntiFreezeThread()
{
    stop(); // Ensure the thread stops on object destruction
}

void AntiFreezeThread::start()
{
    if (isRunning)
        return; // Avoid starting the thread twice

    isRunning = true;
    antifreezeThread = std::thread(&AntiFreezeThread::run, this);
}

void AntiFreezeThread::stop()
{
    if (isRunning)
    {
        isRunning = false; // Signal the thread to stop
        if (antifreezeThread.joinable())
        {
            antifreezeThread.join(); // Wait for the thread to finish
        }
    }
}

void AntiFreezeThread::run()
{
    if (delaytime == 0)
        return;

    sLog.outString("AntiFreeze Thread started ( %u seconds max stuck time)", delaytime / 1000);

    while (isRunning)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        uint32_t curtime = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::system_clock::now().time_since_epoch())
                                .count());

        if (w_loops != w_loops.load())  // Detect changes in world loop counter
        {
            w_lastchange = curtime;
            w_loops = w_loops.load();
        }
        else if ((curtime - w_lastchange.load()) > delaytime)  // Possible freeze detected
        {
            sLog.outError("World Thread appears frozen! Initiating shutdown...");

            throw std::runtime_error("World thread hangs, shutting down server.");
        }
    }

    sLog.outString("AntiFreeze Thread stopped.");
}
