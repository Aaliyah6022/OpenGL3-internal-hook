#pragma once

#include <vector>
#include <Windows.h>
#include <TlHelp32.h>

namespace mem
{
	uintptr_t FindDMAAdy(HANDLE hproc, uintptr_t ptr, std::vector<unsigned int> offsets);
	uintptr_t FindDMAAdy(uintptr_t ptr, std::vector<unsigned int> offsets);
}