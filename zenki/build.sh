
echo "********************************************************************************"
echo "Kishin Douji Zenki (Game Gear) English translation build script"
echo "********************************************************************************"

echo "********************************************************************************"
echo "Setting up environment..."
echo "********************************************************************************"

set -o errexit

PATH=".:$PATH"

BASE_PWD=$PWD
INROM="zenki.gg"
OUTROM="zenki_en.gg"
WLADX="./wla-dx/binaries/wla-z80"
WLALINK="./wla-dx/binaries/wlalink"
#rm -f -r "out"

cp "$INROM" "$OUTROM"

mkdir -p out/precmp

echo "********************************************************************************"
echo "Building project tools..."
echo "********************************************************************************"

make blackt
make libsms
make

if [ ! -f $WLADX ]; then
  
  echo "********************************************************************************"
  echo "Building WLA-DX..."
  echo "********************************************************************************"
  
  cd wla-dx
    cmake -G "Unix Makefiles" .
    make
  cd $BASE_PWD
  
fi

echo "********************************************************************************"
echo "Inserting strings..."
echo "********************************************************************************"

# note: also expands ROM
zenki_stringbuild "strings/zenki_strings.txt" "$OUTROM" 0x80000 "table/zenki_en.tbl" "$OUTROM"

echo "********************************************************************************"
echo "Generating font graphics..."
echo "********************************************************************************"

zenki_grpundmp "rsrc/font/font.png" "out/precmp/font.bin" 162 "pal/font.pal"

echo "********************************************************************************"
echo "Generating text-based tilemaps..."
echo "********************************************************************************"

mkdir -p "out/tilescript"
zenki_tilescriptbuild "strings/zenki_stringmaps.txt" "table/zenki_en.tbl" "out/tilescript/"

echo "********************************************************************************"
echo "Running tilemapper scripts..."
echo "********************************************************************************"

mkdir -p "out/maps"
mkdir -p "out/trash"
for file in tilemappers/*; do
  tilemapper_gg $file
done

echo "********************************************************************************"
echo "Compressing graphics..."
echo "********************************************************************************"

mkdir -p "out/cmp"
for file in out/precmp/*; do
  zenki_grpcmp "$file" "out/cmp/$(basename $file)"
done

echo "********************************************************************************"
echo "Generating final injection scripts..."
echo "********************************************************************************"

mkdir -p "out/injectscript"
cat "rsrc/zenki_freespace.txt" "out/tilescript/msg_space.txt" > "out/injectscript/zenki_freespace.txt"
cat "rsrc/zenki_inject_script.txt" "out/tilescript/msg_index.txt" > "out/injectscript/zenki_inject_script.txt"

echo "********************************************************************************"
echo "Injecting data..."
echo "********************************************************************************"

mkdir -p "out"
datinjct "out/injectscript/zenki_freespace.txt" "out/injectscript/zenki_inject_script.txt" "$OUTROM" "$OUTROM" "out/injectdef.txt"

echo "********************************************************************************"
echo "Assembling scripts..."
echo "********************************************************************************"

mkdir -p "out/scripts"
zenki_scriptasm "scripts/scripts.txt" "out/scripts/scripts.bin"
datpatch "$OUTROM" "$OUTROM" "out/scripts/scripts.bin" 0x14000

echo "********************************************************************************"
echo "Applying ASM patches..."
echo "********************************************************************************"

cp "$OUTROM" "asm/zenki.gg"

cd asm
  # apply hacks
  ../$WLADX -o "main.o" "main.s"
  ../$WLALINK -v linkfile zenki_patched.gg
  
  mv -f "zenki_patched.gg" "zenki.gg"
  
  # update region code in header (WLA-DX forces it to 4,
  # for "export SMS", when the .smstag directive is used
  # -- we want 7, for "international GG")
  ../$WLADX -o "main2.o" "main2.s"
  ../$WLALINK -v linkfile2 zenki_patched.gg
cd "$BASE_PWD"

mv -f "asm/zenki_patched.gg" "$OUTROM"
rm "asm/zenki.gg"
rm "asm/main.o"
rm "asm/main2.o"

echo "********************************************************************************"
echo "Success!"
echo "Output ROM: $OUTROM"
echo "********************************************************************************"



