# Variáveis
CC = gcc
CFLAGS = -Wall -std=c99
SRC_DIR = src
BUILD_DIR = build
EXECUTABLE = chip8

# Encontra todos os arquivos .c no diretório src
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SOURCES))

# Regra padrão
all: $(EXECUTABLE)

# Compila o executável
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

# Compila os arquivos .c em .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Cria o diretório build, se necessário
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Limpa os arquivos gerados
clean:
	rm -rf $(BUILD_DIR) $(EXECUTABLE)

# Declara que "all" e "clean" não são arquivos
.PHONY: all clean