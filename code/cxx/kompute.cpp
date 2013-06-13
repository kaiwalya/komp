#include "kompute.hpp"
#include "komp.hpp"

namespace komp
{
	virtual_destructor::~virtual_destructor()
	{
		
	}

	namespace mq
	{
		ktx * ktx::create()
		{
			return new ctx_thread();
		}
	};
};