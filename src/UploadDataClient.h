#ifndef UPLOAD_DATA_CLIENT_H_
#define UPLOAD_DATA_CLIENT_H_

#include <core/Sliceable.h>
#include <Preferences.h>

class NetworkClient;

#define MAX_HEADER_BUF_SIZE 128
#define UDC_IP_LEN 16

class UploadDataClient : public Sliceable {
private:
    static const char s_PREF_NAMESPACE[];

    bool m_connected;
    char m_ip[UDC_IP_LEN];
    unsigned m_port;
    uint8_t m_state;
    bool m_sendRequest;
    char m_headerBuf[MAX_HEADER_BUF_SIZE];
    char *m_routeToSend;
    char *m_fileToSend;
    unsigned m_fileLength;

    NetworkClient* m_client;

  void changeState( uint8_t newState);
  void sendHeader();
public:
    UploadDataClient();
    virtual ~UploadDataClient();
    virtual const char* sliceName( ) { return "UploadDataClient"; }
    void init();
    virtual void slice( void);
    void setup(Preferences &pref);
    void save(Preferences &pref);
    void setHostIp(const char *ip);
    void setHostPort(unsigned port);
    void sendFile(char *route, char *file, unsigned len);
    bool busy();
};

#endif // #ifndef UPLOAD_DATA_CLIENT_H_