# Suppresses warnings from third-party libraries by appending INTERFACE_INCLUDE_DIRECTORIES
# to INTERFACE_SYSTEM_INCLUDE_DIRECTORIES for a specified target.
function(suppress_third_party_warnings target_name)
    get_target_property(existing_system_include_dirs "${target_name}" INTERFACE_SYSTEM_INCLUDE_DIRECTORIES)
    get_target_property(interface_include_dirs "${target_name}" INTERFACE_INCLUDE_DIRECTORIES)

    if(NOT interface_include_dirs)
        message(WARNING "Target '${target_name}' does not have any INTERFACE_INCLUDE_DIRECTORIES set.")
        return()
    endif()

    if(existing_system_include_dirs)
        list(APPEND existing_system_include_dirs ${interface_include_dirs})
    else()
        set(existing_system_include_dirs "${interface_include_dirs}")
    endif()

    set_target_properties("${target_name}" PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${existing_system_include_dirs}")
endfunction()
