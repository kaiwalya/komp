	//
	//  koroutine.hpp
	//  Code
	//
	//  Created by Kaiwalya Kher on 6/5/13.
	//  Copyright (c) 2013 Kaiwalya Kher. All rights reserved.
	//

#ifndef Code_koroutine_hpp
#define Code_koroutine_hpp

#include <cstddef>
#include <cstring>
#include <assert.h>
#include <functional>
#include <memory>

namespace ko
{
	/**
	 The root kontext. Entry point to access most of the api
	 */
	class ktx
	{
	public:
		ktx();
		virtual ~ktx();
		ktx(const ktx &) = delete;
		ktx & operator = (const ktx &) = delete;
		
		/**
		Reference the same context but from a new thread.
		*/
		virtual ktx * attachThread() = 0;
		
		/**
		Factory to create ktx instances. Use on calling thread only
		*/
		static ktx * create();

	};
	
}
#endif
