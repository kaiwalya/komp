#include "komp/typ.hpp"

#include <iostream>

using namespace komp::type;

int main() {
	
	{
		using Stream1 = Managed<int[]>;
		Stream1 val;
		std::cout << "Stream: " <<typeid(Stream1).name() << std::endl;
	}

	{
		using Array = Managed<int[5]>;
		Array val;
		std::cout << "Array: " <<typeid(Array).name() << " " << Array::nTotalUnits << std::endl;
	}

	{
		using Array = Managed<int[9][5][2]>;
		Array val;
		std::cout << "ArrayOfArrays: " <<typeid(Array).name() << " " << Array::nTotalUnits << std::endl;
	}

	{
		using StreamOfArrays = Managed<int[][5]>;
		StreamOfArrays val;
		std::cout << "StreamOfArrays: " <<typeid(StreamOfArrays).name() << std::endl;
	}
 

	return 0;
}