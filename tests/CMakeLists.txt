find_package(GTest REQUIRED)

add_executable(test_utf8 test_utf8.cpp ${CMAKE_SOURCE_DIR}/src/utf8.cpp)
target_link_libraries(test_utf8 PRIVATE utf8proc GTest::Main GTest::GTest)
target_include_directories(test_utf8 PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_compile_features(test_utf8 PRIVATE cxx_std_20)
