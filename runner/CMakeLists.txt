add_executable(zi-runner zi_entry_point.c)

target_link_libraries(zi-runner PRIVATE
				zi-runtime
)