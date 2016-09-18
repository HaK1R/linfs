CXXFLAGS += -std=c++14 -Wall -Wextra -Werror -fpic -fno-rtti -fno-exceptions
LDFLAGS += -shared

SRCS = file_impl.cc linfs.cc linfs_factory.cc path.cc reader_writer.cc section_allocator.cc
SRCS += $(addprefix entries/,directory_entry.cc file_entry.cc none_entry.cc)
SRCS += $(addprefix layout/,device_layout.cc)
SRCS += $(addprefix sections/,section.cc section_directory.cc section_file.cc)

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
