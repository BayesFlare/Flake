set (EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
add_executable (Flares Data.cpp CustomConfigFile.cpp FlareDistribution.cpp WaveDistribution.h FlareWave.cpp flake.cpp)
set_target_properties (Flares PROPERTIES OUTPUT_NAME flake)
target_link_libraries (Flares ${FLAKE_DEPS})
