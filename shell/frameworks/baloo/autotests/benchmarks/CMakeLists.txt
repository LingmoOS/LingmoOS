# Benchmarks, compiled, but not run automatically with ctest

add_executable(positioncodecbenchmark positioncodecbenchmark.cpp)
target_link_libraries(positioncodecbenchmark Qt6::Test KF6::BalooCodecs)
