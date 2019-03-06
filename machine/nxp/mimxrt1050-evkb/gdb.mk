TYPE := custom

.PHONY: gdb
gdb:
	BOARD=$(CONFIG_BOARD) \
	BUILDDIR=$(CONFIG_BUILDDIR) \
	TOOLSDIR=$(CONFIG_APEXDIR)/machine/nxp/mimxrt1050-evkb/tools \
	arm-none-eabi-gdb -x $(CONFIG_APEXDIR)/machine/nxp/mimxrt1050-evkb/gdbinit
