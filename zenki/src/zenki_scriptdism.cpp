#include "util/TStringConversion.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TBufStream.h"
#include "zenki/ZenkiScriptDism.h"
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
    cout << "Kishin Douji Zenki script disassembler" << endl;
    cout << "Usage: " << argv[0]
      << " <rom> <outprefix>" << endl;
    
    
    
    return 0;
  }
  
  TIfstream ifs(argv[1]);
  std::string outprefix = std::string(argv[2]);
  
//  ZenkiScriptPointerMap scriptPointers;
  ZenkiDismStruct dismStruct;
  std::vector<ScriptInfo> scriptInfos;
  std::map<int, ZenkiScriptOpCollection> scripts;
  
  // disassemble the enumerated scripts
  
  for (int i = 0; i < numScriptPointers; i++) {
    ZenkiScriptOpCollection ops;
    
    ifs.seek(scriptPointerTableAddr + (i * 2));
    int ptr = ifs.readu16le();
    
    if (ptr == 0x0000) {
      std::cout << "Skipping dummy script "
        << std::hex << i << std::endl;
      continue;
    }
    
    int addr = scriptPointerBaseAddr + (ptr % 0x4000);
    ifs.seek(addr);
    
    int scriptStart = addr;
    
    std::cout << "Disassembling script "
      << std::hex << i << " at " << std::hex << ifs.tell() << std::endl;
    
    // check for overlap
    bool overlaps = false;
    for (int i = 0; i < scriptInfos.size(); i++) {
      if ((addr >= scriptInfos[i].addr)
          && (addr < scriptInfos[i].addr + scriptInfos[i].size)) {
        std::cout << "pos " << std::hex << addr << " overlaps" << std::endl;
        overlaps = true;
        break;
      }
    }
    
    ZenkiScriptDism(ifs, ops, dismStruct)();
    
    int scriptEnd = ifs.tell();
    int scriptSize = scriptEnd - scriptStart;
    
    ScriptInfo info;
    info.scriptNum = i;
    info.addr = scriptStart;
    info.size = scriptSize;
    scriptInfos.push_back(info);
    
    // don't multiply disassemble overlapping scripts
    // (not foolproof)
    if (overlaps) continue;
    
    scripts[addr] = ops;
  }
  
  // disassemble anything referenced in the enumerated scripts
  // (and anything referenced in those, and ...)
  
  bool unvisitedFound;
  do {
    unvisitedFound = false;
    for (ZenkiScriptPointerMap::iterator it = dismStruct.scriptPointers.begin();
         it != dismStruct.scriptPointers.end();
         ++it) {
  //    std::cerr << std::hex << it->first << std::endl;
      
      if (it->second.visited) continue;
      
      unvisitedFound = true;
      it->second.visited = true;
      
      int ptr = it->first;
      int addr = scriptPointerBaseAddr + (ptr % 0x4000);
      
      // don't disassemble code that is already part of an enumerated script
      bool overlaps = false;
      for (int i = 0; i < scriptInfos.size(); i++) {
        if ((addr >= scriptInfos[i].addr)
            && (addr < scriptInfos[i].addr + scriptInfos[i].size)) {
          std::cout << "pos " << std::hex << addr << " overlaps" << std::endl;
          overlaps = true;
          break;
        }
      }
      if (overlaps) continue;
        
      ifs.seek(addr);
      
      int scriptStart = addr;
      
      ZenkiScriptOpCollection ops;
      
      std::cout << "Disassembling unenumerated script"
        " at " << std::hex << ifs.tell() << std::endl;
      
      ZenkiScriptDism(ifs, ops, dismStruct)();
      
      int scriptEnd = ifs.tell();
      int scriptSize = scriptEnd - scriptStart;
      
      ScriptInfo info;
      info.scriptNum = -1;
      info.addr = scriptStart;
      info.size = scriptSize;
      scriptInfos.push_back(info);
      
      scripts[addr] = ops;
    }
    
  } while (unvisitedFound == true);
  
  // produce final disassembly output
  
  std::ofstream ofs(outprefix + "scripts.txt");
  
  // enumerated script table
  
  ofs << ";==================================" << std::endl;
  ofs << "; Script table" << std::endl;
  ofs << ";==================================" << std::endl << std::endl;
  
  for (int i = 0; i < numScriptPointers; i++) {
    bool found = false;
    for (int j = 0; j < scriptInfos.size(); j++) {
      if ((scriptInfos[j].scriptNum != -1)
          && (scriptInfos[j].scriptNum == i)) {
        int num = scriptInfos[j].scriptNum;
        ofs << ".dw enumScript"
            << TStringConversion::intToString(scriptInfos[j].scriptNum,
                            TStringConversion::baseHex)
                              .substr(2, std::string::npos)
            << std::endl;
        found = true;
        break;
      }
    }
    
    if (!found) {
      ofs << ".dw $0000" << std::endl;
    }
  }
  
  ofs << std::endl;
  
  // data section
  
  ofs << ";==================================" << std::endl;
  ofs << "; Data section" << std::endl;
  ofs << ";==================================" << std::endl << std::endl;
  
  ifs.seek(dataBlockStart);
  for (int i = 0; i < dataBlockSize / 0x10; i++) {
    ofs << ".db ";
    for (int j = 0; j < 0x10; j++) {
      std::string str
        = TStringConversion::intToString(
                          (unsigned int)(unsigned char)ifs.get(),
                          TStringConversion::baseHex)
                            .substr(2, std::string::npos);
      while (str.size() < 2) str = "0" + str;
      ofs << "$"
          << str;
      if (j != 0x0F) ofs << ",";
      else ofs << std::endl;
    }
  }
  ofs << std::endl;
  
  // scripts
  
  for (std::map<int, ZenkiScriptOpCollection>::iterator it = scripts.begin();
       it != scripts.end();
       ++it) {
    int addr = it->first;
    ZenkiScriptOpCollection& script = it->second;
    
    ofs << ";==================================" << std::endl;
    ofs << "; Script " << TStringConversion::intToString(addr,
                            TStringConversion::baseHex)
        << std::endl;
    ofs << ";==================================" << std::endl << std::endl;
    
    for (unsigned int i = 0; i < script.size(); i++) {
      // check if current address is enumerated
      for (unsigned int i = 0; i < scriptInfos.size(); i++) {
        if ((scriptInfos[i].scriptNum != -1)
            && (scriptInfos[i].addr == addr)) {
  //        ofs << ".setEnumeratedScript "
  //            << TStringConversion::intToString(scriptInfos[i].scriptNum,
  //                            TStringConversion::baseHex)
  //            << std::endl << std::endl;
          ofs << "enumScript"
              << TStringConversion::intToString(scriptInfos[i].scriptNum,
                              TStringConversion::baseHex)
                                .substr(2, std::string::npos)
              << ":"
              << std::endl << std::endl;
        }
      }
      
      // check if current address requires a label
      for (ZenkiScriptPointerMap::iterator it
             = dismStruct.scriptPointers.begin();
           it != dismStruct.scriptPointers.end();
           ++it) {
        int scriptPointer = it->first;
        int scriptAddr = scriptPointerBaseAddr + (scriptPointer % 0x4000);
        if (addr == scriptAddr) {
          ofs << "loc_" << std::hex << scriptAddr << ":" << std::endl;
        }
      }
    
      ZenkiScriptOp& op = script[i];
      
      op.print(ofs);
      
      addr += op.size;
    }
    
    ofs << std::endl;
    ofs << "; end: "
        << TStringConversion::intToString(addr,
                            TStringConversion::baseHex);
    ofs << std::endl << std::endl;
  }
  
}
 
