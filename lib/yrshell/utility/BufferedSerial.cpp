#include "BufferedSerial.h"
#include "YRShellInterpreter.h"

BufferedSerial::BufferedSerial( HardwareSerial* hs) {
	m_hs = hs;
}
#ifdef HWCDC_SERIAL_IS_DEFINED
BufferedSerial::BufferedSerial( HWCDC* hwcdc) {
	m_hs = nullptr;
	m_hwcdc = hwcdc; 
}
#endif
void BufferedSerial::init(  CircularQBase<char>& nq, CircularQBase<char>& pq) {
	m_nextQ = &nq;
	m_previousQ = &pq;
}
void BufferedSerial::slice( void) {
	int c;
	if( m_nextQ != NULL) {
		while( m_nextQ->spaceAvailable()) {
#ifdef HWCDC_SERIAL_IS_DEFINED
			c = m_hwcdc->read();
#else
			c = m_hs->read();
#endif
			if( c == -1) {
				break;
			}
			m_nextQ->put( c);
		}
	}	
	if( m_previousQ != NULL) {
#ifdef HWCDC_SERIAL_IS_DEFINED
		while( m_previousQ->valueAvailable() && m_hwcdc->availableForWrite() > 0) {
			m_hwcdc->write( m_previousQ->get());
		}
#else
		while( m_previousQ->valueAvailable() && m_hs->availableForWrite() > 0) {
			m_hs->write( m_previousQ->get());
		}
#endif
	}
}
#ifdef HWCDC_SERIAL_IS_DEFINED
void BufferedSerial::begin( uint32_t baud) {
	if(m_hs) {
		m_hs->begin( baud);
	} else if(m_hwcdc) {
		m_hwcdc->begin( baud);
	}
}
void BufferedSerial::end( void) {
	if(m_hs) {
		m_hs->end( );
	} else if(m_hwcdc) {
		m_hwcdc->end( );
	}
}
void BufferedSerial::setBaud( uint32_t baud) {
	if(m_hs) {
		m_hs->end( );
		m_hs->begin( baud);
	} else if(m_hwcdc) {
		m_hwcdc->end( );
		m_hwcdc->begin( baud);
	}
}
#else
void BufferedSerial::begin( uint32_t baud) {
	m_hs->begin( baud);
}
void BufferedSerial::end( void) {
	m_hs->end( );
}
void BufferedSerial::setBaud( uint32_t baud) {
	m_hs->end( );
	m_hs->begin( baud);
}
#endif
BufferedSerial BSerial( &Serial);
#ifdef ENABLE_SERIAL1
BufferedSerial BSerial1( &Serial1);  
#endif
#ifdef ENABLE_SERIAL2
BufferedSerial BSerial2( &Serial2);  
#endif
#ifdef ENABLE_SERIAL3
BufferedSerial BSerial3( &Serial3);  
#endif

