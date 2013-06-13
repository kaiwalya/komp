#include "koroutine.hpp"
#include "kompute.hpp"
#include "stack.hpp"
#include "tutil.hpp"
#include <chrono>
#include <thread>
#include <vector>

/*

void parellelTest(std::function<void(size_t)> func)
{
	const size_t nMaxThreads = std::thread::hardware_concurrency() ? std::thread::hardware_concurrency() : 8;
	const size_t nIterations = 0x1000;
	std::atomic_int_least64_t accumulateIterationDuration;

	auto f =[&accumulateIterationDuration, &func, nIterations]()
	{
		auto start = std::chrono::high_resolution_clock::now();
		func(nIterations);
		auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start);
		accumulateIterationDuration += duration.count();
	};
	
	for(size_t nThreads = 1; nThreads <= nMaxThreads; nThreads++)
	{
		accumulateIterationDuration = 0;
		
		std::vector<std::thread> threads;
		for(int i = 0; i < nThreads; i++)
		{
			threads.emplace_back(f);
		}
		
		for(auto & th: threads)
		{
			th.join();
		}
		{
			float f = (float)accumulateIterationDuration;
			f /= (float)nThreads;
			f /= (float)nIterations * nThreads;
			printf("%.2f ns for one iteration distributed over %d threads\n", f, (int)nThreads);
		}
		
	}
}

void koro_ktx_ParellelTest(koro::ktx * ktxIn)
{
	std::unique_ptr<koro::ktx> ktx(ktxIn->attachThread());
}



class qp_logic_generator: public komp::mq::qp_logic
{
public:
	void process(komp::mq::qp_inner * inner)
	{
		printf("In Process\n");
	}
};

void komp_mq_ktx_ParellelTest(komp::mq::ktx * ktxIn)
{
	std::unique_ptr<komp::mq::ktx> ktx(ktxIn->attachThread());
	qp_logic_generator gen;
	ktx->createQProcessor(&gen);
	
}
 
 */

class global
{
	
};

class local
{
	
};

#include <assert.h>

int main(int argc, char ** argv)
{
	/*
	{
		std::unique_ptr<koro::ktx> ktx(koro::ktx::create());
		parellelTest([&ktx](size_t iterations){
			while (iterations) {
				koro_ktx_ParellelTest(ktx.get());
				iterations--;
			}
		});
	}
	{
		std::unique_ptr<komp::mq::ktx> ktx(komp::mq::ktx::create());
		parellelTest([&ktx](size_t iterations){
			while (iterations) {
				komp_mq_ktx_ParellelTest(ktx.get());
				iterations--;
			}
		});
	}
	*/
	
	komp::thread::test();
}



