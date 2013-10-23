#include "komp/komp.hpp"
#include <iostream>
#include <assert.h>
#include <atomic>

#include <unistd.h>

using namespace komp;
using namespace komp::typ;

int gCount;
std::mutex gmutex;

void incr() {
	std::unique_lock<std::mutex> lock(gmutex);
	gCount++;
}
void decr() {
	std::unique_lock<std::mutex> lock(gmutex);
	gCount++;
}

class HelloWorldSender: public BlockDefinition{
	void defineInterface(InterfaceProgrammer & programmar) const {
		programmar.setBlockType(BlockType::NoSideEffect);
	}
	
	void performDefinition(InvocationContext & ctx) {
		auto & stream = ctx.getOutput<char []>(0);
		char msg[] = "hello world!\n";
		for (auto & c: msg) {
			stream << c;
		}
	}
};

class HelloWorldReceiver: public BlockDefinition{
	void defineInterface(InterfaceProgrammer & programmar) const {
		programmar.setBlockType(BlockType::ExternalState);
	}
	
	void performDefinition(InvocationContext & ctx) {
		auto & stream = ctx.getInput<char []>(0);
		for (auto it: stream) {
			std::cout << *it;
		}
	}
};
#include <assert.h>

int main(int argc, char ** argv) {
	gCount = 0;
	{

		const int nIterations = 10;
		for(int iIteration = 0; iIteration < nIterations; iIteration++) {
			Context ctx;
			auto b1 = ctx.createBlock<HelloWorldSender>();
			auto b2 = ctx.createBlock<HelloWorldReceiver>();
			ctx.join(b1, b2);
		}
	}
	{
		Context ctx;
		const int nIterations = 10;
		for(int iIteration = 0; iIteration < nIterations; iIteration++) {
			auto b1 = ctx.createBlock<HelloWorldSender>();
			auto b2 = ctx.createBlock<HelloWorldReceiver>();
			ctx.join(b1, b2);
		}
	}

	assert(gCount == 40);
	std::unique_lock<std::mutex> lock(gmutex);
	std::cout << "a = a + "<<gCount << std::endl;
	std::cout << "a" << std::endl;
}

