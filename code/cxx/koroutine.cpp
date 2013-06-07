#include "koroutine.hpp"


#include "ko.hpp"
/////////
//def ktx
//
namespace ko
{
	ktx * ktx::create()
	{
		return (new ctx)->getCurrentThreadContext();
	}
	ktx::ktx(){}
	ktx::~ktx(){}
};

