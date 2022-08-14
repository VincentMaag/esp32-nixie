# maag-wifi and websever component. Add c/cpp files here, also add required idf-components!

idf_component_register(SRCS "maag_gpio.cpp" 
                            PRIV_REQUIRES driver
                            INCLUDE_DIRS "." "./include")