ADD_EXECUTABLE(test_load_page)
TARGET_SOURCES(test_load_page PRIVATE test_load_page.cpp)
TARGET_LINK_LIBRARIES(
    test_load_page
    PRIVATE
        growable_buffer
        Boost::ut
        spdlog::spdlog
)

ADD_TEST(NAME test_load_page COMMAND $<TARGET_FILE:test_load_page>)
