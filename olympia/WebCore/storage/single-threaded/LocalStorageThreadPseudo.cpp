/*
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 *  This library is distributed in the hope that i will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "LocalStorageThread.h"

#if ENABLE(DOM_STORAGE)
#if ENABLE(SINGLE_THREADED)

#include "LocalStorageTask.h"
#include "StorageAreaSync.h"

namespace WebCore {

PassOwnPtr<LocalStorageThread> LocalStorageThread::create()
{
    return new LocalStorageThread;
}

LocalStorageThread::LocalStorageThread()
: m_timer(this, &LocalStorageThread::timerFired)
{
}

LocalStorageThread::~LocalStorageThread()
{
	clearQueue();
}

void LocalStorageThread::clearQueue()
{
    for (Deque<LocalStorageTask*>::const_iterator i = m_queue.begin(); i != m_queue.end(); ++i)
        delete *i;
    m_queue.clear();
}

bool LocalStorageThread::start()
{
    return true;
}

void LocalStorageThread::timerFired(Timer<LocalStorageThread>*)
{
    if (m_queue.isEmpty())
        return;

    OwnPtr<LocalStorageTask> task(m_queue.first());
    m_queue.removeFirst();
    task->performTask();
    if (!m_queue.isEmpty())
        m_timer.startOneShot(0);
}

void LocalStorageThread::scheduleTask(PassOwnPtr<LocalStorageTask> task)
{
    m_queue.append(task.release());
    if (!m_timer.isActive())
        m_timer.startOneShot(0);
}

void LocalStorageThread::terminate()
{
    clearQueue();
    m_timer.stop();
}

void LocalStorageThread::performTerminate()
{
    ASSERT_NOT_REACHED();
    terminate();
}

}
#endif // ENABLE(SINGLE_THREADED)
#endif // ENABLE(DOM_STORAGE)
