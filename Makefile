C_SOURCE_FILES += $(wildcard ./src/*.c)
C_INCLUDES += ./src ./include

ifeq ($(NO_FLOAT), 1)
C_FLAGS += -DNO_FLOAT=1
endif

include ./cc-with-test.mk
