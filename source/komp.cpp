#include "komp/komp.hpp"
#include "log.hpp"

using namespace komp;
using namespace komp::native;


Context::Context(){
	m_nBlockCountCurrent = m_nBlockCountTotal = 0;
	using MapElement = std::shared_ptr<BlockListInfo>;
	m_blocks.emplace(BlockState::Initialize, MapElement(new BlockListInfo));
	m_blocks.emplace(BlockState::Running, MapElement(new BlockListInfo));
	m_blocks.emplace(BlockState::Finalize, MapElement(new BlockListInfo));
}

Context::~Context() {
	blockCountZero();
	m_pool.stop();
	
	//thislog("blockCountTotal: %d", (int)m_nBlockCountTotal)
}

void Context::initialize(BlockInfoIter it) {
	{
		auto & srcli = findListInfo(BlockState::Initialize);
		auto & destli = findListInfo(BlockState::Running);
		auto srcLock = srcli.lock();
		auto destLock = destli.lock();
		auto & srcList = srcli.list();
		auto & destList = destli.list();
		assert(srcList.size());
		destList.splice(destList.begin(), srcList, it);
	}
	m_pool.addJob([this, it](){
		run(it);
	});
}

void Context::run(BlockInfoIter it) {

	InvocationContext ctx;
	(**it).definition->performDefinition(ctx);
	{
		auto & srcli = findListInfo(BlockState::Running);
		auto & destli = findListInfo(BlockState::Finalize);
		auto srcLock = srcli.lock();
		auto destLock = destli.lock();
		auto & srcList = srcli.list();
		auto & destList = destli.list();
		assert(srcList.size());
		destList.splice(destList.begin(), srcList, it);
	}
	m_pool.addJob([this, it](){
		finalize(it);
	});
}

void Context::finalize(BlockInfoIter it) {
	BlockInfoPtr bi = *(it);
	{
		auto & srcli = findListInfo(BlockState::Finalize);
		auto srcLock = srcli.lock();
		auto & srcList = srcli.list();
		assert(srcList.size());
		srcList.erase(it);
	}
	delete bi->definition;
	delete bi;
	blockCountDecr();
}