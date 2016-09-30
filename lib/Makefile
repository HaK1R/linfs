CXXFLAGS += -std=c++14 -O2 -Wall -Wextra -Werror -fpic -fno-rtti
LDFLAGS += -shared

# Define SYSTEM_ORDER.  Possible variables are: LittleEndian, BigEndian.
# My computer has little endian architecture
CPPFLAGS += -DSYSTEM_ORDER=LittleEndian

# Define DEVICE_ORDER.  Possible variables are: LittleEndian, BigEndian.
# We use little endian by default because it's the most popular architecture in the world.
# !!! Don't modify it unless you are sure what you are doing !!!
CPPFLAGS += -DDEVICE_ORDER=LittleEndian

SRCS = entry_cache.cc file_impl.cc linfs.cc linfs_factory.cc section_allocator.cc
SRCS += $(addprefix entries/,directory_entry.cc entry.cc file_entry.cc none_entry.cc symlink_entry.cc)
SRCS += $(addprefix layout/,device_layout.cc)
SRCS += $(addprefix sections/,section.cc section_directory.cc section_file.cc)
SRCS += $(addprefix utils/,exception_handler.cc format_exception.cc path.cc reader_writer.cc)

OBJS = $(SRCS:.cc=.o)

LIBNAME = liblinfs.so

.PHONY: build clean depend

build: $(LIBNAME)

$(LIBNAME): $(OBJS)
	$(CXX) $(LDFLAGS) -o $(LIBNAME) $(OBJS)

%.o : %.cc %.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(LIBNAME)

#depend: $(SRCS)
#	makedepend $(CPPFLAGS) -I/usr/include/c++/5 $^

# DO NOT DELETE THIS LINE -- make depend needs it
