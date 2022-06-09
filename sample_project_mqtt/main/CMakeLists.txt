idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES  "cerf/AmazonRootCA1.pem"
                                    "cerf/certificate.pem.crt"
                                    "cerf/private.pem.key")
