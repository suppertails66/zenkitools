#include "zenki/ZenkiScriptDism.h"
#include "util/TStringConversion.h"
#include <iostream>

using namespace BlackT;

namespace Sms {


ZenkiScriptDism::ZenkiScriptDism(BlackT::TStream& src__,
                  ZenkiScriptOpCollection& dst__,
                  ZenkiDismStruct& dismStruct__)
  : src(src__),
    dst(dst__),
    dismStruct(dismStruct__) {
  
}
  
void ZenkiScriptDism::operator()() {
  while (true) {
    
 //   std::cerr << src.tell() << std::endl;
    ZenkiScriptOpId id;
    try {
      id = ZenkiScriptOps::matchOpcode(src.peek());
    }
    catch (ZenkiOpMatchFailureException& e) {
      std::cerr << "Error: Unknown opcode "
        << TStringConversion::intToString(src.peek(),
              TStringConversion::baseHex)
        << " at "
        << TStringConversion::intToString(src.tell(),
              TStringConversion::baseHex) << std::endl;
      throw e;
    }
    
    std::cout << std::hex << src.tell();
    std::cout << " " << std::hex << (int)id.opcode << std::endl;
    
    ZenkiScriptOp op;
    op.readOp(src, id, dismStruct);
    dst.push_back(op);
    
    // check script termination conditions
    // 00 = end of script
    // 0B = jump
    // 18 = ret from local call
    // ...
    if ((id.opcode == 0x00)
        || (id.opcode == 0x0B)
        || (id.opcode == 0x18)) {
      break;
    }
  }
}


}

 
