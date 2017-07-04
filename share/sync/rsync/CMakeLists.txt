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
            ams-core-exclude-rules
            ams-dpkg-exclude-rules
            ams-site-exclude-rules
            ams-ctrl-exclude-rules
       DESTINATION
            share/sync/rsync
        PERMISSIONS
            OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
        )
