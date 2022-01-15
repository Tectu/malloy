target_sources(
    ${TARGET}
    PRIVATE
        sqlite3.c
        sqlite3.h
)

target_link_libraries(
    ${TARGET}
    PRIVATE
        $<$<BOOL:${UNIX}>:dl>
        $<$<BOOL:${UNIX}>:pthread>
)
