CXX = g++
cli: CXXFLAGS = -Wall -Wextra -Werror -pedantic -fopenmp -std=c++98 -O2
gui: CXXFLAGS = -Wall -Wextra -Werror -pedantic -fopenmp -std=c++98 -O2 -DCOMPILE_WITH_GUI
GLLIBS = -lGL -lglut

OBJECTS = main.o core/camera.o core/ray.o core/scene.o core/triplet.o parsescene/parsescene.o
OBJECTS_WITH_GUI = main-gui.o core/camera.o core/ray.o core/scene.o core/triplet.o gui/gui.o gui/motion.o parsescene/parsescene.o parsescene/parsemotions.o

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

main.o: core/camera.h core/scene.h parsescene/parsescene.h

main-gui.o: main.cpp gui/gui.h core/camera.h core/scene.h parsescene/parsescene.h parsescene/parsemotions.h
	$(CXX) $(CXXFLAGS) -c -o $@ main.cpp

core/camera.o: core/camera.h core/ray.h core/triplet.h

core/ray.o: core/ray.h core/aux.h core/scene.h core/triplet.h

core/scene.o: core/scene.h core/aux.h core/material.h core/ray.h core/triplet.h

core/triplet.o: core/triplet.h core/aux.h

gui/gui.o: gui/gui.h core/camera.h

parsescene/parsescene.o: parsescene/parsescene.h core/material.h core/scene.h

parsescene/parsemotions.o: parsescene/parsemotions.h gui/motion.h

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f $(PROGNAME) $(PROGNAME_WITH_GUI) *.o core/*.o gui/*.o parsescene/*.o

