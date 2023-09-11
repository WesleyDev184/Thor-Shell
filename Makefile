CC = gcc
CFLAGS = -Wall -g
SRC = src/main.c   # Substitua com a lista de seus arquivos de origem
TARGET = shell # Substitua pelo nome do seu executável

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC)

.PHONY: clean start

clean:
	rm -f $(TARGET) & clear

start:
	./$(TARGET)