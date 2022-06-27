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
				utils.hpp

	PRIVATE
		cookie.cpp
		generator.cpp
)
