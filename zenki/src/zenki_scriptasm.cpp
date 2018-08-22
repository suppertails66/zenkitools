#include "util/TStringConversion.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TBufStream.h"
#include "zenki/ZenkiScriptAsm.h"
#include "zenki/ZenkiScriptLink.h"
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

struct ScriptInfo {
  int scriptNum;
  int addr;
  int size;
};

int main(int argc, char* argv[]) {
  if (argc < 3) {
    cout << "Kishin Douji Zenki script assembler" << endl;
    cout << "Usage: " << argv[0]
      << " <infile> <outfile>" << endl;
    
    
    
    return 0;
  }
  
  TIfstream ifs(argv[1]);
  std::string outfile = std::string(argv[2]);
  
  ZenkiAsmStruct asmStruct;
  ZenkiScriptAsm(ifs, asmStruct)();
  
//  asmStruct.activeSection.data.save(outfile.c_str());
  
  TBufStream ofs(0x10000);
  ZenkiScriptLink(asmStruct, ofs, 0x14000)();
  ofs.save(outfile.c_str());
  
//  for (PendingExpressionList::iterator it
//        = asmStruct.activeSection.pendingExpressions.begin();
//       it != asmStruct.activeSection.pendingExpressions.end();
//       ++it) {
//    std::cout << std::hex << it->addr << " " << it->dataType
//              << " " << it->expression << std::endl;
//  } 
  
}
 
