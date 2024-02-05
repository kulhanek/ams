# ==============================================================================
# Advanced Module System (AMS) CMake File
# ==============================================================================

ADD_SUBDIRECTORY(DEBIAN)

INSTALL(DIRECTORY
            software
            usr
            etc
        DESTINATION
            share/dpkg/personal
        FILE_PERMISSIONS
            OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
        DIRECTORY_PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        )

