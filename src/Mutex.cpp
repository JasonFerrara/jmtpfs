/*
 * Mutex.cpp
 *
 *      Author: Jason Ferrara
 *
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 3 as published by the Free Software Foundation.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02111-1301, USA.
 * licensing@fsf.org
 */

#include "Mutex.h"

#include <sstream>
#include <stdexcept>

void checkPthreadError(int err)
{
	if (err)
	{
		std::ostringstream msg;
		msg << "pthread error " << err;
		throw std::runtime_error(msg.str());
	}
}

RecursiveMutex::RecursiveMutex()
{
	pthread_mutexattr_t mattr;
	checkPthreadError(pthread_mutexattr_init(&mattr));
	checkPthreadError(pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE));
	checkPthreadError(pthread_mutex_init(&m_mutex, &mattr));
	checkPthreadError(pthread_mutexattr_destroy(&mattr));
}

RecursiveMutex::~RecursiveMutex()
{
	pthread_mutex_destroy(&m_mutex);
}

void RecursiveMutex::Lock()
{
	checkPthreadError(pthread_mutex_lock(&m_mutex));
}

void RecursiveMutex::Unlock()
{
	checkPthreadError(pthread_mutex_unlock(&m_mutex));
}

LockMutex::LockMutex(RecursiveMutex& mutex) : m_mutex(mutex)
{
	m_mutex.Lock();
}

LockMutex::~LockMutex()
{
	m_mutex.Unlock();
}


