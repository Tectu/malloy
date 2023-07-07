# Macro for deducing value for _WIN32_WINNT definition
#
# Borrowed from https://stackoverflow.com/questions/9742003/platform-detection-in-cmake
macro(get_WIN32_WINNT version)
    if(CMAKE_SYSTEM_VERSION)
        set(ver ${CMAKE_SYSTEM_VERSION})
        string(REGEX MATCH "^([0-9]+).([0-9])" ver ${ver})
        string(REGEX MATCH "^([0-9]+)" verMajor ${ver})

        # Check for Windows 10, b/c we'll need to convert to hex 'A'.
        if("${verMajor}" MATCHES "10")
            set(verMajor "A")
            string(REGEX REPLACE "^([0-9]+)" ${verMajor} ver ${ver})
        endif()

        # Remove all remaining '.' characters.
        string(REPLACE "." "" ver ${ver})

        # Prepend each digit with a zero.
        string(REGEX REPLACE "([0-9A-Z])" "0\\1" ver ${ver})
        set(${version} "0x${ver}")
    endif()
endmacro()
