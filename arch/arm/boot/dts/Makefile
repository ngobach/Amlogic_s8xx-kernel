ifeq ($(CONFIG_OF),y)

dtb-$(CONFIG_MACH_MESON6) += \
	meson6-atv1200.dtb
dtb-$(CONFIG_MACH_MESON8) += \
	meson8-minix-neo-x8.dtb
dtb-$(CONFIG_MACH_MESON8B) += \
	meson8b-mxq.dtb \
	meson8b-odroidc1.dtb
dtb-$(CONFIG_MACH_MESON8M2) += \
	meson8m2-m8s.dtb
endif

dtstree		:= $(srctree)/$(src)
dtb-$(CONFIG_OF_ALL_DTBS) := $(patsubst $(dtstree)/%.dts,%.dtb, $(wildcard $(dtstree)/*.dts))

always		:= $(dtb-y)
clean-files	:= *.dtb
