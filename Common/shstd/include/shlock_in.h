#ifndef __SH_STD_SAFE_LOCK__IN_
#define __SH_STD_SAFE_LOCK__IN_
#include <pthread.h>

namespace shstd
{
	namespace lock
	{

		class CSafeGuard
		{
		private:
			CSafeGuard();
			pthread_mutex_t & m_mutex;
			bool m_bLock;
		public:
			CSafeGuard(pthread_mutex_t &mutex)
				:m_mutex(mutex),m_bLock(false)
			{
				//互斥锁方案
				if (0 == pthread_mutex_lock(&m_mutex)) m_bLock = true;
				//自旋锁方案
			}

			~CSafeGuard()
			{
				if (m_bLock) pthread_mutex_unlock(&m_mutex);
			}

			bool IsLocked() {return m_bLock;}
		};


		class CMutex;
		class CCond
		{
		private:
			pthread_cond_t m_tCond;
		public:
			CCond();
			virtual ~CCond();
			bool Wait(CMutex & mutex, int nMillisecond);
			bool Notify();
		};

		class CMutex
		{
		protected:
			pthread_mutex_t m_tMutex;
			friend class CCond;
		public:
			CMutex();
			virtual ~CMutex();

			bool Lock();
			bool TryLock();
			bool Unlock();
		};

		class CSafeMutex
		{
			CMutex &m_mutex;
			CSafeMutex();
			bool m_bLock;
		public:
			CSafeMutex(CMutex & mutex, bool bTryLock = false)
				:m_mutex(mutex)
			{
				if (bTryLock)
				{
					m_bLock = m_mutex.TryLock();
				}
				else
				{
					m_bLock = m_mutex.Lock();
				}        
			}
			bool IsLocked()
			{
				return m_bLock;
			}
			~CSafeMutex()
			{
				if (m_bLock)
				{
					m_mutex.Unlock();
				}        
			}
		};

		//读写锁
		class CRWLock
		{
			pthread_rwlock_t m_rwlock;
		public:
			CRWLock();
			virtual ~CRWLock();


			bool ReadLock();
			bool WriteLock();
			bool TryReadLock();
			bool TryWriteLock();
			bool Unlock();
		};

		class CSafeRWLock
		{
			CRWLock &m_rwLock;
			CSafeRWLock();
			bool m_bLock;
		public:
			CSafeRWLock(CRWLock & rwLock, bool bReadLock = true)
				:m_rwLock(rwLock)
			{
				if (bReadLock)
				{
					m_bLock = m_rwLock.ReadLock();
				}
				else
				{
					m_bLock = m_rwLock.WriteLock();
				}                
			}

			bool IsLocked()
			{
				return m_bLock;
			}

			~CSafeRWLock()
			{
				if (m_bLock)
				{
					m_rwLock.Unlock();
				}
			}
		};

		class CThreadCounter
		{
			unsigned int m_nIndex;
			static int m_nThreadCount[100];
			static CMutex m_mutex;
		public:
			CThreadCounter();
			CThreadCounter(unsigned int nIndex);
			~CThreadCounter();
			static int GetThreadCount(unsigned int nIndex = 0);
		};
	} 
}

#endif







