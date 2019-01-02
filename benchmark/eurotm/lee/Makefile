include Makefile.in

.PHONY: clean

OBJFILES = $(OBJ_DIR)/lee.o $(OBJ_DIR)/lee_main.o

all: $(OBJ_DIR)/lee

# main
$(OBJ_DIR)/lee: $(OBJFILES)
	$(CPP) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)
	cp $(OBJ_DIR)/lee .

$(OBJ_DIR)/lee_main.o: $(SRC_DIR)/lee_main.cc
	mkdir -p $(OBJ_DIR)
	$(CPP) $(CPPFLAGS) $(SRC_DIR)/lee_main.cc -c -o $@

$(OBJ_DIR)/lee.o: $(SRC_DIR)/lee.cc
	mkdir -p $(OBJ_DIR)
	$(CPP) $(CPPFLAGS) $(SRC_DIR)/lee.cc -c -o $@

# general
clean:
	rm -r $(TARGET_DIR) lee
