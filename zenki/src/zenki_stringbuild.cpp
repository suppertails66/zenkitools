#include "util/TStringConversion.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TBufStream.h"
#include "util/TThingyTable.h"
#include "zenki/ZenkiStringReader.h"
#include <vector>
#include <map>
#include <iostream>

using namespace std;
using namespace BlackT;
using namespace Sms;

const static int scriptPointerTableAddr = 0x14000;
const static int scriptPointerBaseAddr = 0x14000;
const static int numScriptPointers = 0x2B;
const static int dataBlockStart = 0x14056;
const static int dataBlockEnd = 0x141C6;
const static int dataBlockSize = dataBlockEnd - dataBlockStart;

const static int outRomSize = 0x100000;

struct ScriptInfo {
  int scriptNum;
  int addr;
  int size;
};

int main(int argc, char* argv[]) {
  if (argc < 6) {
    cout << "Kishin Douji Zenki string table builder" << endl;
    cout << "Usage: " << argv[0]
      << " <infile> <rom> <offset> <table> <outfile>" << endl;
    
    
    
    return 0;
  }
  
  TIfstream ifs(argv[1]);
  
  TBufStream rom(outRomSize);
  TIfstream inrom;
  inrom.open(argv[2]);
  rom.writeFrom(inrom, inrom.size());
  int remaining = outRomSize - inrom.size();
  for (int i = 0; i < remaining; i++) rom.put(0xFF);
  inrom.close();
  
  TThingyTable thingy;
  int offset = TStringConversion::stringToInt(std::string(argv[3]));
  thingy.readSjis(std::string(argv[4]));
  std::string outfile = std::string(argv[5]);
  
  ZenkiStringReader(ifs, rom, thingy, offset)();
  
  rom.save(outfile.c_str());
}
 
