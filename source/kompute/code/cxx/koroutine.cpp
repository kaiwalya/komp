#include "koroutine.hpp"


#include "koro.hpp"
/////////
//def ktx
//
namespace koro
{
	ktx * ktx::create()
	{
		return new ctx_thread();
	}
	ktx::ktx(){}
	ktx::~ktx(){}
};

