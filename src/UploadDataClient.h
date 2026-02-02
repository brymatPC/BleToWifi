#ifndef UPLOAD_DATA_CLIENT_H_
#define UPLOAD_DATA_CLIENT_H_

#include <utility/Sliceable.h>

class DebugLog;
class WiFiClient;

class UploadDataClient : public Sliceable {
private:
    bool m_connected;
    char m_ip[16];
    unsigned m_port;
    uint8_t m_state;
    bool m_sendRequest;
    char *m_fileToSend;
    unsigned m_fileLength;

    DebugLog *m_log;
    WiFiClient* m_client;

  void changeState( uint8_t newState);
public:
    UploadDataClient();
    virtual ~UploadDataClient();
    virtual const char* sliceName( ) { return "UploadDataClient"; }
    void init( const char *ip, unsigned port, DebugLog* log = NULL);
    virtual void slice( void);
    void sendFile(char *file, unsigned len);
};

#endif // #ifndef UPLOAD_DATA_CLIENT_H_