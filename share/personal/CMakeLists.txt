# ==============================================================================
# Advanced Module System (AMS) CMake File
# ==============================================================================

INSTALL(FILES
            ams-personal-gensshkeys
            ams-personal-showsshkey
            ams-personal-useradd
            ams-personal-addbuilds
            ams-personal-rmbuilds
            ams-personal-envinit
            ams-personal
       DESTINATION
            share/personal
        PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        )

