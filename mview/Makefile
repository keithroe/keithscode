

CXX       = g++ 
CC        = gcc 
LEX       = flex 
YACC      = bison -y

LIBS      = -lm  


#CXXFLAGS  = -O3 -DNDEBUG -g3 -Wall -I./src 
#CFLAGS    = -O3 -DNDEBUG -g -Wall -I./src 
CXXFLAGS  = -g -Wall -I./src 
CFLAGS    = -g -Wall -I./src 
GLLIBS    = -framework GLUT -framework OpenGL -framework Foundation
#GLLIBS    = -lglut -lGL -lGLU -lX11
LDFLAGS   =  
LEXLIB    = 

SRCDIR    = .

###############################################################################
#
#   search through source tree finding all plugins and source files
#
###############################################################################

SOURCES      := $(wildcard  $(SRCDIR)/src/*/*.cc)  
OBJS         := $(addprefix objs/, $(notdir $(SOURCES:.cc=.o))) 
CSOURCES     := $(wildcard  $(SRCDIR)/src/*/*.c)  
COBJS        := $(addprefix objs/, $(notdir $(CSOURCES:.c=.o))) 
CORE_LIB     := objs/libmesh.a

CORE_HEADERS := $(wildcard $(SRCDIR)/src/mesh/*.h) \
                $(wildcard $(SRCDIR)/src/math/*.h) \
		$(wildcard $(SRCDIR)/src/viewer/*.h) \
                $(wildcard $(SRCDIR)/src/io/*.h) 

CORE_OBJS    := $(wildcard $(SRCDIR)/src/mesh/*.cc) \
                $(wildcard $(SRCDIR)/src/math/*.cc) \
		$(wildcard $(SRCDIR)/src/viewer/*.cc) \
		$(wildcard $(SRCDIR)/src/io/*.cc) 
CORE_OBJS    := $(addprefix objs/, $(notdir $(CORE_OBJS:.cc=.o)))

CORELIB_OBJS := $(filter-out %main.o, $(CORE_OBJS))




###############################################################################
#
#  Build rules
#
###############################################################################


default: mkdirs output $(CORE_LIB) main $(OBJS) $(COBJS) 

mkdirs: 
	@mkdir -p objs;
	@mkdir -p bin;
       
output: 
	@echo "Building with cpp compiler $(CXX) with flags:"
	@echo "  $(CXXFLAGS)"
	@echo "c compiler $(CC) with flags:"
	@echo "  $(CFLAGS)"

$(CORE_LIB): $(CORELIB_OBJS) $(COBJS)
	@echo "Building the core mesh library (libmesh .a)"
	@ar rcs $(CORE_LIB) $(CORELIB_OBJS) $(COBJS)

objs/%.o: $(SRCDIR)/src/mesh/%.cc $(CORE_HEADERS)
	@echo "Building $<"
	@$(CXX) $(CXXFLAGS) -o $@ -c $< 

objs/%.o: $(SRCDIR)/src/math/%.cc $(CORE_HEADERS)
	@echo "Building $<"
	@$(CXX) $(CXXFLAGS) -o $@ -c $< 

objs/%.o: $(SRCDIR)/src/viewer/%.cc $(CORE_HEADERS)
	@echo "Building $<"
	@$(CXX) $(CXXFLAGS) -o $@ -c $< 

objs/%.o: $(SRCDIR)/src/io/%.cc $(CORE_HEADERS)
	@echo "Building $<"
	@$(CXX) $(CXXFLAGS) -o $@ -c $< 

objs/%.o: $(SRCDIR)/src/io/%.c $(CORE_HEADERS)
	@echo "Building $<"
	@$(CC) $(CFLAGS) -o $@ -c $< 


main: objs/main.o $(CORE_LIB)
	@echo "Linking main... "  
	@echo "  libs : $(LIBS)"  
	@echo "  gl libs: $(GLLIBS)"  
	@echo "  flags: $(LDFLAGS)"
	@$(CXX) $(CXXFLAGS) $(COBJS) $(CORE_OBJS) -o bin/vmesh $(LIBS) $(GLLIBS) $(LDFLAGS)	



.PHONY: clean 
clean:
	rm  -rf bin/* objs/*.o lib.*/*.a


test:
	@echo " CC = $(CC) "
	@echo " CXX = $(CC) "
	@echo " FLEX = $(CC) "
	@echo " YACC = $(CC) "
	@echo " LEXLIB = $(CC) "
	@echo " LIBS = $(CC) "
	@echo " CC = $(CC) "
	@echo " CC = $(CC) "
	@echo " COBJS = $(COBJS) "
	@echo " CORE_OBJS = $(CORE_OBJS) "
        

