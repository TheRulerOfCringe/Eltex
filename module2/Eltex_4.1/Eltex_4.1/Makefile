CC = gcc

TARGET = DSK

SOURCE = DSK.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(SOURCE) -o $(TARGET)

clean:
	rm -f $(TARGET) *.exe

rebuild: clean all

.PHONY: all clean rebuild