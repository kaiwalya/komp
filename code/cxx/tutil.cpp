#include "tutil.hpp"
#include "log.hpp"


namespace komp
{
	namespace thread
	{
		
		class test_localize_shared;
		class test_localize_local: public localized<test_localize_shared, test_localize_local>
		{

		public:
			test_localize_local(test_localize_shared * shared)
			:localized<komp::thread::test_localize_shared, komp::thread::test_localize_local>(shared)
			{
			}
		};
		
		class test_localize_shared: public localizable<test_localize_shared, test_localize_local>
		{
		};
		
		void test()
		{
			auto local = new test_localize_local(new test_localize_shared());
			
			assert(local == local->tsharedcontext()->tlocalcontext_find());
			
			std::thread t([local](){
				std::unique_ptr<test_localize_local> l(local->tlocalcontext_fork());
				assert(l.get() != local);
				assert(l.get() == local->tsharedcontext()->tlocalcontext_find());
				assert(l.get() == local->tsharedcontext()->tlocalcontext_find_or_make());
			});
			t.join();
			delete local;

		}
	}
}
