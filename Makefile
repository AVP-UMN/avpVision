PROJECT = avpVision
CC = g++

COMPILE_OPTIONS = -std=c++11 -Wall

#Include directories
INCLUDE =-I/usr/local/include/opencv -I/usr/local/include
#Libraries for linking
LIBS =-L/usr/local/lib -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_contrib -lopencv_legacy -lopencv_flann

SOURCE=main.cpp

OBJECTS=$(patsubst %.c,%.o, $(SOURCE))

CALIBRATION=calibrateCam

all:$(PROJECT) $(CALIBRATION)

$(CALIBRATION): camera_calibration.cpp
	$(CC) $(COMPILE_OPTIONS) camera_calibration.cpp $(LIBS) $(INCLUDE) -o $@

$(PROJECT):$(OBJECTS) 
	$(CC) $(OBJECTS) $(LIBS) -o $@
%.o: %.cpp
	$(CC) -c $(COMPILE_OPTIONS) $(INCLUDE) $(LIBS) -o $@ $<

run: $(PROJECT)
	./$(PROJECT)

.PHONY: clean
clean:
	rm -f $(PROJECT) %.o 
