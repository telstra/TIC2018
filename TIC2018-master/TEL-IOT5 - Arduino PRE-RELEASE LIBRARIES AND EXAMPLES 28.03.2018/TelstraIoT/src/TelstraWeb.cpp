#include "TelstraWeb.h"


/* ############################## TelstraWeb ############################ */
TelstraWeb::TelstraWeb(Connection4G* conn, TelstraM1Device* shield) {
	this->conn = conn;
	this->shield = shield;
}
TelstraWeb::~TelstraWeb(){

}
void TelstraWeb::setHost(const char* _host, const char* _path, const int _port) {
	host = _host;
	path = _path;
    port = _port;
}

HttpRequest* TelstraWeb::setupRequest(const char* method) {
    HttpRequest* request = new HttpRequest(conn, host, path, port, method);
    return request;
}

int TelstraWeb::post(const char* data)
{
	  HttpRequest* request = setupRequest("POST");
    request->write("\r\n");
		request->write(data);
    //request->print();
		HttpResponse* response = request->execute();
    delete request;
    int exitcode = response->getHttpCode();
	delete response;
    if (exitcode == 0) {
		Serial.println("No response received!");
        exitcode = -2;
    } else if(exitcode == 201) {
		Serial.println("Received valid HTTP response!");
        exitcode = 1;
    } else if (exitcode > 0) {
 		Serial.print("Received invalid response code - HTTP ");
 		Serial.println(exitcode);
       exitcode *= -1;
    }
    return exitcode;
}
