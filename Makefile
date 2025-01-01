TARGET=ctrl_monoprice_4067
CFLAGS=-Wall -std=c99

.PHONY: all clean

all: $(TARGET)

$(TARGET):

clean:
	rm -f $(TARGET)
