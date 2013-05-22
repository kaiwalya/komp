#include <memory>
#include <functional>
#include <stdint.h>
#include <tuple>
#include <vector>
#include <string>
#include <memory>
#include <thread>
#include <setjmp.h>
#include <assert.h>
#include <stdio.h>
#include "setjmp.h"

namespace koroutine
{
	class stack
	{
		void * m_pStackAllocStart;
		void * m_pStackTop;
	public:
		stack(size_t szWanted = 1024 * 8, const size_t roundOff = 16)
		{
			//Roundoff szWanted, and then add more padding so we can shift the pointer if we want
			auto allocSize = ((szWanted + roundOff - 1) & ~(roundOff - 1)) + (roundOff - 1);
			auto alloc = (uintptr_t)new unsigned char[allocSize];
			memset((void *)alloc, 'x', allocSize);
			auto stackTop = alloc + allocSize;
			stackTop -= (stackTop & (roundOff - 1));
			m_pStackTop = (void*) stackTop;
			m_pStackAllocStart = (void *)alloc;
		}
		
		stack(stack && s)
		{
			m_pStackAllocStart = s.m_pStackAllocStart;
			m_pStackTop = s.m_pStackTop;
			s.m_pStackTop = nullptr;
			s.m_pStackAllocStart = nullptr;
		}
		
		~stack()
		{
			if (m_pStackAllocStart)
			{
				size_t unusedBytes = 0;
				auto start = (unsigned char *)m_pStackAllocStart;
				while (start < (unsigned char*)m_pStackTop && *start == 'x')
				{
					unusedBytes++;
					start++;
				}
				assert(unusedBytes > (bytesUsed() + bytesFree())/2);
				printf("Stack(%p)::~Stack() Unused Bytes %u/%u\n", this, (unsigned int)unusedBytes, (unsigned int)(bytesUsed() + bytesFree()));
				delete [] (unsigned char *)m_pStackAllocStart;
			}
		}
		
		stack(const stack &) = delete;
		stack & operator = (const stack &) = delete;
		
		void * base() const {return m_pStackAllocStart;}
		void * top() const { return m_pStackTop;}
		void * current() const {
			void * ret;
			ret = &ret;
			return ret;
		}
		
		size_t bytesUsed() const
		{
			return ((uintptr_t)top() - (uintptr_t)current());
		}
		
		size_t bytesFree() const
		{
			return ((uintptr_t)current() - (uintptr_t)base());
		}
		
	};
	
	class koroutine
	{
		bool m_buffValid;
		jmp_buf m_buff;
		stack m_stack;
		std::function<void(koroutine *)> m_function;
		
		static void remoteCall(void * stackTop, void (*fn)(void *), void * data){
			asm volatile(
				"pushq %%rbp;"
				"movq %%rsp, %%rbp;"
				"movq %[stackTop], %%rsp;"
				"movq %[data], %%rdi;"
				"callq *%[fn];"
				"leave;"
				: /*No output*/
				:[stackTop]"r"(stackTop), [fn]"r"(fn), [data]"r"(data)
				:/*clobber list*/
			);
		}

		static void _remote_entry(void * data)
		{
			((koroutine*)data)->remote_entry();
		}
		
		void remote_entry()
		{
			auto longjumped = setjmp(m_buff);
			if (!longjumped) {
				m_buffValid = true;
				xchange();
				m_function(this);
				m_buffValid = false;
				longjmp(m_buff,1);
			}
			return;
		}
	public:
		koroutine(stack &&s, std::function<void(koroutine *)> f): m_buffValid(false), m_stack(std::move(s)), m_function(f)
		{
			remoteCall(m_stack.top(), _remote_entry, this);
		}
		
		const stack & stack() const
		{
			return m_stack;
		}
		
		koroutine (const koroutine &) = delete;
		koroutine & operator = (const koroutine &) = delete;
		~koroutine()
		{
			assert(!m_buffValid && "koroutine is dying, but we still have a valid jmp buff");
		}
		//The current thread starts executing the stored kontext
		//The calling kontext is then stored in this
		void xchange()
		{
			assert(m_buffValid);
			jmp_buf target;
			memcpy(&target, &m_buff, sizeof(target));
			auto longjumped = setjmp(m_buff);
			if (longjumped)
				return;
			longjmp(target, 1);
		}
	};
};

namespace kompute{
	//Execution kontext - coroutine
	
	class kontext;
	
	typedef std::function<void (kontext &)> processor;
	
	class thread_pool
	{
		std::vector<std::thread> m_pool;
	public:
		thread_pool(std::function<std::function<void(void)>(void)>);
		~thread_pool();
	};
}
namespace kompute {
	
	class kontext
	{
	private:
		//processor m_processor;
		//processor & proc() {return m_processor;}

		std::unique_ptr<thread_pool> m_pool;
		void worker(size_t workerID);
		
		std::mutex m_workerMutex;
		std::condition_variable m_workerCondition;
		size_t m_nTotalWorkers;
		
		struct Job
		{
			std::unique_ptr<koroutine::koroutine> job_routine;
		};
		
		std::vector<Job> m_readyQ;
		std::mutex m_readyQMutex;
		std::condition_variable m_readyQCondition;
		bool m_bWorkerQuit;
		
		
	public:
		kontext();
		//kontext(kontext * parent);

		const koroutine::koroutine & koroutine() {return *m_readyQ[0].job_routine;}
		virtual ~kontext();
	};

	static void spawn(processor);
};

using namespace kompute;

thread_pool::thread_pool(std::function<std::function<void(void)>(void)> createWorker)
{
	auto nCores = std::thread::hardware_concurrency();
	if (nCores == 0)
	{
		nCores = 16;
	}
	
	printf("Cores %u\n", nCores);
	
	while (nCores--) {
		m_pool.emplace_back(std::thread(createWorker()));
	}
}

thread_pool::~thread_pool()
{
	for(auto & thread: m_pool)
	{
		if (thread.joinable())
			thread.join();
	}
}


void kompute::spawn(kompute::processor p){
	kontext k;
}

kontext::kontext()
{
	m_bWorkerQuit = false;
	m_nTotalWorkers = 0;
	m_pool.reset(new thread_pool([this] () {
		size_t id;
		{
			std::unique_lock<std::mutex> lock(m_workerMutex);
			id = m_nTotalWorkers++;
			m_workerCondition.notify_one();
		}
		return [this, id] () {
			this->worker(id);
			{
				std::unique_lock<std::mutex> lock(m_workerMutex);
				m_nTotalWorkers--;
				m_workerCondition.notify_one();
			}
		};
	}));
	/*
	m_koroutine.reset(new koroutine::koroutine(koroutine::stack(), [this] (koroutine::koroutine *) {
		m_processor(*this);
	}));
	m_koroutine->xchange();
	 */
}

void kontext::worker(size_t workerID)
{
	printf("Enter Worker %d\n", (int)workerID);
	{
		std::unique_lock<std::mutex> lock(m_readyQMutex);
		while (!m_bWorkerQuit && !m_readyQ.size())
		{
			m_readyQCondition.wait(lock);
		}
	}
	printf("Exit Worker %d\n", (int)workerID);
}

kontext::~kontext()
{
	{
		std::unique_lock<std::mutex> lock(m_readyQMutex);
		m_bWorkerQuit = true;
		m_readyQCondition.notify_all();
	}
	{
		std::unique_lock<std::mutex> lock(m_workerMutex);
		while(m_nTotalWorkers)
		{
			m_workerCondition.wait(lock);
		}
	}
}

int main() {
	spawn([] (kontext & ktx) {
		printf("%d bytes free, %d bytes used\n", (int)ktx.koroutine().stack().bytesFree(), (int)ktx.koroutine().stack().bytesUsed());
	});
	return 0;
}

