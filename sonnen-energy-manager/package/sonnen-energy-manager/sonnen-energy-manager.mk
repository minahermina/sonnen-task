SONNEN_ENERGY_MANAGER_VERSION = 1.0
SONNEN_ENERGY_MANAGER_SITE = ./package/sonnen-energy-manager/src
SONNEN_ENERGY_MANAGER_SITE_METHOD = local

define SONNEN_ENERGY_MANAGER_BUILD_CMDS
    $(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D)
endef

define SONNEN_ENERGY_MANAGER_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 0755 $(@D)/sonnen-server $(TARGET_DIR)/usr/bin
    $(INSTALL) -D -m 0755 $(@D)/sonnen-client $(TARGET_DIR)/usr/bin
    $(INSTALL) -D -m 0644 $(@D)/sonnen-client.service $(TARGET_DIR)/etc/systemd/system/
    $(INSTALL) -D -m 0644 $(@D)/sonnen-server.service $(TARGET_DIR)/etc/systemd/system/
endef

$(eval $(generic-package))
