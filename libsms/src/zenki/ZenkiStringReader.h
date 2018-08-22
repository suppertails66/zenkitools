#ifndef ZENKISTRINGREADER_H
#define ZENKISTRINGREADER_H


#include "util/TStream.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TBufStream.h"
#include "util/TThingyTable.h"
#include <string>
#include <vector>
#include <map>
#include <fstream>

namespace Sms {


class ZenkiStringReader {
public:
  ZenkiStringReader(BlackT::TStream& src__,
                  BlackT::TStream& dst__,
                  const BlackT::TThingyTable& thingy__,
                  int outputAddr__);
  
  void operator()();
protected:
  struct ThingyValueAndKey {
    std::string value;
    int key;
    
    bool operator<(const ThingyValueAndKey& src) const {
      return (value.size() > src.value.size());
    }
  };

  BlackT::TStream& src;
  BlackT::TStream& dst;
  BlackT::TThingyTable thingy;
  int outputAddr;
//  std::vector<ThingyValueAndKey> thingiesBySize;
  int lineNum;
  
  std::vector<BlackT::TBufStream> messageStrings;
  BlackT::TBufStream currentScriptBuffer;
  
//  std::vector<int> quantityPointers;
//  std::vector<int> messagePointers;
  
//  BlackT::TBufStream currentScriptBuffer;
//  int blockRemaining;
//  bool blockIsInFixedBank;
//  int currentPointerTablePos;
//  int freeBlockPos;
//  int freeBlockRemaining;
//  bool freeBlockIsInFixedBank;
//  int blockStart;
  
//  struct PointerStruct {
//    int address;
//    int slotNum;
//  };
//  std::vector<PointerStruct> pointers;
//  std::map<std::string, int> labels;
//  std::map<int, std::string> pendingJumps;
  
  void outputNextSymbol(BlackT::TStream& ifs);
  
  bool checkSymbol(BlackT::TStream& ifs, std::string& symbol);
  
  void flushActiveScript();
  
  void processDirective(BlackT::TStream& ifs);
  void processLoadTable(BlackT::TStream& ifs);
  void processEnd(BlackT::TStream& ifs);
//  void processSetAddr(BlackT::TStream& ifs);
//  void processEndMsgSet(BlackT::TStream& ifs);
//  void processEndMsgSetGroup(BlackT::TStream& ifs);
//  void processAddQuantityPtr(BlackT::TStream& ifs);
//  void processAddMessagePtr(BlackT::TStream& ifs);
  
  void loadThingy(const BlackT::TThingyTable& thingy__);
  
};


}


#endif
