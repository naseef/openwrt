# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2025 OpenWrt.org
#

include $(TOPDIR)/rules.mk

PKG_NAME:=uboot-econet
PKG_VERSION:=2025.10
PKG_RELEASE:=1

PKG_LICENSE:=GPL-2.0+
PKG_LICENSE_FILES:=Licenses/README

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/uboot-econet
  SECTION:=boot
  CATEGORY:=Boot Loaders
  TITLE:=U-Boot for EcoNet EN751221
  DEPENDS:=@TARGET_econet
  URL:=https://www.denx.de/wiki/U-Boot
endef

define Package/uboot-econet/description
  U-Boot bootloader for EcoNet EN751221 SoC.
  Provides serial console, network boot (TFTP), and SPI-NAND support.
  Load address: 0x80020000 (compatible with factory bootloader XMODEM)
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CROSS_COMPILE=$(TARGET_CROSS) \
		econet_en751221_defconfig
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CROSS_COMPILE=$(TARGET_CROSS) \
		HOSTCC="$(HOSTCC)" \
		HOSTCFLAGS="$(HOST_CFLAGS) $(HOST_CPPFLAGS)" \
		HOSTLDFLAGS="$(HOST_LDFLAGS)"
endef

define Package/uboot-econet/install
	$(INSTALL_DIR) $(1)/
	$(CP) $(PKG_BUILD_DIR)/u-boot.bin $(1)/uboot-econet.bin
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/u-boot.bin $(STAGING_DIR_IMAGE)/uboot-econet.bin
endef

$(eval $(call BuildPackage,uboot-econet))
