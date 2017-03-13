# ==============================================================================
# AMS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(LISTDIRSCMD_SRC
        ListDirsCmd.cpp
        ListDirsCmdOptions.cpp
        NodeItem.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(ams-list-dirs ${LISTDIRSCMD_SRC})

TARGET_LINK_LIBRARIES(ams-list-dirs ${AMS_LIBS})

INSTALL(TARGETS
            ams-list-dirs
        DESTINATION
            bin
        )

