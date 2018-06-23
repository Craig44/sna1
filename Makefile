all: sna1.exe

clean:
	rm -f *.debug *.exe *.o
#############################################################
# Operating system

UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
	OS := linux
endif
ifeq ($(UNAME), WindowsNT)
	OS := win
endif

$(warning If you have issues building on windows see 'WindowNotesForBuilding.txt' in the scripts folder for help)



#############################################################
# Requirements

BOOST_VERSION := 1_62_0

requires/boost_$(BOOST_VERSION).tar.bz2:
	@mkdir -p requires
	wget --no-check-certificate -O $@ http://prdownloads.sourceforge.net/boost/boost_$(BOOST_VERSION).tar.bz2

requires/boost: requires/boost_$(BOOST_VERSION).tar.bz2
	tar --bzip2 -xf $< -C requires
	mv requires/boost_$(BOOST_VERSION) requires/boost
	touch $@

BOOST_BOOTSTRAP_FLAGS := --with-libraries=filesystem,test
BOOST_B2_FLAGS := -d0 --prefix=. link=static install 
ifeq ($(OS), linux)
	BOOST_B2_FLAGS += cxxflags=-fPIC
endif
ifeq ($(OS), win)
	BOOST_B2_FLAGS += --layout=system release toolset=gcc
endif

ifeq ($(OS), linux)
requires/boost/lib: requires/boost
	cd $< ;./bootstrap.sh $(BOOST_BOOTSTRAP_FLAGS)
	sed -i "s/mingw/gcc/g" $</project-config.jam
	cd $< ; ./b2 $(BOOST_B2_FLAGS)
	touch $@
endif
ifeq ($(OS), win)
requires/boost/lib: requires/boost
	$(warning cd =  $< )
	cd $< ; cmd /C bootstrap.bat gcc $(BOOST_BOOTSTRAP_FLAGS)
	$(warning The file 'project-config.jam' may need a more comprehensive change, but for now this is all I did, see the file building dependencies on windows.txt in the scripts folder)
	sed -i "s/msvc/gcc/g" $</project-config.jam
	cd $< ; ./b2 $(BOOST_B2_FLAGS)
	##./b2 -d0 --prefix=. link=static install cxxflags=-fPIC runtime-link=static stage
	touch $@
endif


STENCILA_VERSION := 0.2

requires/stencila-cpp-$(STENCILA_VERSION).zip:
	@mkdir -p requires
	wget --no-check-certificate -O $@ https://github.com/stencila/cpp/archive/$(STENCILA_VERSION).zip

requires/stencila: requires/stencila-cpp-$(STENCILA_VERSION).zip
	rm -rf requires/stencila
	unzip -o $< -d requires
	mv requires/cpp-$(STENCILA_VERSION)/ requires/stencila/
	touch $@


requires/r-packages.installed:
	Rscript -e "install.packages(c('tidyr','dplyr','ggplot2', 'knitr'), repos='http://cran.us.r-project.org')"
	touch $@
	
requires: requires/boost/lib requires/stencila requires/r-packages.installed


#############################################################
# Executables
$(info entering executable)
# Define compile options and required libraries
CXX_FLAGS := -std=c++11 -Wall -Wno-unused-function -Wno-unused-local-typedefs -Wno-unused-variable -pthread
INC_DIRS := -I. -Irequires/boost -Irequires/stencila 

ifeq ($(OS), linux)
	# Find all .hpp and .cpp files (to save time don't recurse into subdirectories)
	SRC := $(shell find src -maxdepth 3 -mindepth 1 -name "*.h")
	SRC += $(shell find src -maxdepth 3 -mindepth 2 -name "*.cpp") # No source files in upper layer apart from the main();
	main := $(shell find src -maxdepth 1 -name "*.cpp")
	sub_dir := $(shell find src -type d)
	INC_DIRS += $(addprefix  -I,  $(sub_dir))
	LIB_DIRS := -Lrequires/boost/lib
	LIBS := -lboost_system -lboost_filesystem
	TEST_LIB := -lboost_unit_test_framework
endif
ifeq ($(OS), win)
	# Find all .hpp and .cpp files (to save time don't recurse into subdirectories)
	SRC := $(shell find_linux src -maxdepth 3 -mindepth 1 -name "*.h")
	SRC += $(shell find_linux src -maxdepth 3 -mindepth 2 -name "*.cpp") # No source files in upper layer apart from the main();
	main := $(shell find_linux src -maxdepth 1 -name "*.cpp")
	sub_dir := $(shell find_linux src -type d)
	INC_DIRS += $(addprefix  -I,  $(sub_dir))
	LIB_DIRS := -Lrequires/boost/lib
	LIBS := -lboost_system -lboost_filesystem -lboost_thread -lws2_32 -pthread
	TEST_LIB := -lboost_unit_test_framework-mgw51-mt-1_62
endif

$(info source files are $(SRC))
$(info includes are $(INC_DIRS))
$(info main $(main))

# Executable for normal use
sna1.exe: $(SRC)
	$(CXX) $(CXX_FLAGS) -O3 $(INC_DIRS) -DDEBUG -o$@ $(main) $(LIB_DIRS) $(LIBS)

# Executable for debugging
sna1.debug: $(SRC) requires
	$(CXX) $(CXX_FLAGS) -g -O0 $(INC_DIRS) -DDEBUG -o$@ $(main) $(LIB_DIRS) $(LIBS)

# Executable for profiling
sna1.prof: $(SRC) requires
	$(CXX) $(CXX_FLAGS) -pg -O3 $(INC_DIRS) -o$@ $(main) $(LIB_DIRS) $(LIBS)

# Test executable with coverage (and no optimisation) for tests that run fast
# These tests are likely to be run often during development and don't require optimisation
# The `no-inline` options provide better coverage statistics because lines don't get "inlined away"
tests-fast.exe: $(SRC) tests/fast.cpp $(TEST_CPPS) requires
	$(CXX) $(CXX_FLAGS) -g -O0 --coverage -fno-inline -fno-inline-small-functions -fno-default-inline $(INC_DIRS) -o$@ tests/fast.cpp $(LIB_DIRS) $(LIBS) $(TEST_LIB) -lgcov

# Test executable with no coverage and full optimisation for tests that run very slowly without these
tests-slow.exe: $(SRC) tests/slow.cpp $(TEST_CPPS) requires
	$(CXX) $(CXX_FLAGS) -O3 $(INC_DIRS) -o$@ tests/slow.cpp $(LIB_DIRS) $(LIBS) $(TEST_LIB)


#############################################################
# Running
 
# Run the model
run: sna1.exe
	time ./sna1.exe run


#############################################################
# Testing

# CASAL v230 for a linux binary
.PHONY: casal/casal-230.zip
casal/casal-230.zip:
	wget -O $@ ftp://ftp.niwa.co.nz/software/casal/CASALv230-2012-03-21.zip

# CASAL "latest" for a version of the R package that is compatible
# with recent R versions
casal/casal-latest.zip:
	wget -O $@ ftp://ftp.niwa.co.nz/software/casal/latest_casal.zip

# Unzip a CASAL zip
casal/casal-%: casal/casal-%.zip
	unzip -o -d $@ $<

# Install CASAL
#   - R package
#   - symbolic link to Linux executable
ifeq ($(OS), linux)
casal/casal.installed: casal/casal-230 casal/casal-latest
	R CMD INSTALL casal/casal-latest/R_library/casal_2.30.tar.gz
	chmod 755 casal/casal-230/Program/Linux/casal
	ln -sf casal-230/Program/Linux/casal tests/casal/casal
	touch $@endif
endif
ifeq ($(OS), win)
casal/casal.installed: casal/casal-230 casal/casal-latest
	cmd /C  R CMD INSTALL casal/casal-latest/R_library/casal_2.30.tar.gz
	cp casal/casal-230/Program/casal.exe tests/casal/casal.exe
	touch $@
endif

# Run fast tests
tests-fast: tests-fast.exe
	time ./tests-fast.exe

# Run slow tests
tests-slow: tests-slow.exe casal/casal.installed
	time ./tests-slow.exe

# Run all tests
test: tests-fast tests-slow
