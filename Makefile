CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -pedantic -fopenmp -std=c++98 -O2
#GLLIBS = -lGL -lGLU -lglut

OBJECTS = main.o core/camera.o core/ray.o core/scene.o core/triplet.o parsescene/parsescene.o

PROGNAME = retra

.PHONY: cli
cli: $(PROGNAME)
	@echo ==== Retra built successfully ====

$(PROGNAME): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $(PROGNAME) $^ -fopenmp

#.PHONY: gui
#gui: gui/gui.h $(OBJECTS)
#	$(CXX) $(LDFLAGS) -o $(PROGNAME) $^ -fopenmp $(GLLIBS)
#	@echo ==== Retra built successfully ====

main.o: core/camera.h parsescene/parsescene.h

core/camera.o: core/camera.h core/ray.h core/triplet.h

core/ray.o: core/ray.h core/aux.h core/scene.h core/triplet.h

core/scene.o: core/scene.h core/aux.h core/material.h core/ray.h core/triplet.h

core/triplet.o: core/triplet.h core/aux.h

parsescene/parsescene.o: core/material.h core/scene.h

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f $(PROGNAME) *.o core/*.o parsescene/*.o

