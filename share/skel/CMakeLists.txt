# ==============================================================================
# Advanced Module System (AMS) CMake File
# ==============================================================================

INSTALL(FILES
            .bash_profile
            .bashrc
       DESTINATION
            share/skel
        PERMISSIONS
            OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
        )
