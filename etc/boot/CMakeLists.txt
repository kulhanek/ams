# ==============================================================================
# Advanced Module System (AMS) CMake File
# ==============================================================================

INSTALL(FILES
            bashrc.ncbr
            bashrc.it4i
            dashrc.ams
            init.ams
            bash_env
        DESTINATION
            etc/boot
        PERMISSIONS
            OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
        )
