CXX = g++
cli: CXXFLAGS = -Wall -Wextra -Werror -pedantic -fopenmp -std=c++11 -O2
gui: CXXFLAGS = -Wall -Wextra -Werror -pedantic -fopenmp -std=c++11 -O2 -DCOMPILE_WITH_GUI
GLLIBS = -lGL -lglut

OBJECTS = src/main.o src/core/beam.o src/core/camera.o src/core/ray.o src/core/renderer.o src/core/scene.o src/core/shadow.o src/core/triplet.o src/core/zone.o src/parser/parsescene.o
OBJECTS_WITH_GUI = src/main-gui.o src/core/beam.o src/core/camera.o src/core/ray.o src/core/renderer.o src/core/scene.o src/core/shadow.o src/core/triplet.o src/core/zone.o src/gui/gui.o src/gui/motion.o src/parser/parsescene.o src/parser/parsemotions.o

PROGNAME = silence
PROGNAME_WITH_GUI = silence-gui

.PHONY: all
all: cli gui

.PHONY: cli
cli: $(PROGNAME)
	@echo ==== Command line Silence built successfully ====

$(PROGNAME): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $(PROGNAME) $^ -fopenmp

.PHONY: gui
gui: $(PROGNAME_WITH_GUI)
	@echo ==== Graphical Silence built successfully ====

$(PROGNAME_WITH_GUI): $(OBJECTS_WITH_GUI)
	$(CXX) $(LDFLAGS) -o $(PROGNAME_WITH_GUI) $^ -fopenmp $(GLLIBS)

src/main.o: src/core/camera.h src/core/renderer.h src/core/scene.h src/parser/parsescene.h

src/main-gui.o: src/gui/gui.h src/core/camera.h src/core/renderer.h src/core/scene.h src/parser/parsescene.h src/parser/parsemotions.h
	$(CXX) $(CXXFLAGS) -c -o $@ src/main.cpp

src/core/beam.o: src/core/beam.h src/core/ray.h src/core/scene.h src/core/triplet.h

src/core/camera.o: src/core/camera.h src/core/triplet.h

src/core/ray.o: src/core/ray.h src/core/aux.h src/core/scene.h src/core/triplet.h

src/core/renderer.o: src/core/renderer.h src/core/camera.h src/core/scene.h src/core/tree.h src/core/zone.h

src/core/scene.o: src/core/scene.h src/core/aux.h src/core/material.h src/core/ray.h src/core/tree.h src/core/triplet.h

src/core/shadow.o: src/core/shadow.h src/core/beam.h

src/core/zone.o: src/core/zone.h src/core/beam.h src/core/shadow.h

src/core/triplet.o: src/core/triplet.h src/core/aux.h

src/gui/gui.o: src/gui/gui.h src/core/camera.h

src/gui/motion.o: src/gui/motion.h src/core/scene.h src/core/triplet.h

src/parser/parsescene.o: src/parser/parsescene.h src/core/material.h src/core/scene.h

src/parser/parsemotions.o: src/parser/parsemotions.h src/gui/motion.h

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f $(PROGNAME) $(PROGNAME_WITH_GUI) src/*.o src/core/*.o src/gui/*.o src/parser/*.o

