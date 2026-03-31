add_executable(yuki
      main.cpp
      editor.cpp
      gap_buffer.cpp
      utf8.cpp
)

target_include_directories(yuki PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(yuki PRIVATE utf8proc)

target_compile_features(yuki PRIVATE cxx_std_20)
