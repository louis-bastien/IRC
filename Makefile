# Compiler settings
CXX = c++
CXXFLAGS = -Wall -Werror -Wextra -std=c++98 -fsanitize=address

# Target executable name
TARGET = ircserv

#Folders
SRC_DIR = src
OBJ_DIR = obj
INC_DIR = include

#Filenames
FILES = irc

# Source, Object, Dependency files
SRC = $(FILES:%=$(SRC_DIR)/%.cpp)
OBJ = $(FILES:%=$(OBJ_DIR)/%.o)
DEP = $(OBJ:.o=.d)

# Default target
all: $(TARGET)

# Run target ignoring exit status
run: all
	./$(TARGET) 4242 mypassword

# Compile .cpp files into .o object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -MMD -MP -c $< -o $@

# Link object files into the target executable
$(TARGET): $(OBJ) Makefile
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET)

# Clean up objects and dependency files
clean:
	rm -rf $(OBJ)
	rm -rf $(OBJ:.o=.d)
	
#Also clean up the target executable (ignore error if file does not exist)
fclean: clean
	-rm -f $(TARGET)

#Clean up and recompile
re: fclean all

# Include dependencies
-include $(DEP)