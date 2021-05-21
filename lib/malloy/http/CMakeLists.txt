add_subdirectory(connection)
add_subdirectory(session)

target_sources(
    ${TARGET_OBJS}
    PRIVATE
		cookie.cpp
		cookie.hpp
        generator.cpp
        generator.hpp
        http.hpp
        request.hpp
        response.hpp
		types.hpp
        uri.cpp
        uri.hpp
)
