cc_library(
    name = "roo_toolkit",
    visibility = ["//visibility:public"],
    srcs = glob(
        [
            "**/*.cpp",
            "**/*.h",
        ],
        exclude = ["test/**"],
    ),
    includes = [
        ".",
    ],
    defines = [
        "ROO_TESTING",
        "ARDUINO=10805",
    ],
    deps = [
        "//roo_testing:arduino",
        "//lib/roo_display",
        "//lib/roo_windows",
        "//lib/roo_glog",
        "//lib/roo_material_icons",
        "//lib/roo_time",
        "//lib/roo_scheduler",
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
