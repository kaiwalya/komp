#include "komp/komp.hpp"
#include <iostream>
#include <assert.h>
#include <atomic>

#include <unistd.h>

using namespace komp;
using namespace komp::native;

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
		programmar.setOutputInterface<Stream<Char>>();
	}
	
	void performDefinition(InvocationContext & ctx) {
		incr();
		/*
		auto stream = ctx.getOutput<Stream<Char>>(0);
		stream.next();
		char msg[] = "hello world!\n";
		for (auto & c: msg) {
			*stream.data() = c;
			stream.next();
		}
		 */
	}
};

class HelloWorldReceiver: public BlockDefinition{
	void defineInterface(InterfaceProgrammer & programmar) const {
		programmar.setBlockType(BlockType::ExternalState);
		programmar.setInputInterface<Stream<Char>>();
	}
	
	void performDefinition(InvocationContext & ctx) {
		decr();
		/*
		auto & stream = ctx.getInput<Stream<Char>>(0);
		while(stream.next()) {
			std::cout << *stream.data();
		}
		 */
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

