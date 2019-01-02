ssl :
ifeq (, $(wildcard ${PARSEC_ROOT}/pkgs/lib/ssl/libssl.a))
	cd ${PARSEC_ROOT}/pkgs/lib/ssl;\
	chmod u+x configure;\
	./configure -no-asm;\
	${TSX_ROOT}/tool/makefile_editor.py Makefile Makefile.new "DIRS= crypto ssl";\
	mv Makefile.new Makefile;\
	make -j AR_BIN=ar RANLIB=ranlib
endif

librtm.so :
ifeq (, $(wildcard $(TM_DIR)/librtm.so))
	cd $(TM_DIR); make -C $(TM_DIR) CC=$(CC)
endif
