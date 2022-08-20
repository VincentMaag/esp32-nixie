# include all c/cpp files in main-directory here:

idf_component_register(SRCS "main.cpp"
                            "nixie_webserver.cpp"
                            "nixie_testClass.cpp"
                            "nixie_ds3231.cpp"
                            INCLUDE_DIRS ".")
