
LOCAL_PATH := $(call my-dir)/../thirdparty/oplsynth


include $(CLEAR_VARS)


LOCAL_MODULE    := oplsynth_zm

LOCAL_CFLAGS := -frtti -Wall -Wextra -Wno-unused-parameter -fomit-frame-pointer -fsigned-char

LOCAL_LDLIBS += -llog

LOCAL_C_INCLUDES :=  $(LOCAL_PATH)/oplsynth


LOCAL_SRC_FILES =  	\
	fmopl.cpp \
	musicblock.cpp \
	nukedopl3.cpp \
	opl_mus_player.cpp \
	OPL3.cpp \
	oplio.cpp \
	dosbox/opl.cpp \

include $(BUILD_STATIC_LIBRARY)








