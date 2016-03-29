OS := $(shell uname)
ARCH := $(shell uname -m)
PROGRAMA := env3D_v1_3.cpp
LEAP_LIBRARY := lib/x64/libLeap.so -Wl,-rpath,lib/x64
OTHERS_LIBRARY := -lGL -lGLU -lglut -lm -lIL -lILU -lILUT


make: $(PROGRAMA) include/ShadowMapping.h
	$(CXX) -Wall -g -Iinclude $(PROGRAMA) -o $(PROGRAMA:.cpp=) $(LEAP_LIBRARY) $(OTHERS_LIBRARY) 

run:
	./$(PROGRAMA:.cpp=)

clean:
	rm -rf Sample Sample.dSYM
