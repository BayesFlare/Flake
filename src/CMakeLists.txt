set (EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
add_executable (src Data.cpp CustomConfigFile.cpp FlareDistribution.cpp WaveDistribution.cpp ImpulseDistribution.cpp ChangepointDistribution.cpp FlareWave.cpp flake.cpp)
set_target_properties (src PROPERTIES OUTPUT_NAME flake)
target_link_libraries (src ${FLAKE_DEPS})
