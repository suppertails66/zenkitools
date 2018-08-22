#include "zenki/ZenkiStringReader.h"
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

ZenkiStringReader::ZenkiStringReader(
                  BlackT::TStream& src__,
                  BlackT::TStream& dst__,
                  const BlackT::TThingyTable& thingy__,
                  int outputAddr__)
  : src(src__),
    dst(dst__),
    thingy(thingy__),
    outputAddr(outputAddr__),
    lineNum(0),
    currentScriptBuffer(scriptBufferCapacity)
//    blockStart(0)
{
  loadThingy(thingy__);
  dst.seek(outputAddr);
}

void ZenkiStringReader::operator()() {
  try {
    
    while (!src.eof()) {
      std::string line;
      src.getLine(line);
      ++lineNum;
      
      if (line.size() <= 0) continue;
      
      // discard lines containing only ASCII spaces and tabs
  //    bool onlySpace = true;
  //    for (int i = 0; i < line.size(); i++) {
  //      if ((line[i] != ' ')
  //          && (line[i] != '\t')) {
  //        onlySpace = false;
  //        break;
  //      }
  //    }
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
  
  
  int ptrTableStart = outputAddr;
  int dataStart = outputAddr + (messageStrings.size() * 2);
  
  int pos = dataStart;
  for (unsigned int i = 0; i < messageStrings.size(); i++) {
    BlackT::TBufStream& buf = messageStrings[i];
    buf.seek(0);
    
    // write pointer
    dst.seek(ptrTableStart + (i * 2));
    dst.writeu16le((pos % 0x4000) + 0x8000);
    
    // write data
    dst.seek(pos);
    while (!buf.eof()) dst.put(buf.get());
    pos += buf.size();
  }
}
  
void ZenkiStringReader::loadThingy(const BlackT::TThingyTable& thingy__) {
  thingy = thingy__;
}
  
void ZenkiStringReader::outputNextSymbol(TStream& ifs) {
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

void ZenkiStringReader::flushActiveScript() {
  messageStrings.push_back(currentScriptBuffer);
  currentScriptBuffer = BlackT::TBufStream(scriptBufferCapacity);
}
  
bool ZenkiStringReader::checkSymbol(BlackT::TStream& ifs, std::string& symbol) {
  if (symbol.size() > ifs.remaining()) return false;
  
  int startpos = ifs.tell();
  for (int i = 0; i < symbol.size(); i++) {
    if (symbol[i] != ifs.get()) {
      ifs.seek(startpos);
      return false;
    }
  }
  
  return true;
}
  
void ZenkiStringReader::processDirective(BlackT::TStream& ifs) {
  TParse::skipSpace(ifs);
  
  std::string name = TParse::matchName(ifs);
  TParse::matchChar(ifs, '(');
  
  for (int i = 0; i < name.size(); i++) {
    name[i] = toupper(name[i]);
  }
  
  if (name.compare("LOADTABLE") == 0) {
    processLoadTable(ifs);
  }
  else if (name.compare("END") == 0) {
    processEnd(ifs);
  }
  else {
    throw TGenericException(T_SRCANDLINE,
                            "ZenkiStringReader::processDirective()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Unknown directive: "
                              + name);
  }
  
  TParse::matchChar(ifs, ')');
}

void ZenkiStringReader::processLoadTable(BlackT::TStream& ifs) {
  std::string tableName = TParse::matchString(ifs);
  TThingyTable table(tableName);
  loadThingy(table);
}

void ZenkiStringReader::processEnd(BlackT::TStream& ifs) {
  // write string terminator
  currentScriptBuffer.put(0x00);
  
  flushActiveScript();
}

/*void ZenkiStringReader::processEndMsgSet(BlackT::TStream& ifs) {
  flushActiveScript();
}

void ZenkiStringReader::processEndMsgSetGroup(BlackT::TStream& ifs) {
  int nummessageStrings = messageStrings.size();
  int quantityTableAddr = dst.tell();
  int addrTableAddr = quantityTableAddr + (nummessageStrings * 1);
  int messagesAddr = addrTableAddr + (nummessageStrings * 2);
  
  int putpos = messagesAddr;
  for (unsigned int i = 0; i < messageStrings.size(); i++) {
    
    BlackT::TBufStream& stream = messageStrings[i];
    int numMessages = stream.size() / bytesPerTextBox;
    int messageStartAddr = putpos + (numMessages * 2);
    
    // write count of messages to quantity table
    dst.seek(quantityTableAddr + (i * 1));
    dst.writeu8(numMessages);
    
    // write pointer to submessage pointer table
    dst.seek(addrTableAddr + (i * 2));
    dst.writeu16le((putpos % 0x4000) + 0x8000);
    
    // write pointer table for submessages
    dst.seek(putpos);
    for (int i = 0; i < numMessages; i++) {
      dst.writeu16le(((messageStartAddr + (i * bytesPerTextBox))
                       % 0x4000) + 0x8000);
    }
    
    // write messages
    stream.seek(0);
    while (!stream.eof()) dst.put(stream.get());
    putpos = dst.tell();
  }
  
  int endpos = dst.tell();
  
  dst.seek(endpos);
  
  // Clear all data in preparation for next set
  messageStrings.clear();
//  quantityPointers.clear();
//  messagePointers.clear();
} */


}
