/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SSLConnection.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: matrus <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/05 10:14:18 by matrus            #+#    #+#             */
/*   Updated: 2021/02/05 10:14:22 by matrus           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SSLConnection.hpp"
#include <string>
#include <stdexcept>
#include <sys/socket.h>
#include "BigLogger.hpp"


SSLConnection::SSLConnection()
{}

SSLConnection::~SSLConnection()
{
	/* todo: delete map */
}

void SSLConnection::init()
{
	_initRng();
	_initCertsAndPkey();
	_initAsServer();
	_listen();
}

void SSLConnection::_initRng()
{
	mbedtls_entropy_init( &_entropy );
	mbedtls_ctr_drbg_init( &_ctrDrbg );
	const std::string seed = "JUST another random seed%^&TYU";
	if (mbedtls_ctr_drbg_seed(&_ctrDrbg,
							   mbedtls_entropy_func,
							   &_entropy,
							   reinterpret_cast<const unsigned char *>(seed.c_str()),
							   seed.size())
		!= 0 )
	{
		throw std::runtime_error("SSL failed to generate drbg seed");
	}
}

void SSLConnection::_initCertsAndPkey() {
	int ret;
	mbedtls_pk_init( &_pkey );
	mbedtls_x509_crt_init( &_serverCert );
	/* todo: remove hardcode */
	ret = mbedtls_x509_crt_parse_file(&_serverCert, "./certs/localhost.crt");
	if (ret != 0) {
		BigLogger::cout(std::string("mbedtls_x509_crt_parse failed") );
		/* todo: catch throw */
		throw std::runtime_error("mbedtls_x509_crt_parse failed");
	}
	/* todo: change nullptr on key password */
	ret = mbedtls_pk_parse_keyfile(&_pkey, "./certs/localhost.key", nullptr);
	if (ret != 0) {
		BigLogger::cout(std::string("mbedtls_pk_parse_key failed") );
		/* todo: catch throw */
		throw std::runtime_error("mbedtls_pk_parse_key failed");
	}
}

void SSLConnection::_initAsServer() {
	mbedtls_ssl_config_init( &_conf );
	if (mbedtls_ssl_config_defaults(&_conf,
									MBEDTLS_SSL_IS_SERVER,
									MBEDTLS_SSL_TRANSPORT_STREAM,
									MBEDTLS_SSL_PRESET_DEFAULT) != 0)
	{
		/* todo: catch throw */
		throw std::runtime_error("SSL setting default configs failed");
	}

	mbedtls_ssl_conf_rng(&_conf, mbedtls_ctr_drbg_random, &_ctrDrbg);
	//	mbedtls_ssl_conf_dbg( &_conf, my_debug, stdout );
	/* here can be debug function, see mbedtls_ssl_conf_dbg() */
	mbedtls_ssl_conf_ca_chain( &_conf, &_serverCert, nullptr );
	if(mbedtls_ssl_conf_own_cert( &_conf, &_serverCert, &_pkey ) != 0 ) {
		BigLogger::cout(std::string("mbedtls_ssl_conf_own_cert failed") );
		/* todo: catch throw */
		throw std::runtime_error("mbedtls_ssl_conf_own_cert failed");
	}
}

void SSLConnection::_listen() {
	const std::string sslPort = "6697"; /* todo: default port by RFC 7194, but by checklist port should be PORT + 1 ??! */
	int ret = 0;
	mbedtls_net_init( &_listener );
	if ((ret = mbedtls_net_bind(&_listener, nullptr, sslPort.c_str(), MBEDTLS_NET_PROTO_TCP)) != 0) {
		if (ret == MBEDTLS_ERR_NET_SOCKET_FAILED)
			throw std::runtime_error("SSL socket() failed");
		else if (ret == MBEDTLS_ERR_NET_BIND_FAILED)
			throw std::runtime_error("SSL bind() failed");
		else if (ret == MBEDTLS_ERR_NET_LISTEN_FAILED)
			throw std::runtime_error("SSL listen() failed");
		else
			throw std::runtime_error("SSL _listen() undefined error");
	}
	if (mbedtls_net_set_nonblock(&_listener) != 0) {
		throw std::runtime_error("Unnable to set SSL socket noblock");
	}
}

socket_type SSLConnection::getListener() const {
	return _listener.fd;
}

ssize_t SSLConnection::send(socket_type fd, const std::string & buff, size_t maxLen)
{
	if (_connections.find(fd) == _connections.end()) {
		BigLogger::cout("Socket trying to send via SSL does not exist", BigLogger::RED);
		return -1;
	}

	int nBytes = mbedtls_ssl_write(&_connections[fd]->sslContext,
							reinterpret_cast<const unsigned char *>(buff.c_str()),
							std::min(buff.size(), maxLen));
	if (nBytes < 0) {
		if (nBytes == MBEDTLS_ERR_SSL_WANT_WRITE) {
			BigLogger::cout("SSL_WANT_WRITE event happen in _ssl.send()", BigLogger::YELLOW);
		}
		else if (nBytes == MBEDTLS_ERR_SSL_WANT_READ) {
			BigLogger::cout("SSL_WANT_READ event happen in _ssl.send()", BigLogger::YELLOW);
		}
		else {
			BigLogger::cout("Undefined error happen in ssl_write()", BigLogger::RED);
			/* todo: reload ssl (how?) */
		}
		return nBytes;
	}
	return nBytes;
}

ssize_t SSLConnection::recv(socket_type fd, unsigned char * buff, size_t maxLen)
{
	if (_connections.find(fd) == _connections.end()) {
		BigLogger::cout("Socket trying to recv via SSL does not exist", BigLogger::RED);
		return -1;
	}

	int nBytes = mbedtls_ssl_read(&_connections[fd]->sslContext, buff, maxLen);
	if (nBytes < 0) {
		if (nBytes == MBEDTLS_ERR_SSL_WANT_READ) {
			BigLogger::cout("SSL_WANT_READ event happen in _ssl.recv()", BigLogger::YELLOW);
		}
		else if (nBytes == MBEDTLS_ERR_SSL_WANT_WRITE) {
			BigLogger::cout("SSL_WANT_WRITE event happen in _ssl.recv()", BigLogger::YELLOW);
		}
		else if (nBytes == EOF) {
			BigLogger::cout("EOF event happen in _ssl.recv()", BigLogger::RED);
			/* todo: ssl_reconnect() ??*/
		}
		return nBytes;
	}
	return nBytes;
}

bool SSLConnection::isSSLSocket(socket_type sock) {
	return sock == _listener.fd || (_connections.find(sock) != _connections.end());
}

socket_type SSLConnection::accept() {
	mbedtls_net_context newContext;
	struct sockaddr_storage		remoteAddr = {};
	socklen_t					addrLen = sizeof(remoteAddr);
	size_t						addrLenWritten = 0;

	int ret = mbedtls_net_accept(&_listener, &newContext, &remoteAddr, addrLen, &addrLenWritten);
	if (ret != 0) {
		if (ret == MBEDTLS_ERR_NET_ACCEPT_FAILED) {
			BigLogger::cout("SSL accept() failed", BigLogger::YELLOW);
		}
		else if (ret == MBEDTLS_ERR_NET_BUFFER_TOO_SMALL) {
			BigLogger::cout("SSL accept() failed: buffer to small o_O", BigLogger::YELLOW);
		}
		else if (ret == MBEDTLS_ERR_SSL_WANT_READ) {
			BigLogger::cout("SSL accept() failed: SSL_WANT_READ event happen", BigLogger::YELLOW);
		}
		return -1;
	}
	try {
		_connections[newContext.fd] = new SSLInfo(newContext, _conf);
	} catch (SSLConnection::SSLInfo::SetupError &) {
		BigLogger::cout("SSL setup error", BigLogger::YELLOW);
		_connections.erase(newContext.fd);
		mbedtls_net_close(&newContext);
		return -1;
	} catch (SSLConnection::SSLInfo::HandshakeError &) {
		BigLogger::cout("SSL handshake error", BigLogger::YELLOW);
		_connections.erase(newContext.fd);
		mbedtls_net_close(&newContext);
		return -1;
	}
	return newContext.fd;
}

SSLConnection::SSLInfo::SSLInfo(const mbedtls_net_context & context,
								mbedtls_ssl_config & conf)
{

	if (mbedtls_ssl_setup(&sslContext, &conf) != 0) {
		throw SetupError();
	}

	mbedtls_ssl_set_bio(&sslContext, &netContext, mbedtls_net_send, mbedtls_net_recv, nullptr);

	int ret;
	while (	(ret = mbedtls_ssl_handshake(&sslContext)) != 0) {
		if (ret == MBEDTLS_ERR_SSL_HELLO_VERIFY_REQUIRED) {
			BigLogger::cout("Handshake hello required!", BigLogger::RED);
		}
		if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE ) {
			BigLogger::cout("Handshake failed!", BigLogger::RED);
			/* todo: reload ssl ?? */
			throw HandshakeError();
		}
	}
}

SSLConnection::SSLInfo::~SSLInfo()
{
	/* todo: destroy struct */
}
