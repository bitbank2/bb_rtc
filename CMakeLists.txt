idf_component_register(
    SRCS "src/bb_rtc.cpp"
    INCLUDE_DIRS "src"

    REQUIRES driver esp_timer
)

