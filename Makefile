CXX = g++
CXXFLAGS = -Wall -O2 -fPIC -DLINUX
DEBUGFLAGS = -Wall -g -O0 -fPIC -DLINUX
LDFLAGS = -Llib -lCAENDigitizer -lCAENDPPLib -lCAENN957 -lCAENComm -lCAENVME -lusb-1.0 -lCore -lGui -lX11 -Wl,-rpath,$(PWD)/lib
ROOTCFLAGS = $(shell root-config --cflags) -DLINUX
ROOTLIBS = $(shell root-config --libs --glibs)
INCDIR = include
SRCDIR = src
BUILDDIR = build
SOURCES = $(SRCDIR)/WindowMain.cpp $(SRCDIR)/WindowGenerator.cpp $(SRCDIR)/DigitizerClass.cpp $(SRCDIR)/ParamsMenu.cpp $(SRCDIR)/DT5770Params.cpp $(SRCDIR)/LogicMenu.cpp $(SRCDIR)/FuncMain.cpp $(SRCDIR)/FuncGenerator.cpp $(SRCDIR)/Main.cpp
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SOURCES))
HEADERS = $(INCDIR)/WindowMain.h $(INCDIR)/WindowGenerator.h $(INCDIR)/DigitizerClass.h $(INCDIR)/ParamsMenu.h $(INCDIR)/DT5770Params.h $(INCDIR)/LogicMenu.h $(INCDIR)/FuncMain.h $(INCDIR)/FuncGenerator.h
DICT_SRC = $(BUILDDIR)/G__PsiDAQ.cxx
DICT_OBJ = $(DICT_SRC:.cxx=.o)
EXEC = $(BUILDDIR)/PsiDAQ
PCM_FILE = $(BUILDDIR)/G__PsiDAQ_rdict.pcm
COPIED_PCM = G__PsiDAQ_rdict.pcm
INCLUDES = -I$(INCDIR)

CXXFLAGS += -g -O0 -pthread
LDFLAGS += -pthread

# Dynamically calculate total steps: number of source files + 3 (dict gen, dict compile, linking)
NUM_SOURCES = $(words $(SOURCES))
TOTAL_STEPS = $(shell expr $(NUM_SOURCES) + 3)
STEPS = 0

# Macro to increment steps and calculate percentage
define incr_step
$(eval STEPS = $(shell expr $(STEPS) + 1))
$(eval PERCENT = $(shell expr $(STEPS) '*' 100 / $(TOTAL_STEPS)))
@echo "[$(PERCENT)%] $(1)"
endef

all: $(EXEC)

debug: CXXFLAGS = $(DEBUGFLAGS)
debug: $(EXEC)

$(EXEC): $(OBJECTS) $(DICT_OBJ)
	$(call incr_step,Linking executable $(notdir $(EXEC)))
	@mkdir -p $(BUILDDIR)
	@$(CXX) $(OBJECTS) $(DICT_OBJ) $(LDFLAGS) $(ROOTLIBS) -o $@
	@cp $(EXEC) .
	@cp $(PCM_FILE) $(COPIED_PCM)
	@echo "[100%] Build complete"

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(call incr_step,Building CXX object $(notdir $@))
	@mkdir -p $(BUILDDIR)
	@$(CXX) $(CXXFLAGS) $(ROOTCFLAGS) $(INCLUDES) -c $< -o $@

$(DICT_SRC): $(HEADERS) $(INCDIR)/LinkDef.h
	$(call incr_step,Generating dictionary $(notdir $(@F)))
	@mkdir -p $(BUILDDIR)
	@cd $(BUILDDIR) && rootcling -f $(notdir $(DICT_SRC)) $(addprefix ../,$(HEADERS)) $(addprefix ../,$(INCDIR)/LinkDef.h) -DLINUX
	@sed -i.bak 's|#include "include/|#include "|g' $@

$(DICT_OBJ): $(DICT_SRC)
	$(call incr_step,Building dictionary object $(notdir $(@F)))
	@$(CXX) $(CXXFLAGS) $(ROOTCFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@echo "[0%] Cleaning build artifacts"
	@rm -rf $(BUILDDIR)/*.o $(BUILDDIR)/*.cxx $(BUILDDIR)/*.pcm $(BUILDDIR)/*.rootmap $(BUILDDIR)/*.bak $(EXEC)
	@rm -f PsiDAQ $(COPIED_PCM)
	@echo "[100%] Clean complete"

.PHONY: all clean
