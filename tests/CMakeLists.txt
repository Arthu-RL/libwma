# Deliberately framework-free: these assert on pure functions, and a
# dependency-free test binary keeps `ctest` runnable in the same containers
# that build the library, with nothing to fetch or vendor.
add_executable(wma_test_utf8 test_utf8.cpp)
target_link_libraries(wma_test_utf8 PRIVATE wma)
add_test(NAME wma_test_utf8 COMMAND wma_test_utf8)

# Threaded, unlike the decoder tests: MixCallbackSlot exists to make
# setMixCallback() safe on a running device, and only two threads can show that.
# Worth running under -fsanitize=thread, which is what turns a missed
# synchronization back into a hard failure rather than a rare glitch.
add_executable(wma_test_mix_callback_slot test_mix_callback_slot.cpp)
target_link_libraries(wma_test_mix_callback_slot PRIVATE wma)
add_test(NAME wma_test_mix_callback_slot COMMAND wma_test_mix_callback_slot)
