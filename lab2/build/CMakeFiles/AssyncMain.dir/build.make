# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/pasha/Study/OS/lab2

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/pasha/Study/OS/lab2/build

# Include any dependencies generated for this target.
include CMakeFiles/AssyncMain.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/AssyncMain.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/AssyncMain.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/AssyncMain.dir/flags.make

CMakeFiles/AssyncMain.dir/src/AssyncMain.cpp.o: CMakeFiles/AssyncMain.dir/flags.make
CMakeFiles/AssyncMain.dir/src/AssyncMain.cpp.o: ../src/AssyncMain.cpp
CMakeFiles/AssyncMain.dir/src/AssyncMain.cpp.o: CMakeFiles/AssyncMain.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pasha/Study/OS/lab2/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/AssyncMain.dir/src/AssyncMain.cpp.o"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/AssyncMain.dir/src/AssyncMain.cpp.o -MF CMakeFiles/AssyncMain.dir/src/AssyncMain.cpp.o.d -o CMakeFiles/AssyncMain.dir/src/AssyncMain.cpp.o -c /home/pasha/Study/OS/lab2/src/AssyncMain.cpp

CMakeFiles/AssyncMain.dir/src/AssyncMain.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/AssyncMain.dir/src/AssyncMain.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pasha/Study/OS/lab2/src/AssyncMain.cpp > CMakeFiles/AssyncMain.dir/src/AssyncMain.cpp.i

CMakeFiles/AssyncMain.dir/src/AssyncMain.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/AssyncMain.dir/src/AssyncMain.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pasha/Study/OS/lab2/src/AssyncMain.cpp -o CMakeFiles/AssyncMain.dir/src/AssyncMain.cpp.s

# Object files for target AssyncMain
AssyncMain_OBJECTS = \
"CMakeFiles/AssyncMain.dir/src/AssyncMain.cpp.o"

# External object files for target AssyncMain
AssyncMain_EXTERNAL_OBJECTS =

../bin/AssyncMain: CMakeFiles/AssyncMain.dir/src/AssyncMain.cpp.o
../bin/AssyncMain: CMakeFiles/AssyncMain.dir/build.make
../bin/AssyncMain: CMakeFiles/AssyncMain.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/pasha/Study/OS/lab2/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/AssyncMain"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/AssyncMain.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/AssyncMain.dir/build: ../bin/AssyncMain
.PHONY : CMakeFiles/AssyncMain.dir/build

CMakeFiles/AssyncMain.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/AssyncMain.dir/cmake_clean.cmake
.PHONY : CMakeFiles/AssyncMain.dir/clean

CMakeFiles/AssyncMain.dir/depend:
	cd /home/pasha/Study/OS/lab2/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pasha/Study/OS/lab2 /home/pasha/Study/OS/lab2 /home/pasha/Study/OS/lab2/build /home/pasha/Study/OS/lab2/build /home/pasha/Study/OS/lab2/build/CMakeFiles/AssyncMain.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/AssyncMain.dir/depend
