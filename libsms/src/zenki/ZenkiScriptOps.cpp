#include "zenki/ZenkiScriptOps.h"
#include "zenki/ZenkiScriptAsm.h"
#include "zenki/ZenkiScriptDism.h"
#include "util/TStringConversion.h"
#include "util/TParse.h"
#include "exception/TGenericException.h"
#include <cstring>

using namespace BlackT;

namespace Sms {


const ZenkiScriptOpId zenkiScriptOps[] = {
  { 0x00, "terminate", "" },
  { 0x01, "sendTilemap", "221111" }, // if high bit of param 4 set,
                                     // fullwidth tilemap
                                     // if unset, param 4 is high byte
                                     // of halfwidth tilemap
  { 0x02, "op02", "2111" },
  { 0x03, "op03", "221111" },
  { 0x04, "op04", "221111" },
  { 0x05, "vramFill", "222" },
  { 0x06, "waitFrames", "1" },
  { 0x07, "playSound0", "1" },
  { 0x08, "playSound1", "1" },
  { 0x09, "refresh", "" },
  { 0x0A, "print", "11" },
  { 0x0B, "jump", "S" },
  { 0x0C, "writeByte", "21" },
  { 0x0D, "writeWord", "22" },
  { 0x0E, "op0E", "11" },
  { 0x0F, "op0F", "111" },
  { 0x10, "op10", "1" },
  { 0x11, "op11", "1" },
  { 0x12, "loadPsGraphic", "221" },
  { 0x13, "op13", "112" },
  { 0x14, "setColor", "22" },
  { 0x15, "op15", "1" },
  { 0x16, "op16", "1" },
  { 0x17, "call", "S" },
  { 0x18, "ret", "" },
  { 0x19, "setLoopCount", "1" },
  { 0x1A, "loop", "S" },
  { 0x1B, "op1B", "11" },
  { 0x1C, "op1C", "21" },
  { 0x1D, "op1D", "" },
  { 0x1E, "op1E", "21" },
  { 0x1F, "op1F", "22" },
  { 0x20, "op20", "21" },
  { 0x21, "op21", "" },
  { 0x22, "ifEq", "21S" },
  { 0x23, "and", "21" },
  { 0x24, "or", "21" },
  { 0x25, "promptCursor", "" },
  { 0x26, "op26", "1" },
  { 0x27, "op27", "" },
  { 0x28, "op28", "1" },
  { 0x29, "op29", "2211" },
  { 0x2A, "op2A", "" },
  { 0x2B, "op2B", "" },
  { 0x2C, "op2C", "" },
};

const int numZenkiScriptOps
  = sizeof(zenkiScriptOps) / sizeof(ZenkiScriptOpId);

ZenkiScriptPointerInfo::ZenkiScriptPointerInfo()
  : visited(false),
    isEnumerated(false),
    enumNumber(-1) { };

ZenkiScriptOp::ZenkiScriptOp()
  : opcode(0),
    size(1) { };

void ZenkiScriptOp::readOp(BlackT::TStream& src,
                           const ZenkiScriptOpId& id,
                           ZenkiDismStruct& dismStruct) {
  opcode = src.get();
  readArgs(src, id, dismStruct);
}
  
void ZenkiScriptOp::readArgs(BlackT::TStream& src,
                             const ZenkiScriptOpId& id,
                             ZenkiDismStruct& dismStruct) {
  for (unsigned int i = 0; i < strlen(id.argstring); i++) {
    char c = id.argstring[i];
    
    ZenkiScriptArg arg;
    arg.type = ZenkiScriptArg::literal;
    
    switch (c) {
    // 1-byte int
    case '1':
      arg.value = src.readu8();
      size += 1;
      break;
    // 2-byte int
    case '2':
      
      // if a writeWord op (0D) is used to write to location C90F,
      // it's setting the script pointer to jump to if the
      // user cancels the scene with start (or just if the script ends?)
      // seems to only be used once, for pre-title screen
      if ((opcode == 0x0D)
          && (i == 1)
          && ((args[0].value == 0xC90F))) {
        goto scriptP;
      }
    
      arg.value = src.readu16le();
      size += 2;
      break;
    // script pointer
    case 'S':
    scriptP:
      arg.value = src.readu16le();
      // add to script pointer list if not on it
      if ((dismStruct.scriptPointers.find(arg.value)
            == dismStruct.scriptPointers.end())) {
        dismStruct.scriptPointers[arg.value] = ZenkiScriptPointerInfo();
      }
      size += 2;
      break;
    default:
      throw TGenericException(T_SRCANDLINE,
                              "ZenkiScriptOp::readArgs()",
                              std::string("Unknown arg symbol: ")
                                + c);
      break;
    }
    
    args.push_back(arg);
  }
}

void ZenkiScriptOp::writeOp(BlackT::TStream& dst) {
  dst.put(opcode);
  writeArgs(dst);
}

void ZenkiScriptOp::writeArgs(BlackT::TStream& dst) {
  ZenkiScriptOpId id = ZenkiScriptOps::matchOpcode(opcode);
  
  for (unsigned int i = 0; i < strlen(id.argstring); i++) {
    char c = id.argstring[i];
    
    ZenkiScriptArg arg = args.at(i);
    
    switch (c) {
    // 1-byte int
    case '1':
      dst.writeu8(arg.value);
      break;
    // 2-byte int
    case '2':
    case 'S':
      dst.writeu16le(arg.value);
      break;
    default:
      throw TGenericException(T_SRCANDLINE,
                              "ZenkiScriptOp::writeArgs()",
                              std::string("Unknown arg symbol: ")
                                + c);
      break;
    }
  }
}

std::string asStringLiteral(std::ostream& ofs, int value, int digits = -1) {
  std::string str
    = TStringConversion::intToString(value,
        TStringConversion::baseHex).substr(2, std::string::npos);
  
  if (digits != -1) {
    while (str.size() < digits) str = "0" + str;
  }
  
  str = "$" + str;
  
  return str;
}

void ZenkiScriptOp::print(std::ostream& ofs) {
  ZenkiScriptOpId id = ZenkiScriptOps::matchOpcode(opcode);
  
  std::string namestr = std::string(id.name);
  while (namestr.size() < 16) namestr += ' ';
  ofs << "  " << namestr << " ";
  
  for (unsigned int i = 0; i < strlen(id.argstring); i++) {
    char c = id.argstring[i];
    
    ZenkiScriptArg arg = args[i];
    
    switch (c) {
    // 1-byte int
    case '1':
      ofs << asStringLiteral(ofs, arg.value, 2);
      break;
    // 2-byte int
    case '2':
      
      // if a writeWord op (0D) is used to write to location C90F,
      // it's setting the script pointer to jump to if the
      // user cancels the scene with start (or just if the script ends?)
      if ((opcode == 0x0D)
          && (i == 1)
          && ((args[0].value == 0xC90F))) {
        goto scriptP;
      }
      
      ofs << asStringLiteral(ofs, arg.value, 4);
      break;
    // script pointer
    case 'S':
    scriptP:
//      ofs << "loc_" << asStringLiteral(arg.value, 4)
//                        .substr(1, std::string::npos);
      ofs << "loc_" << std::hex << (arg.value % 0x4000) + 0x14000;
      break;
    default:
      throw TGenericException(T_SRCANDLINE,
                              "ZenkiScriptOp::print()",
                              std::string("Unknown arg symbol: ")
                                + c);
      break;
    }
    
    if (i != (strlen(id.argstring) - 1)) {
      ofs << ",";
    }
  }
  ofs << std::endl;
}

void ZenkiScriptOp::parseOp(BlackT::TStream& src,
                            ZenkiAsmStruct& asmStruct) {
  std::string namestr = TParse::getSpacedSymbol(src);
  
  ZenkiScriptOpId id;
  try {
    id = ZenkiScriptOps::matchOpcode(namestr);
  }
  catch (ZenkiOpMatchFailureException& e) {
    throw TGenericException(T_SRCANDLINE,
                            "ZenkiScriptOp::parseOp()",
                            std::string("Unknown operation: ")
                              + namestr);
  }
  
  opcode = id.opcode;
  
  for (unsigned int i = 0; i < strlen(id.argstring); i++) {
    char c = id.argstring[i];
    
    ZenkiScriptArg arg;
    parseArg(src, arg);
    
//    std::cout << arg.type << " " << arg.value << " " << arg.expressionValue << std::endl;
    
    switch (c) {
    // 1-byte int
    case '1':
      if (arg.type == ZenkiScriptArg::expression) {
        ZenkiAsmExpression expr;
        expr.addr = asmStruct.activeSection.data.tell() + size;
        expr.dataType = ZenkiAsmExpression::oneByte;
        expr.expression = arg.expressionValue;
        asmStruct.activeSection.pendingExpressions.push_back(expr);
      }
      size += 1;
      break;
    // 2-byte int
    case '2':
    // script pointer
    case 'S':
      if (arg.type == ZenkiScriptArg::expression) {
        ZenkiAsmExpression expr;
        expr.addr = asmStruct.activeSection.data.tell() + size;
        expr.dataType = ZenkiAsmExpression::twoByte;
        expr.expression = arg.expressionValue;
        asmStruct.activeSection.pendingExpressions.push_back(expr);
      }
      size += 2;
      break;
    default:
      throw TGenericException(T_SRCANDLINE,
                              "ZenkiScriptOp::parseOp()",
                              std::string("Unknown arg symbol: ")
                                + c);
      break;
    }
    
    
    args.push_back(arg);
  }
}

void ZenkiScriptOp::parseArg(BlackT::TStream& src,
                             ZenkiScriptArg& arg) {
  TParse::skipSpace(src);
  int startpos = src.tell();
  
  std::string argstr = TParse::getUntilSpaceOrChars(src, ",");
  if (argstr.size() == 0) {
    throw TGenericException(T_SRCANDLINE,
                            "ZenkiScriptOp::parseArg()",
                            std::string("Unexpected end of input"));
  }
  
  int endpos = src.tell();
  
  // we're not doing any math, so if the string matches an int, assume
  // it's a literal;
  // otherwise, it's an expression (label, etc.)
  
  src.seek(startpos);
  if (TParse::checkInt(src)) {
    arg.type = ZenkiScriptArg::literal;
    arg.value = TParse::matchInt(src);
  }
  else {
    arg.type = ZenkiScriptArg::expression;
    arg.expressionValue = argstr;
  }
  
  src.seek(endpos);
}

ZenkiOpMatchFailureException::ZenkiOpMatchFailureException(
                        const char* nameOfSourceFile__,
                        int lineNum__,
                        const std::string& source__)
  : TException(nameOfSourceFile__, lineNum__, source__) { };

ZenkiScriptOpId ZenkiScriptOps::matchOpcode(BlackT::TByte opcode) {
  for (int i = 0; i < numZenkiScriptOps; i++) {
    if (zenkiScriptOps[i].opcode == opcode) return zenkiScriptOps[i];
  }
  
  throw ZenkiOpMatchFailureException(T_SRCANDLINE,
                                "ZenkiScriptOps::matchOpcode(BlackT::TByte)");
}

ZenkiScriptOpId ZenkiScriptOps::matchOpcode(std::string name) {
  for (int i = 0; i < numZenkiScriptOps; i++) {
    if (strcmp(zenkiScriptOps[i].name, name.c_str()) == 0)
      return zenkiScriptOps[i];
  }
  
  throw ZenkiOpMatchFailureException(T_SRCANDLINE,
                                 "ZenkiScriptOps::matchOpcode(std::string)");
}


}

 
