#ifndef YRShellExec_h
#define YRShellExec_h

#include <YRShell.h>

class YRShellExec {
public:
  virtual inline bool isAuxQueueInUse( void) = 0;
  virtual CircularQBase<char>& getAuxOutq(void) = 0;
  virtual void startExec( void) = 0;
  virtual void endExec( void) = 0;
  virtual void execString( const char* p) = 0;
  virtual bool isExec( void) = 0;
};

#endif