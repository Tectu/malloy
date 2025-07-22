target_sources(
    ${TARGET}
    PUBLIC
		FILE_SET
			HEADERS
			BASE_DIRS ${MALLOY_SERVER_BASE_DIR}
			FILES
				connection.hpp
				connection_detector.hpp
				connection_plain.hpp
				connection_t.hpp
				connection_tls.hpp
				preflight_config.hpp
				request_generator_t.hpp

	PRIVATE
		connection_detector.cpp
		preflight_config.cpp
)
