# maag-wifi and websever component. Add c/cpp files here, also add required idf-components!

idf_component_register(SRCS "maag_wifi.cpp"
                            "maag_webserver.cpp" 
                            "maag_sntp.cpp"
                            PRIV_REQUIRES nvs_flash nghttp lwip
                            INCLUDE_DIRS "." "./include")