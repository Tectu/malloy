#pragma once

#include <fmt/format.h>

#if (FMT_VERSION >= (8 * 10000)) // fmt version is major * 10,000 ...
    #define MALLOY_DETAIL_HAS_FMT_8 1
#else
    #define MALLOY_DETAIL_HAS_FMT_8 0
#endif
