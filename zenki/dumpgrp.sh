# 
# mkdir -p rsrc/font/orig
# #./zenki_psgrpdmp zenki.gg rsrc/font/font.png 0x2714F -p pal/font.pal
# ./zenki_grpdmp zenki.gg rsrc/font/font.png 0x2714F -p pal/font.pal
# #./zenki_grpdecmp zenki.gg 0x2714F rsrc/font/font.bin
# cp rsrc/font/* rsrc/font/orig
# 
# #mkdir -p rsrc/title/orig
# #./zenki_grpdmp zenki.gg rsrc/title/title.png 0x40000 -p pal/grayscale.pal
# #cp rsrc/title/* rsrc/title/orig
# 
# #mkdir -p rsrc/titlecard/orig
# #./zenki_grpdmp zenki.gg rsrc/titlecard/font.png 0x3B6CA -p pal/titlecard.pal
# #cp rsrc/titlecard/* rsrc/titlecard/orig
# 
# # dump raw tiles for stuff that needs them
# mkdir -p rsrc_raw/titlecard
# ./zenki_grpdecmp zenki.gg 0x3B6CA rsrc_raw/titlecard/tiles.bin
# mkdir -p rsrc_raw/title
# ./zenki_grpdecmp zenki.gg 0x40000 rsrc_raw/title/tiles.bin
# 
# # title card tilemaps
# mkdir -p rsrc/titlecard/orig
# ./tilemapdmp_gg zenki.gg 0x3BF19 half 7 2 rsrc_raw/titlecard/tiles.bin 0x40 rsrc/titlecard/card00.png -p pal/titlecard.pal
# ./tilemapdmp_gg zenki.gg 0x3BF27 half 6 2 rsrc_raw/titlecard/tiles.bin 0x40 rsrc/titlecard/card01.png -p pal/titlecard.pal
# ./tilemapdmp_gg zenki.gg 0x3BF33 half 7 2 rsrc_raw/titlecard/tiles.bin 0x40 rsrc/titlecard/card02.png -p pal/titlecard.pal
# ./tilemapdmp_gg zenki.gg 0x3BF41 half 6 2 rsrc_raw/titlecard/tiles.bin 0x40 rsrc/titlecard/card03.png -p pal/titlecard.pal
# ./tilemapdmp_gg zenki.gg 0x3BF4D half 7 2 rsrc_raw/titlecard/tiles.bin 0x40 rsrc/titlecard/card04.png -p pal/titlecard.pal
# ./tilemapdmp_gg zenki.gg 0x3BF5B half 7 2 rsrc_raw/titlecard/tiles.bin 0x40 rsrc/titlecard/card05.png -p pal/titlecard.pal
# ./tilemapdmp_gg zenki.gg 0x3BF69 half 7 2 rsrc_raw/titlecard/tiles.bin 0x40 rsrc/titlecard/card06.png -p pal/titlecard.pal
# ./tilemapdmp_gg zenki.gg 0x3BF77 half 6 2 rsrc_raw/titlecard/tiles.bin 0x40 rsrc/titlecard/card07.png -p pal/titlecard.pal
# ./tilemapdmp_gg zenki.gg 0x3BF83 half 7 2 rsrc_raw/titlecard/tiles.bin 0x40 rsrc/titlecard/card08.png -p pal/titlecard.pal
# ./tilemapdmp_gg zenki.gg 0x3BF91 half 5 2 rsrc_raw/titlecard/tiles.bin 0x40 rsrc/titlecard/card09.png -p pal/titlecard.pal
# ./tilemapdmp_gg zenki.gg 0x3BF9B half 6 2 rsrc_raw/titlecard/tiles.bin 0x40 rsrc/titlecard/card0A.png -p pal/titlecard.pal
# ./tilemapdmp_gg zenki.gg 0x3BFA7 half 6 2 rsrc_raw/titlecard/tiles.bin 0x40 rsrc/titlecard/card0B.png -p pal/titlecard.pal
# ./tilemapdmp_gg zenki.gg 0x3BFB3 half 7 2 rsrc_raw/titlecard/tiles.bin 0x40 rsrc/titlecard/card0C.png -p pal/titlecard.pal
# ./tilemapdmp_gg zenki.gg 0x3BFC1 half 8 2 rsrc_raw/titlecard/tiles.bin 0x40 rsrc/titlecard/card0D.png -p pal/titlecard.pal
# cp rsrc/titlecard/* rsrc/titlecard/orig
# 
# # title screen tilemaps
# mkdir -p rsrc/title/orig
# ./tilemapdmp_gg zenki.gg 0x41DAF full 0x14 0x12 rsrc_raw/title/tiles.bin 0x0 rsrc/title/intro.png -p pal/grayscale.pal
# ./tilemapdmp_gg zenki.gg 0x4207F full 0x14 0x12 rsrc_raw/title/tiles.bin 0x0 rsrc/title/main.png -p pal/grayscale.pal
# cp rsrc/title/* rsrc/title/orig
# 



# mkdir rsrc_raw/font
# ./zenki_grpdecmp zenki.gg 0x2714F rsrc_raw/font/font.bin
# 
# # game over tilemaps
# #mkdir -p rsrc/gameover/orig
# #./tilemapdmp_gg zenki.gg 0x73CE2 half 0x8 0x1 rsrc_raw/font/font.bin 0x11F #rsrc/gameover/prompt.png -p pal/font.pal -h 0x01
# #cp rsrc/gameover/* rsrc/gameover/orig
# 
# 
# mkdir -p rsrc_raw/end
# ./zenki_grpdecmp zenki.gg 0x5C265 rsrc_raw/end/end.bin
# 
# mkdir -p rsrc/end/orig
# ./tilemapdmp_gg zenki.gg 0x5D614 full 0x3 0x5 rsrc_raw/end/end.bin 0x100 rsrc/end/end.png -p pal/grayscale.pal
# cp rsrc/end/* rsrc/end/orig

mkdir -p rsrc_raw/gameover
./zenki_grpdecmp zenki.gg 0x27ACF rsrc_raw/gameover/gameover.bin

#mkdir -p rsrc/end/orig
./tilemapdmp_gg zenki.gg 0x27ED7 full 0x10 0x4 rsrc_raw/gameover/gameover.bin 0x0 gameover.png -p pal/grayscale.pal
#cp rsrc/end/* rsrc/end/orig

