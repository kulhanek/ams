# ==============================================================================
# AMS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(ROOT_SRC
        Root.cpp
        RootOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(ams-root ${ROOT_SRC})
ADD_DEPENDENCIES(ams-root ams_shared)

TARGET_LINK_LIBRARIES(ams-root ${AMS_LIBS})

INSTALL(TARGETS
            ams-root
        DESTINATION
            bin
        )

