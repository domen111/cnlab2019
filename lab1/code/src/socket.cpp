#include "common.hpp"
#include "icmp.hpp"
#include "socket.hpp"

#include <string>

#include <cstdint>

extern "C"
{

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>

}

using std::string;

namespace traceroute
{
	IcmpSocket::IcmpSocket( const Ip &hostIp, const uint16_t &hostPort,
			const uint32_t &ttl ) : hostName( ), hostNameValid( false ),
			hostIp( hostIp ), hostPort( hostPort ), ttl( ttl )
	{
		SocketCreate( );
	}

	IcmpSocket::IcmpSocket( const string &hostName, const uint16_t &hostPort,
			const uint32_t &ttl ) : hostName( hostName ),
			hostNameValid( true ), hostIp( Dns( hostName ) ),
			hostPort( hostPort ), ttl( ttl )
	{
		SocketCreate( );
	}

	void IcmpSocket::SocketCreate( )
	{
		socketFd = socket( AF_INET, SOCK_RAW, IPPROTO_ICMP );
		setsockopt( socketFd, IPPROTO_IP, IP_TTL, &ttl, sizeof( ttl ) );
	}

	bool IcmpSocket::Send( const string &content )
	{
		sockaddr_in sockAddr = BuildSockaddr( hostName, hostPort );
		size_t bytes_sent = sendto( socketFd, content.c_str( ),
				content.length( ), 0, reinterpret_cast<sockaddr*>( &sockAddr ),
				sizeof( sockAddr ) );

		return ( bytes_sent == content.length( ) );
	}

	bool IcmpSocket::Send( const ICMP &icmp )
	{
		return Send( icmp.Content( ) );
	}

	bool IcmpSocket::Recv( string &content )
	{
		content.resize( 65536 );

		sockaddr_in sockAddr = BuildSockaddr( hostName, hostPort );
		socklen_t addrLength = sizeof( sockAddr );
		size_t bytes_recv = recvfrom( socketFd, &( content[ 0 ] ), 65536, 0,
				reinterpret_cast<sockaddr*>( &sockAddr ), &addrLength );

		return ( bytes_recv > 0 );
	}

	bool IcmpSocket::Recv( ICMP &icmp )
	{
		string content;
		if( not Recv( content ) )
		{
			return false;
		}
		else
		{
			icmp = ICMP( content );
			return true;
		}
	}
}