sudo ~/bin/opcontrol --no-vmlinux
sudo ~/bin/opcontrol --reset
sudo ~/bin/opcontrol --setup --event=BSQ_CACHE_REFERENCE:10000:0x100
sudo ~/bin/opcontrol --start
./scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver1.txt
mv itm.log ../../Results_Small/cpu4/ITM/itm_ver1.txt
sudo ~/bin/opcontrol --dump
sudo ~/bin/opcontrol -h
sudo ~/bin/opreport -l ./scalparc > ../../Results_Small/cpu4/Oprofile/oprofile_L2cachemisses_ver1.txt
sudo ~/bin/opcontrol --reset
sudo ~/bin/opcontrol --setup --event=BSQ_CACHE_REFERENCE:10000:0x100
sudo ~/bin/opcontrol --start
./scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver2.txt
mv itm.log ../../Results_Small/cpu4/ITM/itm_ver2.txt
sudo ~/bin/opcontrol --dump
sudo ~/bin/opcontrol -h
sudo ~/bin/opreport -l ./scalparc > ../../Results_Small/cpu4/Oprofile/oprofile_L2cachemisses_ver2.txt
sudo ~/bin/opcontrol --reset
sudo ~/bin/opcontrol --setup --event=BSQ_CACHE_REFERENCE:10000:0x100
sudo ~/bin/opcontrol --start
./scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver3.txt
mv itm.log ../../Results_Small/cpu4/ITM/itm_ver3.txt
sudo ~/bin/opcontrol --dump
sudo ~/bin/opcontrol -h
sudo ~/bin/opreport -l ./scalparc > ../../Results_Small/cpu4/Oprofile/oprofile_L2cachemisses_ver3.txt
sudo ~/bin/opcontrol --reset
sudo ~/bin/opcontrol --setup --event=BSQ_CACHE_REFERENCE:10000:0x100
sudo ~/bin/opcontrol --start
./scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver4.txt
mv itm.log ../../Results_Small/cpu4/ITM/itm_ver4.txt
sudo ~/bin/opcontrol --dump
sudo ~/bin/opcontrol -h
sudo ~/bin/opreport -l ./scalparc > ../../Results_Small/cpu4/Oprofile/oprofile_L2cachemisses_ver4.txt
sudo ~/bin/opcontrol --reset
sudo ~/bin/opcontrol --setup --event=BSQ_CACHE_REFERENCE:10000:0x100
sudo ~/bin/opcontrol --start
./scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver5.txt
mv itm.log ../../Results_Small/cpu4/ITM/itm_ver5.txt
sudo ~/bin/opcontrol --dump
sudo ~/bin/opcontrol -h
sudo ~/bin/opreport -l ./scalparc > ../../Results_Small/cpu4/Oprofile/oprofile_L2cachemisses_ver5.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver6.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver6.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver7.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver7.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver8.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver8.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver9.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver9.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver10.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver10.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver11.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver11.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver12.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver12.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver13.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver13.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver14.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver14.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver15.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver15.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver16.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver16.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver17.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver17.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver18.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver18.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver19.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver19.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver20.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver20.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver21.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver21.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver22.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver22.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver23.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver23.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver24.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver24.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver25.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver25.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver26.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver26.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver27.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver27.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver28.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver28.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver29.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver29.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver30.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver30.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver31.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver31.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver32.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver32.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver33.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver33.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver34.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver34.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver35.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver35.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver36.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver36.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver37.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver37.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver38.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver38.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver39.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver39.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver40.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver40.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver41.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver41.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver42.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver42.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver43.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver43.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver44.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver44.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver45.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver45.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver46.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver46.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver47.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver47.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver48.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver48.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver49.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver49.txt
# ../../scalparc ../../../../datasets/ScalParC/para_F26-A32-D125K/F26-A32-D125K.tab 125000 32 2 4 > ../../Results_Small/cpu4/Output/cpu4_ver50.txt
# mv itm.log ../../Results_Small/cpu4/ITM/itm_ver50.txt