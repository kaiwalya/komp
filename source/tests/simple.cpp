#include "komp/komp.hpp"
#include <iostream>
#include <assert.h>

using namespace komp;
using namespace komp::native;

class HelloWorldSender: public BlockDefinition{
	void defineInterface(InterfaceProgrammer & programmar) const {
		programmar.setBlockType(BlockType::NoSideEffect);
		programmar.setOutputInterface<Stream<Char>>();
	}
	
	void performDefinition(InvocationContext & ctx) {
		auto stream = ctx.getOutput<Stream<Char>>(0);
		stream.next();
		char msg[] = "hello world!\n";
		for (auto & c: msg) {
			*stream.data() = c;
			stream.next();
		}
	}
};

class HelloWorldReceiver: public BlockDefinition{
	void defineInterface(InterfaceProgrammer & programmar) const {
		programmar.setBlockType(BlockType::ExternalState);
		programmar.setInputInterface<Stream<Char>>();
	}
	
	void performDefinition(InvocationContext & ctx) {
		auto & stream = ctx.getInput<Stream<Char>>(0);
		while(stream.next()) {
			std::cout << *stream.data();
		}
	}
};
#include <assert.h>

int main(int argc, char ** argv) {

	Context ctx;
	auto b1 = ctx.createBlock<HelloWorldSender>();
	auto b2 = ctx.createBlock<HelloWorldReceiver>();
	ctx.join(b1, b2);
}

