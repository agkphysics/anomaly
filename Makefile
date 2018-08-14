CONTIKI_PROJECT = anomaly test
all: $(CONTIKI_PROJECT)
TARGET_LIBFILES += -lm  # For math library
CFLAGS += -I$(shell pwd)/include -std=gnu99  # Cannot be c99 due to 'asm' treated as function

ifeq ($(TARGET),native)
	DEFINES += NATIVE
endif

CONTIKI = ../contiki
#APPS = powertrace
CONTIKI_WITH_RIME = 1
include $(CONTIKI)/Makefile.include
