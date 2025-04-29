add_executable(
    mp_os_arthmtc_frctn_tests_frctn
    fraction_tests.cpp)

target_link_libraries(
    mp_os_arthmtc_frctn_tests_frctn
    PRIVATE
    gtest_main)

target_link_libraries(
    mp_os_arthmtc_frctn_tests_frctn
    PRIVATE
    mp_os_lggr_clnt_lggr)

target_link_libraries(
    mp_os_arthmtc_frctn_tests_frctn
    PRIVATE
    mp_os_arthmtc_frctn)

target_link_libraries(
    mp_os_arthmtc_frctn_tests_frctn
    PRIVATE
    mp_os_arthmtc_bg_intgr)