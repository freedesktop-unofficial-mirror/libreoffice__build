/*************************************************************************
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Kohei Yoshida.
 *    1039 Kingsway Dr., Apex, NC 27502, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "tool/timer.hxx"
#ifndef _WIN32
#include <sys/time.h>
#else
#include <windows.h>
#endif

namespace scsolver {

class TimerImpl
{
public:
	TimerImpl( double duration ) :
		m_fDuration(duration)
	{
	}

	~TimerImpl() throw()
	{
	}

	void init()
	{
		m_fCurTime = getTime();
	}

	bool isTimedOut()
	{
		return (getTime() - m_fCurTime) > m_fDuration;
	}

private:

	/**
     * Get current time in seconds.
	 * 
     * @return double current time in seconds
	 */
	double getTime()
	{
#ifndef _WIN32
		timeval tv;
		gettimeofday(&tv, NULL);
		return tv.tv_sec + tv.tv_usec / 1000000.0;
#else
		FILETIME ft;
		__int64 *time64 = (__int64 *) &ft;

		GetSystemTimeAsFileTime (&ft);

		/* Convert from 100s of nanoseconds since 1601-01-01
		 * to seconds since the Unix epoch.
		 */
		*time64 -= 116444736000000000i64;
		return *time64 / 10000000.0;
#endif
	}

	double m_fDuration;
	double m_fCurTime;
};

//-----------------------------------------------------------------

/*
Timer::Timer()
{
	// disabled
}
*/

Timer::Timer( double duration ) :
	m_pImpl( new TimerImpl(duration) )
{
}

Timer::~Timer() throw()
{
}

void Timer::init()
{
	m_pImpl->init();
}

bool Timer::isTimedOut() const
{
	return m_pImpl->isTimedOut();
}

}
