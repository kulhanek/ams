# ==============================================================================
# Advanced Module System (AMS) CMake File
# ==============================================================================

INSTALL(FILES
            ams-rsync-push
       DESTINATION
            share/sync/rsync
        PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        )

INSTALL(FILES
            ams-exclude-rules-core
            ams-exclude-rules-dpkg
            ams-exclude-rules-site
            ams-exclude-rules-ctrl
            ams-exclude-rules-blds
       DESTINATION
            share/sync/rsync
        PERMISSIONS
            OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
        )
