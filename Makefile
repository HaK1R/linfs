CXX = g++
CPPFLAGS = -I. -I./include
CXXFLAGS = -std=c++14 -Wall -Wextra -Werror -fpic -fno-rtti -fno-exceptions
LDFLAGS = -shared

SRCS =
SRCS += $(addprefix lib/entries/,directory_entry.cc file_entry.cc none_entry.cc)
SRCS += $(addprefix lib/layout/,device_layout.cc)
SRCS += $(addprefix lib/sections/,section.cc section_directory.cc section_file.cc)
SRCS += $(addprefix lib/,file_impl.cc linfs.cc linfs_factory.cc path.cc reader_writer.cc section_allocator.cc)

OBJS = $(SRCS:.cc=.o)

LIBNAME = liblinfs.so

.PHONY: depend clean

all: $(LIBNAME)

$(LIBNAME): $(OBJS)
	$(CXX) $(LDFLAGS) -o $(LIBNAME) $(OBJS)

%.o : %.cc %.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(LIBNAME)

#depend: $(SRCS)
#	makedepend $(CPPFLAGS) -I/usr/include/c++/5 $^

# DO NOT DELETE THIS LINE -- make depend needs it
