idf_component_register(SRCS 
                    "webserver.h"
                    "pin_diagrams.h" 
                    "stepper.h"
                    "sd_card.h"
                    "MS5837.c"
                    "webserver.c" 
                    "main_float.c" 
                    "stepper.c"
                    "sd_card.c"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES web/index.html
                    )