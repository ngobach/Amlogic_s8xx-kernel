dtb-$(CONFIG_ARM) += meson8m2_n200_2G.dtb meson8m2_n200_1G.dtb \
		meson8m2_n200C_2G.dtb meson8m2_n200C_1G.dtb \
		meson8m2_n200_2G_LAN100.dtb meson8m2_n200C_2G_LAN100.dtb \
		meson8_k200b_1G_emmc_sdio.dtb meson8_k200b_2G_emmc_sdio.dtb \
		meson8_k200b_1G_emmc_sdhc.dtb meson8_k200b_2G_emmc_sdhc.dtb \
		meson8m2_n200_2G_M8SPlus.dtb \
		meson8m2_n200_2G_MXIIIPlus.dtb \
		meson8m2_n200_2G_SD1.dtb \
		meson8m2_n200_2G_volumio.dtb \
		meson8b_m201_1G.dtb

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
