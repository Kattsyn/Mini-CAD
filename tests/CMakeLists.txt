find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)

set(SRC ${CMAKE_SOURCE_DIR}/src)

# ── tst_shapes: serialisation round-trips ─────────────────────────────────────
add_executable(tst_shapes
    tst_shapes.cpp
    ${SRC}/model/shapes.cpp
)
target_include_directories(tst_shapes PRIVATE
    ${SRC}/model
    ${SRC}/commands
    ${SRC}/scene
    ${SRC}/ui
    ${SRC}
)
target_link_libraries(tst_shapes PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME tst_shapes COMMAND tst_shapes)

# ── tst_commands: undo/redo command stack ─────────────────────────────────────
add_executable(tst_commands
    tst_commands.cpp
    ${SRC}/model/shapes.cpp
    ${SRC}/commands/commands.cpp
    ${SRC}/scene/drawingscene.cpp
    ${SRC}/ui/shapedialog.cpp
)
target_include_directories(tst_commands PRIVATE
    ${SRC}/model
    ${SRC}/commands
    ${SRC}/scene
    ${SRC}/ui
    ${SRC}
)
target_link_libraries(tst_commands PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME tst_commands COMMAND tst_commands)
