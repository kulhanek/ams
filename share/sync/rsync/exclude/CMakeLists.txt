# ==============================================================================
# Advanced Module System (AMS) CMake File
# ==============================================================================

INSTALL(FILES
            ams-exclude-rules-core
            ams-exclude-rules-bundle-core
       DESTINATION
            share/sync/rsync/exclude
        PERMISSIONS
            OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
        )
