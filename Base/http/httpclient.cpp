#include "httpclient.h"

#include "http.h"

HttpClient::HttpClient()
{
	ft_http_init();

}

std::string HttpClient::requestUrl(const char * url)
{
	ft_http_client_t* http = ft_http_new();
	std::string body = ft_http_sync_request(http, url, M_GET);

	ft_http_destroy(http);

	return body;
}
