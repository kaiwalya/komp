#include "kaam/header.hpp"
#include <iostream>
#include <assert.h>
using namespace flow;
using namespace flow::native;

class HelloWorldSender: public BlockDeclaration{
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

class HelloWorldReceiver: public BlockDeclaration{
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

int main(int argc, char ** argv) {
	Context ctx;
	ctx.createBlock<HelloWorldSender>();
	ctx.createBlock<HelloWorldReceiver>();
}

