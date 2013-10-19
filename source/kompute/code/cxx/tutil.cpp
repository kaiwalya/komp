#include "tutil.hpp"
#include "log.hpp"


namespace komp
{
	namespace thread
	{
		
		void pool::start(std::function<std::function<void(void)>(void)> & createWorker)
		{
			auto nCores = std::thread::hardware_concurrency();
			if (nCores == 0)
			{
				nCores = 16;
			}
			
			while (nCores--) {
				m_pool.emplace_back(std::thread(createWorker()));
			}
		}
		
		void pool::start(std::function<std::function<void(void)>(void)> && createWorker)
		{
			return start(createWorker);
		}
		
		pool::~pool()
		{
			for(auto & thread: m_pool)
			{
				if (thread.joinable())
					thread.join();
			}
		}
		
		class test_localize_local;

		
		class test_localize_shared: public localizable<test_localize_shared, test_localize_local>
		{
		public:
			test_localize_shared()
			{
				thisCheckpoint();
			}
			
			~test_localize_shared()
			{
				thisCheckpoint();
			}
		};
		
		class test_localize_local: public localized<test_localize_shared, test_localize_local>
		{
			
		public:
			test_localize_local(test_localize_shared * shared)
			:localized<komp::thread::test_localize_shared, komp::thread::test_localize_local>(shared)
			{
				tsharedcontext()->tlocalcontext_attach(this);
				thisCheckpoint();
			}
			
			~test_localize_local()
			{
				tsharedcontext()->tlocalcontext_detach(this);
				thisCheckpoint();
			}
		};
		
		void test()
		{
			auto local = new test_localize_local(new test_localize_shared());
			
			assert(local == local->tsharedcontext()->tlocalcontext_find());
			
			std::thread t([local](){
				std::unique_ptr<test_localize_local> l(new test_localize_local(local->tsharedcontext()));
				assert(l.get() != local);
				assert(l.get() == local->tsharedcontext()->tlocalcontext_find());
			});
			t.join();
			delete local;

		}
	}
}
