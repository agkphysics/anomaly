ifeq ($(DATASET),)
$(error Error: DATASET must be set)
endif

CONTIKI_PROJECT = anomaly test
all: init $(CONTIKI_PROJECT)
init:
	rm -f anomaly.$(TARGET)  # To fully rebuild
TARGET_LIBFILES += -lm  # For math library
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
CFLAGS += -I$(shell pwd)/include -I$(shell pwd)/include/$(DATASET) -std=gnu99  # Cannot be c99 due to 'asm' treated as function

DEFINES += DATASET_$(DATASET)

ifeq ($(TARGET),native)
	DEFINES += NATIVE
endif

CONTIKI = ../contiki
CONTIKI_WITH_RIME = 1
include $(CONTIKI)/Makefile.include
