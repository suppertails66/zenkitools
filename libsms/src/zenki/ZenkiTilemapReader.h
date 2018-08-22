#ifndef ZENKITILEMAPREADER_H
#define ZENKITILEMAPREADER_H


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


class ZenkiTilemapReader {
public:
  ZenkiTilemapReader(BlackT::TStream& src__,
                  const BlackT::TThingyTable& thingy__,
                  std::string outprefix__);
  
  void operator()();
protected:
  BlackT::TStream& src;
//  BlackT::TStream& dst;
  BlackT::TThingyTable thingy;
  std::ofstream spaceOfs;
  std::ofstream indexOfs;
  std::string outprefix;
  int lineNum;
  
  BlackT::TBufStream currentScriptBuffer;
  int blockStart;
  
  struct PointerStruct {
    int address;
    int slotNum;
  };
  std::vector<PointerStruct> pointers;
  
  void outputNextSymbol(BlackT::TStream& ifs);
  
  bool checkSymbol(BlackT::TStream& ifs, std::string& symbol);
  
  void flushActiveScript();
  
  void processDirective(BlackT::TStream& ifs);
  void processLoadTable(BlackT::TStream& ifs);
  void processAddFreeBlock(BlackT::TStream& ifs);
  void processAddMsgPtr(BlackT::TStream& ifs);
  void processEndMsg(BlackT::TStream& ifs);
  
  void loadThingy(const BlackT::TThingyTable& thingy__);
  
};


}


#endif
