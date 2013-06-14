#if !defined kompute_hpp
#define kompute_hpp

#include "tutil.hpp"
/*
 #include <stddef.h>
 #include <stdint.h>
 #include <string>
*/

namespace komp
{
	namespace mq
	{
		
		class io;
		class ctx_shared;
		class ctx;

		typedef std::function<void(ctx &)> func_type;
		
		class processor
		{
			func_type m_f;
		public:
			processor(func_type & f):m_f(f)
			{
				
			}
		};
		
		class io
		{
			ctx & m_ctx;
		public:
			io(ctx & c);
			io & operator = (const io & other);
			~io();
		};
		
		class ctx_shared: public thread::localizable<ctx_shared, ctx>
		{
			std::unique_ptr<thread::pool> m_pool;
			
			
			void bootPool();
			void shutdownPool();
			
			typedef std::mutex mutex;
			typedef std::condition_variable condition;
			typedef std::unique_lock<mutex> lock;
			 
			//////
			//Worker related stuff

			
			//Protects count and map
			mutex m_workerInfoLock;

			size_t m_nWorkers;
			size_t workerCount(){return m_nWorkers;}
			struct worker_params
			{
				size_t m_workerID;
			};
			void worker(worker_params *);
			std::map<size_t, worker_params *> m_workerinfo;


			//Protects booted worker count, and join for boot phase
			mutex m_workerBootMutex;
			size_t m_nBootedWorkers;
			condition m_workerBootingCondition;
			condition m_workerBootedCondition;
			
			//Protects shutdown worker count
			std::atomic_int m_workersShouldShutdown;
			mutex m_workerShutdownMutex;
			size_t m_nShutdownWorkers;
			condition m_workerShuttingdownCondition;
			condition m_workerShutdownCondition;
			
			
		public:
			ctx_shared();
			~ctx_shared();
			
			processor * dispatch(std::function<void(ctx &)> & f);
		};
		
		class ctx: public thread::localized<ctx_shared, ctx>
		{
		public:
			ctx(ctx_shared * shared);
			~ctx();
			
		private:
			io m_io;
		public:
			io & current();
			io dispatch(std::function<void(ctx &)> & f);
			io dispatch(std::function<void(ctx &)> && f);
		};
		
		void test();
	}
}

#endif