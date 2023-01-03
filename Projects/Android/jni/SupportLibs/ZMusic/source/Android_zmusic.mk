
LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)


LOCAL_MODULE    := zmusic

LOCAL_CPPFLAGS := -DHAVE_SNDFILE -DHAVE_FLUIDSYNTH -DHAVE_MPG123 -fexceptions -std=c++11 -Dstricmp=strcasecmp -Dstrnicmp=strncasecmp  -fsigned-char

LOCAL_C_INCLUDES :=  \
$(LOCAL_PATH) \
$(LOCAL_PATH)/zmusic \
$(LOCAL_PATH)/../include \
$(LOCAL_PATH)/../thirdparty/dumb/include \
$(LOCAL_PATH)/../thirdparty/adlmidi \
$(LOCAL_PATH)/../thirdparty/opnmidi \
$(LOCAL_PATH)/../thirdparty/oplsynth/ \
$(LOCAL_PATH)/../thirdparty/oplsynth/oplsynth \
$(LOCAL_PATH)/../thirdparty/timidity \
$(LOCAL_PATH)/../thirdparty/timidityplus \
$(LOCAL_PATH)/../thirdparty/wildmidi \
$(LOCAL_PATH)/../thirdparty/game-music-emu \
$(LOCAL_PATH)/../../fluidsynth-lite/include \
$(LOCAL_PATH)/../../libsndfile-android/jni/ \
$(LOCAL_PATH)/../../libmpg123 \


LOCAL_SRC_FILES =  	\
	loader/i_module.cpp \
	mididevices/music_base_mididevice.cpp \
	mididevices/music_adlmidi_mididevice.cpp \
	mididevices/music_opl_mididevice.cpp \
	mididevices/music_opnmidi_mididevice.cpp \
	mididevices/music_timiditypp_mididevice.cpp \
	mididevices/music_fluidsynth_mididevice.cpp \
	mididevices/music_softsynth_mididevice.cpp \
	mididevices/music_timidity_mididevice.cpp \
	mididevices/music_wildmidi_mididevice.cpp \
	mididevices/music_wavewriter_mididevice.cpp \
	midisources/midisource.cpp \
	midisources/midisource_mids.cpp \
	midisources/midisource_mus.cpp \
	midisources/midisource_smf.cpp \
	midisources/midisource_hmi.cpp \
	midisources/midisource_xmi.cpp \
	streamsources/music_dumb.cpp \
	streamsources/music_gme.cpp \
	streamsources/music_libsndfile.cpp \
	streamsources/music_opl.cpp \
	streamsources/music_xa.cpp \
	musicformats/music_stream.cpp \
	musicformats/music_midi.cpp \
	musicformats/music_cd.cpp \
	decoder/sounddecoder.cpp \
	decoder/sndfile_decoder.cpp \
	decoder/mpg123_decoder.cpp \
	zmusic/configuration.cpp \
	zmusic/zmusic.cpp \
	zmusic/critsec.cpp \
	loader/test.c

LOCAL_STATIC_LIBRARIES := zlib_zm oplsynth_zm  opnmidi_zm timidity_zm timidityplus_zm wildmidi_zm adlmidi_zm gme_zm dumb_zm  sndfile mpg123 fluidsynth-static


include $(BUILD_SHARED_LIBRARY)








