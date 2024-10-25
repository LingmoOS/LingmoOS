include $(sort $(wildcard $(LINGMO_EXTERNAL_GIT_REFS_PATH)/package/*/*.mk))

# Get the git server port number from the test infra
GITREMOTE_PORT_NUMBER ?= 9418
