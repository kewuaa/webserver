add_executable(test_load_page)
target_sources(test_load_page PRIVATE test_load_page.cpp)
target_link_libraries(
    test_load_page
    PRIVATE
        growable_buffer
        Boost::ut
        spdlog::spdlog
)

add_test(NAME test_load_page COMMAND $<TARGET_FILE:test_load_page>)
