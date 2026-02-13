#include "HttpExecServer.h"
#include "YRShellExec.h"

void HttpExecServer::exec( const char *p) {
  if( m_shell) {
    m_shell->execString( p);
  }
  m_auxBufIndex = 0;
}

void HttpExecServer::startExec( ) {
  if( m_shell) {
    m_shell->startExec();
  }
}
void HttpExecServer::endExec( ) {
  if( m_shell) {
    m_shell->endExec();
  }
}

bool HttpExecServer::sendExecReply( void) {
  bool rc = true;
  if( m_shell && m_shell->isAuxQueueInUse() && m_shell->isExec() ) {
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
