#include "HttpExecServer.h"
#include "YRShellExec.h"

#include <utility/DebugLog.h>

void HttpExecServer::exec( const char *p) {
  m_shell->execString( p);
  m_auxBufIndex = 0;
}

void HttpExecServer::startExec( ) {
  m_shell->startExec();
}
void HttpExecServer::endExec( ) {
  m_shell->endExec();
}

bool HttpExecServer::sendExecReply( void) {
  bool rc = true;
  if( m_shell->isAuxQueueInUse() && m_shell->isExec() ) {
    rc = false;
    CircularQBase<char>& q = m_shell->getAuxOutq();
    while( q.valueAvailable()) {
      char c = q.get();
      if( c != '\r' && c != '\n' ) {
        m_auxBuf[ m_auxBufIndex++] = c;
      }
      if( c == '\r' || c == '\n' ||  m_auxBufIndex > (sizeof(m_auxBuf) - 2 ) ) {
        m_auxBuf[ m_auxBufIndex] = '\0';
        bool flag = true;
        for( const char* p = m_auxBuf; flag && *p != '\0'; p++) {
          if( *p != ' ' && *p != '\r' && *p != '\n' && *p != '\t') {
            flag = false;
          }
        }
        if( !flag && m_log != NULL) {
          m_log->print( __FILE__, __LINE__, 4, m_auxBuf, "HttpExecServer_sendExecReply: auxBuf");
        }
        if( c == '\r' || c == '\n' ) {
          m_auxBuf[ m_auxBufIndex++] = c;
        }
        clientWrite( m_auxBuf, m_auxBufIndex);
        m_auxBufIndex = 0;
      }
    }
  }
  return rc;
}
