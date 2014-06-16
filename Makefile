PROJECT = avpVision
CC = g++

CFLAGS = -std=c++11 -Wall

INCLUDES =-I/usr/local/include/opencv -I/usr/local/include

LFLAGS = -L/usr/local/lib 

LIBS =-lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_contrib -lopencv_legacy -lopencv_flann

SRCS =main.cpp

OBJECTS=$(SRCS:.cpp=.o)

CALIBRATION=calibrateCam

all:$(PROJECT) $(CALIBRATION)

$(CALIBRATION): camera_calibration.cpp
	$(CC) $(CFLAGS) camera_calibration.cpp $(LFLAGS) $(LIBS) $(INCLUDES) -o $@

$(PROJECT): $(OBJECTS) 
	$(CC) $(LFLAGS) $(LIBS) -o $@ $<
%.o: %.cpp
	$(CC) -c $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -o $@ $<

run: $(PROJECT)
	./$(PROJECT)

.PHONY: clean
clean:
	rm -f $(PROJECT) %.o 
