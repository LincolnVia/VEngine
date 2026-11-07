# Sources shared by the game and editor.
target_include_directories(Box3dEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(Box3dEngine PRIVATE
    engine/engine.cpp
    engine/box3d_world.cpp
    engine/world/world.cpp
    engine/world/entityRegistry.cpp
    engine/world/quakeBsp.cpp
    engine/template/first_person_character.cpp
)

target_sources(Box3dGame PRIVATE game/main.cpp)

