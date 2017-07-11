# ==============================================================================
# AMS CMake File
# ==============================================================================

# objects in library -----------------------------------------------------------
SET(AMS_SRC
        AMSMainHeader.cpp
    # system -------------------------------------
        system/Shell.cpp
        system/ShellProcessor.cpp
        system/AMSGlobalConfig.cpp
        system/AMSCompletion.cpp
        system/FSIndex.cpp
        system/sha1.cpp

    # software -----------------------------------
        soft/AMSUserConfig.cpp
        soft/SoftStat.cpp
        soft/Actions.cpp
        soft/AmsUUID.cpp
        soft/Cache.cpp
        soft/Host.cpp
        soft/User.cpp
        soft/Map.cpp
        soft/PrintEngine.cpp
        soft/Site.cpp
        soft/Utils.cpp

    # cuda ---------------------------------------
        cuda/Cuda.cpp

    # services -----------------------------------
        srv/ServerWatcher.cpp
        )

# create shared library --------------------------------------------------------
ADD_DEFINITIONS(-DAMS_BUILDING_DLL)
ADD_LIBRARY(ams_shared SHARED ${AMS_SRC})
SET_TARGET_PROPERTIES(ams_shared PROPERTIES
                        OUTPUT_NAME ams
                        CLEAN_DIRECT_OUTPUT 1
                        VERSION ${AMS_VERSION})
INSTALL(TARGETS
            ams_shared
        LIBRARY DESTINATION
            lib)

