#if !defined THREAD_UTILS_HPP
#define THREAD_UTILS_HPP
#include <memory>
#include <thread>
#include <map>
#include <assert.h>

namespace komp
{
	namespace thread
	{
		template<typename tshared, typename tlocal>
		class localized
		{
		private:
			tshared * m_shared;
		public:
			localized(tshared * shared): m_shared(shared)
			{
				shared->tlocalcontext_make();
			}
			~localized()
			{
				m_shared->tlocalcontext_delete();
			}
			
			tshared * tsharedcontext()
			{
				return m_shared;
			}
			
			tlocal * tlocalcontext_fork()
			{
				return m_shared->tlocalcontext_make();
			}
		};
		
		template<typename tshared, typename tlocal>
		class localizable
		{
		private:
			
			typedef std::map<std::thread::id, tlocal *> tlocal_map;
			tlocal_map m_localmap;
			std::mutex m_localmap_mutex;
			tshared * m_shared;
			
			tlocal * locked_create_and_add_to_map(std::thread::id & tid)
			{
				auto ret = tlocalcontext_create();
				m_localmap.emplace(std::make_pair(tid, ret));
				return ret;
			}
		private:
			tlocal * tlocalcontext_create()
			{
				return new tlocal(m_shared);
			}
		public:
			localizable()
			{
				m_shared = static_cast<tshared *>(this);
			}
			
			virtual ~localizable()
			{
			}

			tlocal * tlocalcontext_find()
			{
				auto tid = std::this_thread::get_id();
				std::unique_lock<decltype(m_localmap_mutex)> locl(m_localmap_mutex);
				if (m_localmap.find(tid) == m_localmap.end())
					return nullptr;
				return m_localmap[tid];
			}
			
			tlocal * tlocalcontext_make()
			{
				auto tid = std::this_thread::get_id();
				std::unique_lock<decltype(m_localmap_mutex)> locl(m_localmap_mutex);
				assert(m_localmap.find(tid) == m_localmap.end());
				return locked_create_and_add_to_map(tid);
			}
			
			tlocal * tlocalcontext_find_or_make()
			{
				auto tid = std::this_thread::get_id();
				std::unique_lock<decltype(m_localmap_mutex)> locl(m_localmap_mutex);
				if (m_localmap.find(tid) == m_localmap.end())
					return locked_create_and_add_to_map(tid);
				return m_localmap[tid];
			}
			
			void tlocalcontext_delete()
			{
				bool bDelete = false;
				auto tid = std::this_thread::get_id();
				{
					std::unique_lock<decltype(m_localmap_mutex)> locl(m_localmap_mutex);
					m_localmap.erase(tid);
					if(!m_localmap.size()) bDelete = true;
				}
				if (bDelete) delete this;
			}
		};
		
		void test();
	}
};

#endif