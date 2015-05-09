CXX = g++
cli: CXXFLAGS = -Wall -Wextra -Werror -pedantic -fopenmp -std=c++98 -O2
gui: CXXFLAGS = -Wall -Wextra -Werror -pedantic -fopenmp -std=c++98 -O2 -DCOMPILE_WITH_GUI
GLLIBS = -lGL -lglut

OBJECTS = src/main.o src/core/camera.o src/core/ray.o src/core/scene.o src/core/triplet.o src/parsescene/parsescene.o
OBJECTS_WITH_GUI = src/main-gui.o src/core/camera.o src/core/ray.o src/core/scene.o src/core/triplet.o src/gui/gui.o src/gui/motion.o src/parsescene/parsescene.o src/parsescene/parsemotions.o

PROGNAME = retra
PROGNAME_WITH_GUI = retra-gui

.PHONY: all
all: cli gui

.PHONY: cli
cli: $(PROGNAME)
	@echo ==== Command line Retra built successfully ====

$(PROGNAME): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $(PROGNAME) $^ -fopenmp

.PHONY: gui
gui: $(PROGNAME_WITH_GUI)
	@echo ==== Graphical Retra built successfully ====

$(PROGNAME_WITH_GUI): $(OBJECTS_WITH_GUI)
	$(CXX) $(LDFLAGS) -o $(PROGNAME_WITH_GUI) $^ -fopenmp $(GLLIBS)

src/main.o: src/core/camera.h src/core/scene.h src/parsescene/parsescene.h

src/main-gui.o: src/main.cpp src/gui/gui.h src/core/camera.h src/core/scene.h src/parsescene/parsescene.h src/parsescene/parsemotions.h
	$(CXX) $(CXXFLAGS) -c -o $@ src/main.cpp

src/core/camera.o: src/core/camera.h src/core/ray.h src/core/triplet.h

src/core/ray.o: src/core/ray.h src/core/aux.h src/core/scene.h src/core/triplet.h

src/core/scene.o: src/core/scene.h src/core/aux.h src/core/material.h src/core/ray.h src/core/triplet.h

src/core/triplet.o: src/core/triplet.h src/core/aux.h

src/gui/gui.o: src/gui/gui.h src/core/camera.h

src/parsescene/parsescene.o: src/parsescene/parsescene.h src/core/material.h src/core/scene.h

src/parsescene/parsemotions.o: src/parsescene/parsemotions.h src/gui/motion.h

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f $(PROGNAME) $(PROGNAME_WITH_GUI) src/*.o src/core/*.o src/gui/*.o src/parsescene/*.o

