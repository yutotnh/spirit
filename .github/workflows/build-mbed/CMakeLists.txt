# spirit が Mbed CE (Mbed OS のコミュニティ版) でビルドできることを確認するための、アプリケーションのプロジェクト
# 参考: https://github.com/mbed-ce/mbed-ce-hello-world
cmake_minimum_required(VERSION 3.19)
cmake_policy(VERSION 3.19...3.22)

set(MBED_APP_JSON_PATH mbed_app.json5)

include(mbed-os/tools/cmake/mbed_toolchain_setup.cmake)

project(build-mbed
    LANGUAGES C CXX ASM
)

include(mbed_project_setup)

add_subdirectory(mbed-os)

# spirit は、 mbed-os の後に追加する(mbed-core-flags が必要なため)
add_subdirectory(spirit)

add_executable(${PROJECT_NAME}
    main.cpp
)

# mbed-os ライブラリは、実行ファイルにだけリンクする (他のライブラリは mbed-core-flags にリンクする)
target_link_libraries(${PROJECT_NAME} mbed-os spirit spirit-platform-mbed)

mbed_set_post_build(${PROJECT_NAME})
