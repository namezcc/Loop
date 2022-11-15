#include "ToolFunction.h"
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
namespace as = boost::asio;

std::string getLocalIp()
{
	as::io_context ctx;
	tcp::resolver resolver(ctx);
	tcp::resolver::query query(boost::asio::ip::host_name(), "");
	tcp::resolver::iterator it = resolver.resolve(query);
	for (; it != tcp::resolver::iterator();)
	{
		boost::asio::ip::address addr = (it++)->endpoint().address();
		if (!addr.is_v6())
			return addr.to_string();
	}
	return "";
}
