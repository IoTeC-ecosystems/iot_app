APPLICATION = iot_app
BOARD ?= native
RIOTBASE ?= $(CURDIR)/RIOT

USEMODULE += can_api

EXTERNAL_MODULE_DIRS += $(CURDIR)

DEVELHELP ?= 1
QUIET ?= 1

CFLAGS += -Wno-error=unused-variable

include $(RIOTBASE)/Makefile.include
