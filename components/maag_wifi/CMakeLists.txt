# maag-wifi and websever component. Add c/cpp files here, also add required idf-components!

idf_component_register(SRCS "maag_webserver.cpp" 
                            "maag_wifi.cpp"
                            PRIV_REQUIRES nvs_flash nghttp
                            INCLUDE_DIRS "." "./include")