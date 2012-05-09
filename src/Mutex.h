/*
 * Mutex.h
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

#ifndef MUTEX_H_
#define MUTEX_H_

#include <pthread.h>

class RecursiveMutex
{
public:
	RecursiveMutex();
	~RecursiveMutex();

	void Lock();
	void Unlock();

protected:
	pthread_mutex_t	m_mutex;
};

class LockMutex
{
public:
	LockMutex(RecursiveMutex& mutex);
	~LockMutex();

protected:
	RecursiveMutex&	m_mutex;
};




#endif /* MUTEX_H_ */
