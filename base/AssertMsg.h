/*
* AssertMsg
*
* Copyright (C) 2023 by Sebastien Berube
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#pragma once

#ifdef NDEBUG
#define ASSERT_MSG(condition) ((void)0)
#else
#include <iostream>
#include <assert.h>
#define ASSERT_MSG(exp, msg) { if(!(exp)) { std::cerr << (msg); } assert(exp); }
#endif