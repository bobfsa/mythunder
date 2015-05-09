                                                                                      
# =====================================================================================#
# build enviroment include
# =====================================================================================#
#CROSS_TOOL_DIR = /home/work/licett/project/imx6/firmware/i_MX6QSABRELite/toolchain
#CROSS_COMPILE = $(CROSS_TOOL_DIR)/bin/
#LINUXLIBS_INSTALL_DIR =/home/work/licett/project/imx6/firmware/i_MX6QSABRELite/toolchain

CROSS_TOOL_DIR = /home/bobfsa/fsa_work/code/imx6_3035sdk/i_MX6QSABRELite/toolchain
#CROSS_TOOL_DIR = /home/work/licett/toolchain_newest/toolchain
CROSS_COMPILE = $(CROSS_TOOL_DIR)/bin/

LINUXLIBS_INSTALL_DIR = $(CROSS_TOOL_DIR)


.PHONY: test  

# =====================================================================================#
# system include 
# =====================================================================================#
SYS_INCLUDE 	= \
		 -I$(LINUXLIBS_INSTALL_DIR)/include 

# =====================================================================================#
# system lib
# =====================================================================================#
SYS_LIB 	= 

# =====================================================================================#
# local include
# =====================================================================================#
LOCAL_INCLUDE 	= 	\
			-I$(LINUXLIBS_INSTALL_DIR)/include

# =====================================================================================#
# local lib
# =====================================================================================#
LOCALE_LIB 	= 	\
			-L$(LINUXLIBS_INSTALL_DIR)/lib -lpthread -levent_core -levent_extra -levent_pthreads

# =====================================================================================#
# system toolchain
# =====================================================================================#
GCC_C_FLAGS 	= $(C_FLAGS) -Wall  -O2 $(SYS_INCLUDE) $(LOCAL_INCLUDE)
GCC_COMPILE.c 	= $(CROSS_COMPILE)armv7l-timesys-linux-gnueabi-gcc 

export $(GCC_C_FLAGS) $(GCC_COMPILE.c)

GCC_LD_FLAGS	= $(LD_FLAGS) $(SYS_LIB)

GCC_CXX_FLAGS 	= $(C_FLAGS) -Wall -O2 $(SYS_INCLUDE) $(LOCAL_INCLUDE) -DDSPGUN
#GCC_CXX_FLAGS 	= $(C_FLAGS) -Wall -Werror $(SYS_INCLUDE)
GCC_COMPILE.cxx	= $(CROSS_COMPILE)armv7l-timesys-linux-gnueabi-g++ -c $(GCC_CXX_FLAGS) $(GCC_CPP_FLAGS) -o $@ $<
GCC_LINK.cxx	= $(CROSS_COMPILE)armv7l-timesys-linux-gnueabi-g++  -o $@ $^ $(GCC_LD_FLAGS) $(LOCALE_LIB)

GCC_STRIP 	= $(CROSS_COMPILE)armv7l-timesys-linux-gnueabi-strip

# =====================================================================================#
# local compile macroes
# =====================================================================================#
# aisound module build include
SUPPORT_AISOUND = no

# aitalk module build include
SUPPORT_AITALK 	= no

# ainr module build include
SUPPORT_AINR 	= no

# libiconv module build include
SUPPORT_ICONV  	= no

# =====================================================================================#
# global compile flags
# =====================================================================================#
# gcc flags user define
#GCC_CXX_FLAGS 	+= -DCONFIG_AUDIO_DUMP
# set alsa soc working in nonblocking mode
GCC_CXX_FLAGS 	+= -DCONFIG_NONBLOCKING_MODE -DDSPGUN
# set alsa soc working in no interleaved access mode


# =====================================================================================#
# base source files
# =====================================================================================#
HEADERS		= $(wildcard *.h)
#SOURCES 	= $(wildcard *.cpp)
SOURCES 	= main.cpp 	\
			serialport.cpp\
			thread.cpp \
			filesysmgr.cpp \
			eimdata.cpp \
			util.cpp \
			gunparse.cpp \
			datasocket.cpp


# =====================================================================================#
# base library
# =====================================================================================#
LIBS 	=

# =====================================================================================#
# libiconv module
# =====================================================================================#
ifeq ($(SUPPORT_ICONV), yes)
SOURCES 	+= \
	aisound.cpp
GCC_CXX_FLAGS 	+= -DCONFIG_CODEPAGE_CONVERT
LOCALE_LIB 	+= \
	./libiconv/lib/.libs/libiconv.a
LIBS 		+= iconv
endif

# =====================================================================================#
# aisound module
# =====================================================================================#
ifeq ($(SUPPORT_AISOUND), yes)
SOURCES 	+= \
	aisound.cpp
GCC_CXX_FLAGS 	+= -DSUPPORT_AISOUND
LOCAL_INCLUDE 	+= \
	-I ./Aisound/Inc
LOCALE_LIB 	+= \
	./Aisound/Lib/AiSound5.a
endif



# =====================================================================================#
# object files 
# =====================================================================================#
OBJFILES	= $(SOURCES:%.c=%.o)

# =====================================================================================#
# main target
# =====================================================================================#

TARGET 	= newgun



all: $(LIBS) $(TARGET)
	$(GCC_STRIP) $(TARGET)

$(TARGET): $(OBJFILES)
	@echo Linking $@ from $^..
	$(GCC_LINK.cxx)

# =====================================================================================#
# iconv lib compile
# =====================================================================================#
iconv:
	@(cd libiconv; 	\
	if [ ! -e  .configured ]; then 	\
		./configure 	\
			CC="$(GCC_COMPILE.c)" 	\
			CFLAGS="$(GCC_C_FLAGS)" 	\
			LDFLAGS="$(GCC_LD_FLAGS)" 	\
			--build=i686-pc-linux-gnu 	\
			--host=arm-none-linux-gnueabi 	\
			--target=arm-none-linux-gnueabi 	\
			--enable-shared=no	\
			--enable-static=yes	\
			--disable-rpath 	\
			--disable-nls 	\
			--prefix=$(LINUXLIBS_INSTALL_DIR); 	\
		touch .configured; 	\
	fi; 	\
	if [ ! -e  .compiled ]; then 	\
		make; 	\
		touch .compiled; 	\
	fi; 	\
	cd -)

# =====================================================================================#
# compile rule
# =====================================================================================#
.cpp.o:
	@echo Compiling $@ from $<..
	$(GCC_COMPILE.cxx)

# =====================================================================================#
# copy to filesystem
# =====================================================================================#
copy:
	@(if [ ! -d  $(EXEC_DIR) ]; then 	\
		mkdir -p $(EXEC_DIR); 	\
	fi)
	cp -rf $(TARGET) $(EXEC_DIR)

# =====================================================================================#
# clean target
# =====================================================================================#
clean:
	rm -rf $(TARGET)
	rm -rf *.o *~
#	$(MAKE) -C test clean

distclean:
	$(MAKE) -C speex clean
	$(MAKE) -C libiconv clean
	$(MAKE) -C libiconv distclean
	rm -rf libiconv/.configured
	rm -rf libiconv/.compiled
	rm -rf speex/.compiled

# =====================================================================================#
# test target
# =====================================================================================#
#test:
#	$(MAKE) -C test

# =====================================================================================#
# end.
# =====================================================================================#
