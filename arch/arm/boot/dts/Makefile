dtb-$(CONFIG_ARCH_MESON8B) += meson8b_m201_1G_old.dtb meson8b_m201C_512M_old.dtb meson8b_m201.dtb \
				meson8b_mxq.dtb meson8b_hd180.dtb meson8b_mk808.dtb \
				meson8b_i5.dtb meson8b_i5_console.dtb \
				meson8b_m100_1G.dtb meson8b_m101_512M.dtb \
				meson8b_m102_1G.dtb meson8b_m200_1G.dtb \
				meson8b_m201C_512M.dtb meson8b_m201_1G.dtb \
				meson8b_m202_512M.dtb meson8b_m203a.dtb \
				meson8b_m203b.dtb

DTB_NAMES := $(subst $\",,$(CONFIG_BUILD_ARM_APPENDED_DTB_IMAGE_NAMES))
ifneq ($(DTB_NAMES),)
DTB_LIST := $(addsuffix .dtb,$(DTB_NAMES))
else
DTB_LIST := $(dtb-y)
endif

targets += dtbs
targets += $(DTB_LIST)

# *.dtb used to be generated in the directory above. Clean out the
# old build results so people don't accidentally use them.
dtbs: $(addprefix $(obj)/, $(DTB_LIST))
	$(Q)rm -f $(obj)/../*.dtb

clean-files := *.dtb
