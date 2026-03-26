PART_NAME=firmware
REQUIRE_IMAGE_METADATA=1

platform_check_image() {
	[ "$#" -gt 1 ] && return 1
	return 0
}

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	dasan,h660gm-a-airtel|\
	dasan,h660gm-a-generic)
		nand_do_upgrade "$1"
		;;
	esac
}
