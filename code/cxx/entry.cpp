#include <memory>
#include <functional>
#include <stdint.h>
#include <tuple>
#include <vector>
#include <string>
#include <memory>
#include <thread>
#include <setjmp.h>


#include "setjmp.h"

extern "C" void remote_call_internal ();

#pragma warning(push)
#pragma warning(disable:4731)

static void remoteCall(void * stacklocation, size_t stacksize, void (*fn)(void *), void * data){
	auto pBuff = (jmp_buf *)((unsigned char *)stacklocation + stacksize);
	asm volatile(
		"pushq %%rbp;"
		"movq %%rsp, %%rbp;"
		"movq %[pBuff], %%rsp;"
		"movq %[data], %%rdi;"
		"callq *%[fn];"
		"leave;"
		: /*No output*/
		:[pBuff]"r"(pBuff), [fn]"r"(fn), [data]"r"(data)
		:/*clobber list*/
	);
}

/*
char * pStack = (char *)malloc(nBytes);
if(remoteCall(pStack + nBytes, functionTest, pConsole) == pConsole){
	pConsole->output("Remote Call Test Successfull\n");
}
 */

#pragma warning(pop)


namespace kompute{
	//Execution context - coroutine
	
	class kontext;
	
	typedef std::function<void (kontext &)> processor;
	
	class kontext
	{
	private:
		std::vector<std::thread> freePool;
		std::vector<kontext> readyQ;
		kontext * m_parentKtx;
		kontext * m_rootKtx;
		kontext * parent(){return m_parentKtx;}
		kontext * root(){return m_rootKtx;}
		static void thread_entry(void *);
	public:
		kontext();
		kontext(kontext * parent);
		void handleAndWait(processor);
		virtual ~kontext();
	};

	static void spawn(processor);
};


#include <stdio.h>
using namespace kompute;

void kompute::spawn(kompute::processor p){
	kontext k;
	k.handleAndWait(p);
}

kontext::kontext()
{
	m_parentKtx = nullptr;
	m_rootKtx = this;
}

kontext::kontext(kontext * parent)
{
	m_parentKtx = parent;
	m_rootKtx = parent->root();
}

kontext::~kontext()
{
	
}

void kontext::thread_entry(void * data)
{
	auto d = (int *)data;
	*d = 42;
}

void kontext::handleAndWait(processor p)
{
	if (parent())
		return root()->handleAndWait(p);
	
	if (!freePool.size())
	{
		auto pStackBase = new int64_t[256];
		int param = 0;
		remoteCall(pStackBase, sizeof(int64_t[256]), &thread_entry, (void *)&param);
		param++;
	}
	
}

int main() {
	spawn([] (kontext & ktx) {
	});
	return 0;
}

