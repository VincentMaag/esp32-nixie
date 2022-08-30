# maag-wifi and websever component. Add c/cpp files here, also add required idf-components!

idf_component_register(SRCS "maag_spi_host.cpp" 
                            "maag_spi_device.cpp"
                            PRIV_REQUIRES driver maag_lib
                            INCLUDE_DIRS "." "./include")