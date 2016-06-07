####################################################################################
#  MFGPS makefile
####################################################################################

TARGET	:= ft5336_download_lib.a

TARGETF := ft5336_download_lib

CROSS	= /opt/FriendlyARM/toolschain/4.5.1/bin/arm-linux-

CC		= $(CROSS)gcc -fno-common
LD		= $(CROSS)ld

FTSAR 	= ar
FTSARCFG = -rc

RM    = rm -f
PWD := $(shell pwd)

#kernel directory
#KDIR	=/home/Embedded/kernel/NFS2440_linux2.6.12
KDIR 	=/opt/Tiny210Kernel_New/linux-3.0.8/include/

SRCDIRS	:= . ./libs 

SRCEXTS	:= .c


CPPFLAGS :=

CFLAGS := -Wall -DLINUX -D_REENTRANT -O2 -DDEBUG

SOURCES = $(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*,$(SRCEXTS))))
OBJS    = $(foreach x,$(SRCEXTS),$(patsubst %$(x),%.o,$(filter %$(x),$(SOURCES))))
DEPS    = $(patsubst %.o,%.d,$(OBJS))

.PHONY : all objs clean cleanall rebuild

all : $(TARGET)


# Rules for creating the dependency files (.d).
%.d : %.c
	@cd $(<D);$(CC) -MM -MD $(CFLAGS) $(<F)
%.d : %.C
	@$(CC) -MM -MD $(CXXFLAGS) $<
%.d : %.cc
	@$(CC) -MM -MD $(CXXFLAGS) $<
%.d : %.cpp
	@$(CC) -MM -MD $(CXXFLAGS) $<
%.d : %.CPP
	@$(CC) -MM -MD $(CXXFLAGS) $<
%.d : %.c++
	@$(CC) -MM -MD $(CXXFLAGS) $<
%.d : %.cp
	@$(CC) -MM -MD $(CXXFLAGS) $<
%.d : %.cxx
	@$(CC) -MM -MD $(CXXFLAGS) $<

# Rules for producing the objects.
objs : $(OBJS)
%.o : %.c
	cd $(<D);$(CC) -c $(CPPFLAGS) $(CFLAGS) $(<F)
%.o : %.C
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<
%.o : %.cc
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<
%.o : %.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<
%.o : %.CPP
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<
%.o : %.c++
	$(CXX -c $(CPPFLAGS) $(CXXFLAGS) $<
%.o : %.cp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<
%.o : %.cxx
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

# Rules for producing the executable.
#----------------------------------------------
$(TARGET):$(OBJS)
ifeq ($(strip $(SRCEXTS)), .c)  # C file
	$(CC) -o $(TARGETF) $(OBJS) #$(LDFLAGS)
else                            # C++ file
	$(CXX) -o $(TARGETF) $(OBJS) #$(LDFLAGS)
endif

$(TARGET):$(OBJS)
	$(FTSAR) $(FTSARCFG) $(TARGET) $(OBJS)

-include $(DEPS)

rebuild: clean all

clean :
	@$(RM) $(OBJS) $(DEPS)	

#cleanall: clean
	@$(RM) $(TARGET)

