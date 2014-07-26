#pragma once
#include "compile.h"
#include "gcc.h"
#include <ostream>

void link_and_emit(std::ostream& os, const PreLink& pl);
