# include all c/cpp files in main-directory here:

idf_component_register(SRCS "main.cpp"
                            "nixie_webserver.cpp"
                            "nixie_testClass.cpp"
                            "nixie_ds3231.cpp"
                            "nixie_time.cpp"
                            "nixie_hv5622.cpp"
                            INCLUDE_DIRS ".")
