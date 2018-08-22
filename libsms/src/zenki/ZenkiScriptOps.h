#ifndef ZENKISCRIPTOPS_H
#define ZENKISCRIPTOPS_H


#include "util/TStream.h"
#include "util/TByte.h"
#include "exception/TException.h"
#include <vector>
#include <map>
#include <string>
#include <iostream>

namespace Sms {


struct ZenkiScriptOpId {
  BlackT::TByte opcode;
  const char* name;
  const char* argstring;
  
};

struct ZenkiScriptArg {
  enum Type {
    literal,
    expression
  };
  
  Type type;
  int value;
  std::string expressionValue;
};

struct ZenkiScriptPointerInfo {
  ZenkiScriptPointerInfo();

  bool visited;
  bool isEnumerated;
  int enumNumber;
};

typedef std::map<int, ZenkiScriptPointerInfo> ZenkiScriptPointerMap;

struct ZenkiDismStruct {
  ZenkiScriptPointerMap scriptPointers;
  
};

class ZenkiAsmStruct;
struct ZenkiScriptOp {
  ZenkiScriptOp();

  BlackT::TByte opcode;
  std::vector<ZenkiScriptArg> args;
  int size;
  
  void readOp(BlackT::TStream& src,
              const ZenkiScriptOpId& id,
              ZenkiDismStruct& dismStruct);
  
  void readArgs(BlackT::TStream& src,
                const ZenkiScriptOpId& id,
                ZenkiDismStruct& dismStruct);
  
  void writeOp(BlackT::TStream& dst);
  
  void writeArgs(BlackT::TStream& dst);
  
  void print(std::ostream& ofs);
  
  void parseOp(BlackT::TStream& src,
               ZenkiAsmStruct& asmStruct);
  
  static void parseArg(BlackT::TStream& src,
                ZenkiScriptArg& arg);
};

typedef std::vector<ZenkiScriptOp> ZenkiScriptOpCollection;

class ZenkiOpMatchFailureException : public BlackT::TException {
public:
  ZenkiOpMatchFailureException(const char* nameOfSourceFile__,
                          int lineNum__,
                          const std::string& source__);
protected:
  
};

class ZenkiScriptOps {
public:
  static ZenkiScriptOpId matchOpcode(BlackT::TByte opcode);
  static ZenkiScriptOpId matchOpcode(std::string name);
protected:
  
};


}


#endif
