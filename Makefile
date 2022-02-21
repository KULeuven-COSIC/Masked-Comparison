# PLATFORM = {ARM, host}
PLATFORM=host

# CFLAGS += {-DKYBER, -DSABER}
CFLAGS += -DKYBER

# CFLAGS += {-DNSHARES>=2}
CFLAGS += -DNSHARES=4

# CFLAGS += {-NTESTS>=1}
CFLAGS += -DNTESTS=10

# CFLAGS += {-DPROFILE_TOP_CYCLES, -DPROFILE_TOP_RAND, -DPROFILE_STEP_CYCLES, -DPROFILE_STEP_RAND}
CFLAGS += -DPROFILE_STEP_RAND

PROJECT = MaskedComparison
BUILD_DIR = bin
SHARED_DIR = common
CFILES += $(wildcard src/*.c) common/randombytes.c main.c
HEADERS = $(wildcard src/*.h)
INCLUDES += $(patsubst %,-I%, . $(SHARED_DIR) src)

ifeq ($(PLATFORM), host)

    OPT = -O0 -g -DDEBUG

    include mk/host.mk

else ifeq ($(PLATFORM), ARM)

    OPT = -O3
    CFILES += common/hal.c 
    DEVICE = stm32f407vgt6u

    # You shouldn't have to edit anything below here.
    VPATH += $(SHARED_DIR)
    OPENCM3_DIR=libopencm3

    include $(OPENCM3_DIR)/mk/genlink-config.mk
    include mk/rules.mk
    include $(OPENCM3_DIR)/mk/genlink-rules.mk

endif

	



