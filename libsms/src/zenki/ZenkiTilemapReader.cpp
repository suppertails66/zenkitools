#include "zenki/ZenkiTilemapReader.h"
#include "util/TBufStream.h"
#include "util/TStringConversion.h"
#include "util/TParse.h"
#include "exception/TGenericException.h"
#include <cctype>
#include <algorithm>
#include <string>
#include <iostream>

using namespace BlackT;

namespace Sms {


const static int scriptBufferCapacity = 0x400;

ZenkiTilemapReader::ZenkiTilemapReader(
                  BlackT::TStream& src__,
                  const BlackT::TThingyTable& thingy__,
                  std::string outprefix__)
  : src(src__),
    thingy(thingy__),
    outprefix(outprefix__),
    lineNum(0),
    currentScriptBuffer(scriptBufferCapacity),
    blockStart(0) {
  loadThingy(thingy__);
  spaceOfs.open((outprefix + "msg_space.txt").c_str());
  indexOfs.open((outprefix + "msg_index.txt").c_str());
}

void ZenkiTilemapReader::operator()() {
  try {
    while (!src.eof()) {
      std::string line;
      src.getLine(line);
      ++lineNum;
      
      if (line.size() <= 0) continue;
      
      // discard lines containing only ASCII spaces and tabs
      bool onlySpace = true;
      for (int i = 0; i < line.size(); i++) {
        if ((line[i] != ' ')
            && (line[i] != '\t')) {
          onlySpace = false;
          break;
        }
      }
  //    if (onlySpace) continue;
      
      TBufStream ifs(line.size());
      ifs.write(line.c_str(), line.size());
      ifs.seek(0);
      
      // check for comments
      if ((ifs.size() >= 2)
          && (ifs.peek() == '/')) {
        ifs.get();
        if (ifs.peek() == '/') continue;
        else ifs.unget();
      }
      
      // check for special stuff
      if (ifs.peek() == '#') {
        // directives
        ifs.get();
        processDirective(ifs);
        continue;
      }
      
      while (!ifs.eof()) {
        outputNextSymbol(ifs);
      }
    }
  }
  catch (BlackT::TException& e) {
    std::cerr << "  Line " << lineNum << ":" << std::endl
              << "    Error: "
              << e.what() << std::endl;
//              << "  Source: " << e.source() << std::endl;
    throw e;
  }
  catch (std::exception& e) {
    std::cerr << "  Line " << lineNum << ":" << std::endl
              << "    Unhandled exception: "
              << e.what() << std::endl;
    throw e;
  }
}
  
void ZenkiTilemapReader::loadThingy(const BlackT::TThingyTable& thingy__) {
  thingy = thingy__;
}
  
void ZenkiTilemapReader::outputNextSymbol(TStream& ifs) {
  // literal value
  if ((ifs.remaining() >= 5)
      && (ifs.peek() == '<')) {
    int pos = ifs.tell();
    
    ifs.get();
    if (ifs.peek() == '$') {
      ifs.get();
      std::string valuestr = "0x";
      valuestr += ifs.get();
      valuestr += ifs.get();
      
      if (ifs.peek() == '>') {
        ifs.get();
        int value = TStringConversion::stringToInt(valuestr);
        
//        dst.writeu8(value);
        currentScriptBuffer.writeu8(value);

        return;
      }
    }
    
    // not a literal value
    ifs.seek(pos);
  }
  
  TThingyTable::IdLengthPair nextChar = thingy.matchTableEntry(ifs);
  
  if (nextChar.first != -1) {
    int symbolSize;
    if (nextChar.first <= 0xFF) symbolSize = 1;
    else if (nextChar.first <= 0xFFFF) symbolSize = 2;
    else if (nextChar.first <= 0xFFFFFF) symbolSize = 3;
    else symbolSize = 4;
    
    currentScriptBuffer.writeInt(nextChar.first, symbolSize,
        EndiannessTypes::big, SignednessTypes::nosign);
    
    return;
  }
  
  std::string remainder;
  ifs.getLine(remainder);
  
  // if we reached end of file, this is not an error: we're done
  if (ifs.eof()) return;
  
  throw TGenericException(T_SRCANDLINE,
                          "ZenkiStringReader::outputNextSymbol()",
                          "Line "
                            + TStringConversion::intToString(lineNum)
                            + ":\n  Couldn't match symbol at: '"
                            + remainder
                            + "'");
}
  
void ZenkiTilemapReader::flushActiveScript() {
  int outputSize = currentScriptBuffer.size();
  int outputPos = blockStart;
  
  // output message binary
  std::string blockStartString = TStringConversion::intToString(blockStart,
                 TStringConversion::baseHex);
  currentScriptBuffer.seek(0);
  std::string filename =
    outprefix
    + std::string("msg_")
    + blockStartString
    + ".bin";
  TOfstream ofs(filename.c_str(), std::ios_base::binary);
  ofs.write(currentScriptBuffer.data().data(), currentScriptBuffer.size());
  
  // output index entry
  indexOfs << "#******************************************************************************"
    << std::endl;
  indexOfs << "# Message " << blockStartString
    << std::endl;
  indexOfs << "#******************************************************************************"
    << std::endl;
  indexOfs << "[Message" << blockStartString << "]" << std::endl;
  indexOfs << "source=" << filename << std::endl;
  indexOfs << "origPos=" << blockStartString << std::endl;
  // do not allow overwriting at original location -- force inserter to
  // allocate from free space (since we have specifically freed all the
  // strings we built)
  indexOfs << "origSize=0" << std::endl;
  
  int slotNum = 2;
  if (pointers.size() > 0) {
    // there should always be at least one pointer, and all of them should
    // use the same slot
    
    slotNum = pointers[0].slotNum;
  }
  
  if (slotNum == -1) {
//    throw TGenericException(T_SRCANDLINE,
//                            "ZenkiTilemapReader::flushActiveScript()",
//                            "Line "
//                              + TStringConversion::intToString(lineNum)
//                              + ":\n  Undefined slot number");
  }
  else {
    indexOfs << "slot=" << slotNum << std::endl;
    
    indexOfs << "pointers=";
    for (int i = 0; i < pointers.size(); i++) {
      indexOfs << TStringConversion::intToString(pointers[i].address,
                   TStringConversion::baseHex);
      if (i != (pointers.size() - 1)) indexOfs << " ";
    }
    indexOfs << std::endl;
  }
  
  // clear script buffer
  currentScriptBuffer = TBufStream(scriptBufferCapacity);
  // reset position
  blockStart = -1;
  // clear pointers
  pointers.clear();
}
  
void ZenkiTilemapReader::processDirective(BlackT::TStream& ifs) {
  TParse::skipSpace(ifs);
  
  std::string name = TParse::matchName(ifs);
  TParse::matchChar(ifs, '(');
  
  for (int i = 0; i < name.size(); i++) {
    name[i] = toupper(name[i]);
  }
  
  if (name.compare("LOADTABLE") == 0) {
    processLoadTable(ifs);
  }
//  else if (name.compare("STARTBLOCK") == 0) {
//    processStartBlock(ifs);
//  }
  else if (name.compare("SETMSGLOC") == 0) {
    processAddFreeBlock(ifs);
  }
  else if (name.compare("MSGPTR") == 0) {
    processAddMsgPtr(ifs);
  }
  else if (name.compare("ENDMSG") == 0) {
    processEndMsg(ifs);
  }
  else {
    throw TGenericException(T_SRCANDLINE,
                            "ZenkiTilemapReader::processDirective()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Unknown directive: "
                              + name);
  }
  
  TParse::matchChar(ifs, ')');
}

void ZenkiTilemapReader::processLoadTable(BlackT::TStream& ifs) {
  std::string tableName = TParse::matchString(ifs);
  TThingyTable table(tableName);
  loadThingy(table);
}

void ZenkiTilemapReader::processAddFreeBlock(BlackT::TStream& ifs) {
  // argument 1: address of block we want to put data in
  int freeBlockPos = TParse::matchInt(ifs);
  TParse::matchChar(ifs, ',');
  // argument 2: size of destination block
  int freeBlockRemaining = TParse::matchInt(ifs);
  
  blockStart = freeBlockPos;
  
  spaceOfs << "[" << TStringConversion::intToString(freeBlockPos,
                        TStringConversion::baseHex) << "]"
    << std::endl;
  
  spaceOfs << "size=" << TStringConversion::intToString(freeBlockRemaining,
                        TStringConversion::baseHex)
    << std::endl << std::endl;
}

void ZenkiTilemapReader::processAddMsgPtr(BlackT::TStream& ifs) {
  PointerStruct p;
  // argument 1: address of pointer we want to put data in
  p.address = TParse::matchInt(ifs);
  TParse::matchChar(ifs, ',');
  // argument 2: slot number through which pointer is accessed
  p.slotNum = TParse::matchInt(ifs);
  pointers.push_back(p);
}

void ZenkiTilemapReader::processEndMsg(BlackT::TStream& ifs) {
  flushActiveScript();
}


}
