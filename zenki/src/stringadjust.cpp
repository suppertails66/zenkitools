#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TBufStream.h"
#include "util/TStringConversion.h"
#include "util/TParse.h"
#include <iostream>
#include <string>

using namespace std;
using namespace BlackT;

/*  static void skipSpace(BlackT::TStream& ifs);
  static bool checkString(BlackT::TStream& ifs);
  static bool checkInt(BlackT::TStream& ifs);
  static bool checkChar(BlackT::TStream& ifs, char c);
  static std::string matchString(BlackT::TStream& ifs);
  static int matchInt(BlackT::TStream& ifs);
  static void matchChar(BlackT::TStream& ifs, char c);
  static std::string matchName(BlackT::TStream& ifs);
  
  static std::string getSpacedSymbol(BlackT::TStream& ifs);
  static std::string getRemainingContent(BlackT::TStream& ifs);
  
  static std::string getUntilChars(BlackT::TStream& ifs,
                                   std::string recstring);
  static std::string getUntilSpaceOrChars(BlackT::TStream& ifs,
                                          std::string recstring); */
                                          
int main(int argc, char* argv[]) {
  if (argc < 3) {
    cout << "Kishin Douji Zenki script timing adjuster" << endl;
    cout << "Usage: " << argv[0] << " <infile> <outfile>" << endl;
    
    return 0;
  }
  
  string infile = string(argv[1]);
  string outfile = string(argv[2]);
  
  TIfstream ifs(infile.c_str(), ios_base::binary);
  TOfstream ofs(outfile.c_str(), ios_base::binary);
//  TBufStream ofs(0x100000);
  
  bool printPending = false;
  while (!ifs.eof()) {
    string rawLine;
    ifs.getLine(rawLine);
    TBufStream line(rawLine.size());
    line.write(rawLine.c_str(), rawLine.size());
    line.seek(0);
    
    std::string command = TParse::getSpacedSymbol(line);
    
    if (command.compare("print") == 0) {
      printPending = true;
    }
    else if (printPending
             && (command.compare("waitFrames") == 0)) {
      TParse::skipSpace(line);
      
      int value = TParse::matchInt(line);
      value += 0x28;
      
      string valuestr = "$" + TStringConversion::intToString(value,
                            TStringConversion::baseHex)
                            .substr(2, string::npos);
      string remaining;
      line.getLine(remaining);
      remaining += "\n";
      
      ofs.write(string("  waitFrames       ").c_str(), 19);
      ofs.write(valuestr.c_str(), valuestr.size());
      ofs.write(remaining.c_str(), remaining.size());
      
      printPending = false;
      continue;
    }
    
    line.seek(0);
    ofs.write(rawLine.c_str(), rawLine.size());
    ofs.put('\n');
    
    // check for eof
    ifs.get();
    if (!ifs.good()) break;
    ifs.unget();
  }
}


