cc_library(
    name = "roo_toolkit",
    visibility = ["//visibility:public"],
    srcs = glob(
        [
            "src/**/*.cpp",
            "src/**/*.h",
        ],
        exclude = ["test/**"],
    ),
    includes = [
        "src",
    ],
    defines = [
        "ROO_TESTING",
        "ARDUINO=10805",
    ],
    deps = [
        "//roo_testing/frameworks/arduino-esp32-2.0.4/libraries/Preferences",
        "//roo_testing/frameworks/arduino-esp32-2.0.4/libraries/WiFi",
        "//lib/roo_display",
        "//lib/roo_windows",
        "//lib/roo_logging",
        "//lib/roo_icons",
        "//lib/roo_prefs",
        "//lib/roo_time",
        "//lib/roo_scheduler",
        "//lib/roo_onewire_thermometers",
    ],
)

#cc_test(
#    name = "roo_time_test",
#    srcs = [
#        "test/roo_time_test.cpp",
#    ],
#    copts = ["-Iexternal/gtest/include"],
#    linkstatic = 1,
#    deps = [
#        "//lib/roo_time",
#        "@gtest//:gtest_main",
#    ],
#)
