# ==============================================================================
# Advanced Module System (AMS) CMake File
# ==============================================================================

ADD_SUBDIRECTORY(DEBIAN)

INSTALL(DIRECTORY
            software
        DESTINATION
            share/dpkg/personal
        FILE_PERMISSIONS
            OWNER_READ OWNER_WRITE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        DIRECTORY_PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        )
