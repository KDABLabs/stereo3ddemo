#pragma once
#if defined(WITH_NAVLIB)
#include <shared/spacemouse_navlib.h>
namespace all {
using SpacemouseImpl = SpacemouseNavlib;
}

#elif defined(WITH_SPNAV)
#include <shared/spacemouse_spnav.h>
namespace all {
using SpacemouseImpl = SpacemouseSpnav;
}
#endif // WITH_NAVLIB
