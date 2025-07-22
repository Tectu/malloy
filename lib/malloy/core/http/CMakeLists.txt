add_subdirectory(session)
add_subdirectory(filters)

target_sources(
    ${TARGET}
    PUBLIC
		FILE_SET
			HEADERS
			BASE_DIRS ${MALLOY_CORE_BASE_DIR}
			FILES
				cookie.hpp
				generator.hpp
				http.hpp
				request.hpp
				response.hpp
				type_traits.hpp
				types.hpp
				url.hpp
				utils.hpp

	PRIVATE
		cookie.cpp
		generator.cpp
)

target_link_libraries(
    ${TARGET}
    PUBLIC
        Boost::url
)
