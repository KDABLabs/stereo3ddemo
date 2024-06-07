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
#else
#include <shared/spacemouse.h>
namespace all {
using SpacemouseImpl = Spacemouse;
}
#endif // WITH_NAVLIB
