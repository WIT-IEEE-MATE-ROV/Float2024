idf_component_register(SRCS 
                    "MS5837.c" 
                    "pin_diagrams.c" 
                    "main_float.c" 
                    "webserver.c" 
                    "webserver.h"
                    "stepper.h"
                    "stepper.c"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES web/index.html
                    )
