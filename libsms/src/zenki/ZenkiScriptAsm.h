#ifndef ZENKISCRIPTASM_H
#define ZENKISCRIPTASM_H


#include "util/TStream.h"
#include "util/TBufStream.h"
#include "zenki/ZenkiScriptOps.h"
#include <map>
#include <vector>
#include <list>
#include <string>
#include <iostream>

namespace Sms {

struct ZenkiAsmExpression {
  enum DataType {
    none,
    oneByte,
    twoByte
  };

  int addr;
  DataType dataType;
  std::string expression;
};

typedef std::map<std::string, int> DefinitionMap;
typedef std::list<ZenkiAsmExpression> PendingExpressionList;

struct ZenkiAsmSection {
  
  ZenkiAsmSection();
  
  BlackT::TBufStream data;
  PendingExpressionList pendingExpressions;
  DefinitionMap labels;
//  ZenkiScriptOpCollection data;
};

struct ZenkiAsmStruct {
//  ZenkiAsmStruct();
  
  ZenkiAsmSection activeSection;
  DefinitionMap definitions;
  PendingExpressionList pendingDefinitions;
};

class ZenkiScriptAsm {
public:
  ZenkiScriptAsm(BlackT::TStream& src__,
//                  ZenkiScriptOpCollection& dst__,
                  ZenkiAsmStruct& asmStruct__,
                  std::string filename__ = std::string(""));
  
  virtual void operator()();
protected:
  BlackT::TStream& src;
//  ZenkiScriptOpCollection& dst;
  ZenkiAsmStruct& asmStruct;
  
  std::string filename;
  
  int lineNum;
  
  void processDirective(BlackT::TStream& ifs, std::string name);
  
  void processDbDw(BlackT::TStream& ifs, int size);
  void processDefine(BlackT::TStream& ifs);
  void processInclude(BlackT::TStream& ifs);
};

}


#endif
