find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)

# ── tst_shapes: serialisation round-trips ─────────────────────────────────────
add_executable(tst_shapes
    tst_shapes.cpp
    ../shapes.cpp
)
target_include_directories(tst_shapes PRIVATE ..)
target_link_libraries(tst_shapes PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME tst_shapes COMMAND tst_shapes)

# ── tst_commands: undo/redo command stack ─────────────────────────────────────
add_executable(tst_commands
    tst_commands.cpp
    ../shapes.cpp
    ../commands.cpp
    ../drawingscene.cpp
    ../shapedialog.cpp
)
target_include_directories(tst_commands PRIVATE ..)
target_link_libraries(tst_commands PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME tst_commands COMMAND tst_commands)
