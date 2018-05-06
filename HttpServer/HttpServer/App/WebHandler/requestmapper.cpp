#include "requestmapper.h"

HttpSessionStore* RequestMapper::sessionStore=0;

RequestMapper::RequestMapper(QObject* parent)
    :HttpRequestHandler(parent)
{
    m_serverHandler.start();
}

RequestMapper::~RequestMapper()
{
    delete sessionStore;
    m_serverHandler.quit();
}

void RequestMapper::service(HttpRequest& request, HttpResponse& response) {
    QByteArray path=request.getPath();
    QByteArray msgBody = request.getBody();

    if (path=="/") {
        if(msgBody == "") {
            mainPageController.sendHTML(response);
        }
        else
            mainPageController.service(request, response);
    }
    else {
        response.setStatus(404,"Not found");
        response.write("The URL is wrong, no such document.");
    }

    //qDebug("RequestMapper: finished request");
}

