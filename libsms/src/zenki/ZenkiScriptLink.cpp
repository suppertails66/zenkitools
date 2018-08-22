#include "zenki/ZenkiScriptLink.h"
#include "util/TStringConversion.h"
#include "util/TParse.h"
#include "exception/TException.h"
#include "exception/TGenericException.h"
#include <iostream>

using namespace BlackT;

namespace Sms {


ZenkiScriptLink::ZenkiScriptLink(ZenkiAsmStruct& asmStruct__,
                               BlackT::TStream& dst__,
                               int loadAddr__)
  : asmStruct(asmStruct__),
    dst(dst__),
    loadAddr(loadAddr__) {
  
}

void ZenkiScriptLink::operator()() {
  try {
  
    int baseRomAddr = dst.tell();
    
    // copy unlinked section to output
    TBufStream& section = asmStruct.activeSection.data;
    section.seek(0);
    for (int i = 0; i < section.size(); i++) {
      dst.put(section.get());
    }
    
    // resolve labels using provided load address
//    for (DefinitionMap::iterator it
//          = asmStruct.activeSection.labels.begin();
//         it != asmStruct.activeSection.labels.end();
//         ++it) {
//      asmStruct.definitions[it->first] = it->second + loadAddr;
//    }
    
    // evaluate pending expressions
    for (PendingExpressionList::iterator it
          = asmStruct.activeSection.pendingExpressions.begin();
         it != asmStruct.activeSection.pendingExpressions.end();
         ++it) {
//      std::cout << std::hex << it->addr << " " << it->dataType
//                << " " << it->expression << std::endl;
      
      std::string expr = it->expression;
      if (expr.size() == 0) {
        throw TGenericException(T_SRCANDLINE,
                                "ZenkiScriptLink::operator()()",
                                "Null expression");
      }
      
      int value;
      
      // bank number
      bool isBankNum = false;
      if (expr[0] == ':') {
        isBankNum = true;
        expr = expr.substr(1, std::string::npos);
      }
      
      // label
      if (asmStruct.activeSection.labels.find(expr) !=
            asmStruct.activeSection.labels.end()) {
        value = asmStruct.activeSection.labels.at(expr) + loadAddr;
        
        isBankNum ? (value = value / 0x4000)
          : (value = (value % 0x4000) + 0x8000);
      }
      // standard definition
      else if (asmStruct.definitions.find(expr)
                != asmStruct.definitions.end()) {
        value = asmStruct.definitions.at(expr);
        
//        isBankNum ? (value = value / 0x4000);
        if (isBankNum) {
          throw TGenericException(T_SRCANDLINE,
                                  "ZenkiScriptLink::operator()()",
                                  "Tried to take banknum of standard"
                                  " definition '"
                                  + expr
                                  + "'");
        }
      }
      else {
        throw TGenericException(T_SRCANDLINE,
                                "ZenkiScriptLink::operator()()",
                                "Undefined symbol: '"
                                + expr
                                + "'");
      }
      
      dst.seek(baseRomAddr + it->addr);
      
      switch (it->dataType) {
      case ZenkiAsmExpression::oneByte:
        dst.writeu8(value);
        break;
      case ZenkiAsmExpression::twoByte:
        dst.writeu16le(value);
        break;
      default:
        throw TGenericException(T_SRCANDLINE,
                                "ZenkiScriptLink::operator()()",
                                "Illegal data type: '"
                              + TStringConversion::intToString(it->dataType));
        break;
      }
    }
    
  }
  catch (BlackT::TException& e) {
    std::cerr << "  Link error: "
              << e.what() << std::endl;
//              << "  Source: " << e.source() << std::endl;
    throw e;
  }
  catch (std::exception& e) {
    std::cerr << "  Unhandled link exception: "
              << e.what() << std::endl;
    throw e;
  }
}


}

 
