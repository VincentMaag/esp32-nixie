# include all c/cpp files in main-directory here:

idf_component_register(SRCS "main.cpp"
                            "nixie_webserver.cpp"
                            "nixie_testClass.cpp"
                            "maag_i2c_port.cpp"
                            "maag_i2c_device.cpp"
                            INCLUDE_DIRS ".")
