#if !defined kompute_hpp
#define kompute_hpp

#include "tutil.hpp"
#include "stack.hpp"
#include <string>
#include <memory>
#include <vector>

namespace komp
{
	
	namespace mq
	{
		
		class io;
		class ctx_shared;
		class ctx;

		typedef std::function<void(ctx &)> process;
		
		class ctx_shared: public thread::localizable<ctx_shared, ctx>
		{
		protected:
			size_t m_nExternalCtx;
			virtual void tlocalcontext_attach_locked(ctx * local);
			virtual void tlocalcontext_detach_locked(ctx * local);
		private:
			std::unique_ptr<thread::pool> m_pool;
			
			
			void bootPool();
			void triggerPoolShutdown();
			void waitForPoolShutdown();
			
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
				std::mutex m_workerMutex;
				std::condition_variable m_workerCondition;
				size_t m_workerID;
				bool m_quit;
			};
			void worker(worker_params *);
			std::map<size_t, worker_params *> m_workerinfo;


			//Protects booted worker count, and join for boot phase
			mutex m_workerBootMutex;
			size_t m_nBootedWorkers;
			condition m_workerBootingCondition;
			condition m_workerBootedCondition;
			
			//Protects shutdown worker count
			mutex m_workerShutdownMutex;
			size_t m_nShutdownWorkers;
			condition m_workerShuttingdownCondition;
			//condition m_workerShutdownCondition;
		
		public:
			ctx_shared();
			~ctx_shared();
		};
		
		enum class ctx_role
		{
			external,
			internal,
		};
		
		class ctx: public thread::localized<ctx_shared, ctx>
		{
		public:
			ctx(ctx_shared * shared, ctx_role r = ctx_role::external);
			~ctx();
		private:
			ctx_role m_role;
		public:
			ctx_role role() const;
			//void registerProcessor(std::string && name, process && proc);
		};
		
		void test();
	}
}

#endif