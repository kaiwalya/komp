#if !defined _komp_included_
#define _komp_included_

#include "kompute.hpp"
#include "koroutine.hpp"
#include <thread>
#include <map>

namespace komp
{
	namespace mq
	{
		
		class ctx;
		class ctx_thread;
		
		
		class ctx
		{
		private:
			typedef std::map<std::thread::id, ctx_thread *> ThreadContextMap;
			std::mutex m_mThreadContext;
			ThreadContextMap m_threadContexts;
		public:
			ctx();
			virtual ~ctx();
			
			ctx_thread * getThreadContext(std::thread::id threadID);
			void attachThreadContext(std::thread::id threadID, ctx_thread * ctxThread);
			void detachThreadContext(std::thread::id threadID);
			
		private:
		public:
		};
		
		class ctx_thread: public ktx
		{
		private:
			ctx * m_ctx;
			std::thread::id m_threadID;
		public:
			static std::thread::id getCurrentThreadID();
		public:
			ctx_thread();
			ctx_thread(ctx * ctx);
			virtual ~ctx_thread();
			ktx * attachThread() override;
			
		private:
			koro::ktx * m_koro;
		public:
			qp_outer * createQProcessor(qp_logic *);
		};
		
	}
}
#endif