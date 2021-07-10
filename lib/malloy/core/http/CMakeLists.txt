add_subdirectory(session)
add_subdirectory(filters)

target_sources(
    ${TARGET}
    PUBLIC
		cookie.hpp
        generator.hpp
        http.hpp
        request.hpp
        response.hpp
		types.hpp
        utils.hpp
	PRIVATE
		cookie.cpp
		generator.cpp
)
