TRX_ENDIAN := le

define Device/en7528_generic
  DEVICE_VENDOR := EN7528
  DEVICE_MODEL := Generic
  DEVICE_DTS := en7528_generic
endef
TARGET_DEVICES += en7528_generic

define Device/dasan_h660gm-a
  DEVICE_VENDOR := DASAN
  DEVICE_MODEL := H660GM-A
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt7615e kmod-mt7663-firmware-ap
  TRX_MODEL := Dewberry
  KERNEL_SIZE := 4096k
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  UBINIZE_OPTS := -E 5
  IMAGES := tclinux.trx sysupgrade.bin
  IMAGE/tclinux.trx := append-kernel | lzma | tclinux-trx-kernel | \
	pad-to $$(KERNEL_SIZE) | append-ubi
  IMAGE/sysupgrade.bin := append-kernel | lzma | tclinux-trx-kernel | \
	pad-to $$(KERNEL_SIZE) | tclinux-sysupgrade-tar | append-metadata
endef

define Device/dasan_h660gm-a-airtel
  $(Device/dasan_h660gm-a)
  DEVICE_VARIANT := Airtel
  DEVICE_DTS := en7528_dasan_h660gm-a-airtel
endef
TARGET_DEVICES += dasan_h660gm-a-airtel

define Device/dasan_h660gm-a-generic
  $(Device/dasan_h660gm-a)
  DEVICE_VARIANT := Generic
  DEVICE_DTS := en7528_dasan_h660gm-a-generic
endef
TARGET_DEVICES += dasan_h660gm-a-generic
