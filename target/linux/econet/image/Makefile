include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

define Target/Description
	Build firmware images for EcoNet MIPS based boards.
endef

# tclinux-trx is the default format used in the SDK
define Build/tclinux-trx
  ./tclinux-trx.sh --kernel $@ --rootfs $(IMAGE_ROOTFS) \
    --version $(VERSION_DIST)-$(REVISION) --endian $(TRX_ENDIAN) \
    $(if $(TRX_MODEL),--model $(TRX_MODEL)) > $@.new
	mv $@.new $@
endef

# TRX header wrapping kernel only (no rootfs)
define Build/tclinux-trx-kernel
	./tclinux-trx.sh --kernel $@ \
	  --version $(VERSION_DIST)-$(REVISION) --endian $(TRX_ENDIAN) \
	  $(if $(TRX_MODEL),--model $(TRX_MODEL)) > $@.new
	mv $@.new $@
endef

# Create a sysupgrade-tar with TRX-wrapped kernel for nand_do_upgrade
define Build/tclinux-sysupgrade-tar
	dd if=$@ of=$@.trxkernel bs=$(KERNEL_SIZE) count=1 conv=sync 2>/dev/null
	sh $(TOPDIR)/scripts/sysupgrade-tar.sh \
	  --board $(if $(BOARD_NAME),$(BOARD_NAME),$(DEVICE_NAME)) \
	  --kernel $@.trxkernel \
	  --rootfs $(IMAGE_ROOTFS) \
	  $@
	rm -f $@.trxkernel
endef

# tclinux bootloader requires LZMA, BUT only provides 7.5MB of space
# to decompress into. So we use vmlinuz and decompress twice.
define Device/Default
  DEVICE_DTS_DIR := ../dts
  KERNEL_SIZE := 7480k
  KERNEL_NAME := vmlinuz.bin
  KERNEL_LOADADDR := 0x80020000
  KERNEL := kernel-bin | append-dtb
endef

include $(SUBTARGET).mk

$(eval $(call BuildImage))
