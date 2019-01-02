TM_DIR=${TSX_ROOT}/lib/rtm
TM_FLAGS= #-DUSE_TLH -DLIST_NO_DUPLICATES -DCHUNK_STEP1=2 -DHTM_IBM
TM_OBJS=#$(TM_DIR)/thread.o $(TM_DIR)/htm_util.o $(TM_DIR)/htm_ibm.o


CXXFLAGS += -fopenmp -DRTM -I$(TM_DIR) $(TM_FLAGS)
CFLAGS += -fopenmp -DRTM -I$(TM_DIR) $(TM_FLAGS)
LIBS += -lpthread -L$(TM_DIR) -lrtm 
#LDFLAGS += 
