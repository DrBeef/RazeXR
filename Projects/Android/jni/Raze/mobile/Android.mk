
GZDOOM_TOP_PATH := $(call my-dir)/../

include $(GZDOOM_TOP_PATH)/mobile/Android_lzma.mk
include $(GZDOOM_TOP_PATH)/mobile/Android_zlib.mk
include $(GZDOOM_TOP_PATH)/mobile/Android_gdtoa.mk
include $(GZDOOM_TOP_PATH)/mobile/Android_bzip2.mk
include $(GZDOOM_TOP_PATH)/mobile/Android_vpx.mk
include $(GZDOOM_TOP_PATH)/mobile/Android_tess.mk


include $(GZDOOM_TOP_PATH)/mobile/Android_src.mk




