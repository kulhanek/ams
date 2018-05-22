# ==============================================================================
# AMS CMake File
# ==============================================================================

INCLUDE_DIRECTORIES(lscpu)
INCLUDE_DIRECTORIES(lscpu/libsmartcols/src)
INCLUDE_DIRECTORIES(lscpu/util-linux/include)

SET(LSCPU_SRC
    lscpu/libsmartcols/src/iter.c
    lscpu/libsmartcols/src/version.c
    lscpu/libsmartcols/src/column.c
    lscpu/libsmartcols/src/table_print.c
    lscpu/libsmartcols/src/line.c
    lscpu/libsmartcols/src/init.c
    lscpu/libsmartcols/src/symbols.c
    lscpu/libsmartcols/src/table.c
    lscpu/libsmartcols/src/cell.c

    lscpu/util-linux/src/path.c
    lscpu/util-linux/src/strutils.c
    lscpu/util-linux/src/colors.c
    lscpu/util-linux/src/mbsalign.c
    lscpu/util-linux/src/cpuset.c
    lscpu/util-linux/src/ttyutils.c
    lscpu/util-linux/src/color-names.c

    lscpu/lscpu.c
    lscpu/lscpu-dmi.c
    )

ADD_LIBRARY(amslscpu STATIC ${LSCPU_SRC})
TARGET_COMPILE_OPTIONS(amslscpu PRIVATE -include config.h -fPIC)

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
TARGET_LINK_LIBRARIES(ams_shared amslscpu)

SET_TARGET_PROPERTIES(ams_shared PROPERTIES
                        OUTPUT_NAME ams
                        CLEAN_DIRECT_OUTPUT 1
                        VERSION ${AMS_VERSION})
INSTALL(TARGETS
            ams_shared
        LIBRARY DESTINATION
            lib)

