#include "TelstraWeb.h"

/* ############################## TelstraWeb ############################ */
TelstraWeb::TelstraWeb(Connection4G* conn, TelstraM1Device* IoTDevice) {
	this->conn = conn;
	this->IoTDevice = IoTDevice;
}
void TelstraWeb::setHost(const char* _host, const char* _path, const int _port) {
	this->host = _host;
    this->path = _path;
    this->port = _port;
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
        reportln("<TelstraWeb:post> - No response received!");
        exitcode = -2;
    } else if(exitcode == 201) {
        reportln("<TelstraWeb:post> - Received valid HTTP response!");
        exitcode = 1;
    } else if (exitcode > 0) {
        report("<TelstraWeb:post> - Received invalid response code - HTTP ");
        reportln(exitcode);
       exitcode *= -1;
    }
    return exitcode;
}
