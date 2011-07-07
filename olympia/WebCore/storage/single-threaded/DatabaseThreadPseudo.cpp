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
#include "DatabaseThread.h"

#if ENABLE(DOM_STORAGE)
#if ENABLE(SINGLE_THREADED)

#include "Database.h"
#include "DatabaseTask.h"
#include "NotImplemented.h"
#include "SQLTransactionClient.h"
#include "SQLTransactionCoordinator.h"

namespace WebCore {

DatabaseThread::DatabaseThread()
    : m_timer(this, &DatabaseThread::timerFired)
    , m_transactionClient(new SQLTransactionClient())
    , m_transactionCoordinator(new SQLTransactionCoordinator())
#ifndef NDEBUG
    , m_isPerformingTask(false)
#endif
{
}

DatabaseThread::~DatabaseThread()
{
    clearQueue();
}

void DatabaseThread::clearQueue()
{
    for (Deque<DatabaseTask*>::const_iterator i = m_queue.begin(); i != m_queue.end(); ++i)
        delete *i;
    m_queue.clear();
}

bool DatabaseThread::start()
{
    return true;
}
void DatabaseThread::requestTermination(DatabaseTaskSynchronizer* cleanupSync)
{
    clearQueue();
    m_transactionCoordinator->shutdown();
}

bool DatabaseThread::terminationRequested() const
{
    return m_queue.isEmpty();
}

void DatabaseThread::timerFired(Timer<DatabaseThread>*)
{
    if (m_queue.isEmpty())
        return;

#ifndef NDEBUG
    m_isPerformingTask = true;
#endif

    OwnPtr<DatabaseTask> task(m_queue.first());
    m_queue.removeFirst();
    task->performTask();

#ifndef NDEBUG
    m_isPerformingTask = false;
#endif

    if (!m_queue.isEmpty())
        m_timer.startOneShot(0);
}

void DatabaseThread::scheduleTask(PassOwnPtr<DatabaseTask> task)
{
    m_queue.append(task.release());
    if (!m_timer.isActive())
        m_timer.startOneShot(0);
}

void DatabaseThread::scheduleImmediateTask(PassOwnPtr<DatabaseTask> taskIn, bool waitUntilFinish)
{
    OwnPtr<DatabaseTask> task(taskIn);
    if (waitUntilFinish) {
#ifndef NDEBUG
        // The caller must prevent this from occuring.
        ASSERT(!m_isPerformingTask);
        m_isPerformingTask = true;
#endif
        task->performTask();
#ifndef NDEBUG
        m_isPerformingTask = false;
#endif
        return;
    }

    // Make this task the first one in the queue
    m_queue.prepend(task.release());
    if (!m_timer.isActive())
        m_timer.startOneShot(0);
}

void DatabaseThread::unscheduleDatabaseTasks(Database* database)
{
    Deque<DatabaseTask*> reservedTasks;
    for (Deque<DatabaseTask*>::const_iterator i = m_queue.begin(); i != m_queue.end(); ++i) {
        if ((*i)->database() != database)
            reservedTasks.append(*i);
        else
            delete *i;
    }

    m_queue.swap(reservedTasks);
}

void DatabaseThread::recordDatabaseOpen(Database* database)
{
    notImplemented();
}

void DatabaseThread::recordDatabaseClosed(Database* database)
{
    notImplemented();
}

} // namespace WebCore

#endif // ENABLE(SINGLE_THREADED)
#endif // ENABLE(DOM_STORAGE)
