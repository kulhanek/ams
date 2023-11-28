# ==============================================================================
# AMS CMake File
# ==============================================================================

# objects in library -----------------------------------------------------------
SET(AMS_SRC
        AMSMainHeader.cpp

    # BASE 
        base/AmsUUID.cpp
        base/Shell.cpp
        base/ShellProcessor.cpp
        base/Utils.cpp
        base/CudaRT.cpp
        base/AMSRegistry.cpp
        base/PrintEngine.cpp

    # HOST
        host/HostGroup.cpp
        host/Host.cpp
        host/components/HostSubSystem.cpp
        host/components/HostDefault.cpp
        host/components/HostOS.cpp
        host/components/HostCPU.cpp
        host/components/HostGPU.cpp
        host/components/HostGPU-NVidia.cpp
        host/components/HostDesktop.cpp
        host/components/HostNetwork.cpp
        host/components/HostCompat.cpp

    # USER
        user/UserUtils.cpp
        user/User.cpp

    # SITE
        site/SiteController.cpp
        site/Site.cpp

    # MODULES
        mods/ModUtils.cpp
        mods/ModCache.cpp
        mods/ModBundle.cpp
        mods/ModuleController.cpp
    )

# create shared library --------------------------------------------------------
ADD_DEFINITIONS(-DAMS_BUILDING_DLL)
ADD_LIBRARY(ams_shared SHARED ${AMS_SRC})
TARGET_LINK_LIBRARIES(ams_shared
        ${HWLOC_LIB_NAME}
        ${W3TK_LIB_NAME}
        ${HIPOLY_LIB_NAME}
        )

SET_TARGET_PROPERTIES(ams_shared PROPERTIES
                        OUTPUT_NAME ams
                        CLEAN_DIRECT_OUTPUT 1
                        VERSION ${AMS_VERSION})
INSTALL(TARGETS
            ams_shared
        LIBRARY DESTINATION
            lib)

