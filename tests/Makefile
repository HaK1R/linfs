CXXFLAGS += -std=c++11 -Wall -Wextra -Werror
LDFLAGS += -L$(SRC_DIR)/lib -Wl,-rpath="$(SRC_DIR)/lib" -lboost_system -lboost_filesystem -lboost_unit_test_framework -llinfs

SRCS = directory_operations.cc file_operations.cc filesystem_operations.cc run_tests.cc

OBJS = $(SRCS:.cc=.o)

EXENAME = run_tests

.PHONY: build clean

build: $(EXENAME)

$(EXENAME): $(OBJS)
	$(CXX) -o $(EXENAME) $(OBJS) $(LDFLAGS)

%.o : %.cc *.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXENAME)
