OBJECTS = camfilter capture capturevideo findBallse fiteellipse_cam fiteellipse_camfilter

all: $(OBJECTS)

clean:
	rm -f $(OBJECTS)

camfilter: camfilter.c
	$(CXX) $^ -o $@ `pkg-config --cflags --libs opencv`

capture: capture.c
	$(CXX) $^ -o $@ `pkg-config --cflags --libs opencv`

capturevideo: capturevideo.c
	$(CXX) $^ -o $@ `pkg-config --cflags --libs opencv`

findBallse: findBallse.c
	$(CXX) $^ -o $@ `pkg-config --cflags --libs opencv`

fiteellipse_cam: fiteellipse_cam.c
	$(CXX) $^ -o $@ `pkg-config --cflags --libs opencv`

fiteellipse_camfilter: fiteellipse_camfilter.c
	$(CXX) $^ -o $@ `pkg-config --cflags --libs opencv`