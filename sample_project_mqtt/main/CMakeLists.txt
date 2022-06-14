idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES  "AmazonRootCA1.pem"
                                    "certificate.pem.crt"
                                    "private.pem.key")
