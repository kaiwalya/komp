#if !defined kompute_hpp
#define kompute_hpp

#include "tutil.hpp"
#include "stack.hpp"
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
			koro::stack m_stack;
		public:
			processor(func_type & f):m_f(f)
			{
				
			}
		};
		
		struct message
		{
			virtual size_t size() const = 0;
			virtual void setSize() = 0;
			virtual const void * data() const = 0;
			virtual void * data() = 0;
		};
		
		struct message_queue_window
		{
			virtual void close() = 0;
			virtual size_t size() = 0;
			virtual void set_size(size_t) = 0;
			virtual void slide(size_t) = 0;
		};
		
		struct message_queue_read_window
		{
			virtual const message * getAt(size_t) = 0;
		};
		
		struct message_queue_write_window
		{
			virtual message * getAt(size_t) = 0;
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
			condition m_workerShutdownCondition;
		
		public:
			ctx_shared();
			~ctx_shared();
			processor * dispatch(std::function<void(ctx &)> & f);
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
			io m_io;
		public:
			ctx_role role() const;
			io & current();
			io dispatch(std::function<void(ctx &)> & f);
			io dispatch(std::function<void(ctx &)> && f);
		};
		
		void test();
	}
}

#endif