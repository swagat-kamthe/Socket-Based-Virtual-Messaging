TARGET = brain
CC = gcc
CFLAGS = -pthread
SRC = brain.c ear.c mouth.c
CONFIG_FILES = file1.txt file2.txt file3.txt file4.txt

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: all
	@echo "Launching terminals..."
	@for file in $(CONFIG_FILES); do \
		gnome-terminal -- bash -c "./$(TARGET) $$file; exec bash"; \
	done

clean:
	rm -f $(TARGET)

.PHONY: all run clean

