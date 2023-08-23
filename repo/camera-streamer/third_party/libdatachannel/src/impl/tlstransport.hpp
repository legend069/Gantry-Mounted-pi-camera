/**
 * Copyright (c) 2020 Paul-Louis Ageneau
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef RTC_IMPL_TLS_TRANSPORT_H
#define RTC_IMPL_TLS_TRANSPORT_H

#include "certificate.hpp"
#include "common.hpp"
#include "queue.hpp"
#include "tls.hpp"
#include "transport.hpp"

#if RTC_ENABLE_WEBSOCKET

#include <thread>

namespace rtc::impl {

class TcpTransport;

class TlsTransport : public Transport {
public:
	static void Init();
	static void Cleanup();

	TlsTransport(shared_ptr<TcpTransport> lower, optional<string> host, certificate_ptr certificate,
	             state_callback callback);
	virtual ~TlsTransport();

	void start() override;
	bool stop() override;
	bool send(message_ptr message) override;

	bool isClient() const { return mIsClient; }

protected:
	virtual void incoming(message_ptr message) override;
	virtual void postHandshake();
	void runRecvLoop();

	const optional<string> mHost;
	const bool mIsClient;

	Queue<message_ptr> mIncomingQueue;
	std::thread mRecvThread;

#if USE_GNUTLS
	gnutls_session_t mSession;

	message_ptr mIncomingMessage;
	size_t mIncomingMessagePosition = 0;

	static ssize_t WriteCallback(gnutls_transport_ptr_t ptr, const void *data, size_t len);
	static ssize_t ReadCallback(gnutls_transport_ptr_t ptr, void *data, size_t maxlen);
	static int TimeoutCallback(gnutls_transport_ptr_t ptr, unsigned int ms);
#else
	SSL_CTX *mCtx;
	SSL *mSsl;
	BIO *mInBio, *mOutBio;

	static int TransportExIndex;

	static int CertificateCallback(int preverify_ok, X509_STORE_CTX *ctx);
	static void InfoCallback(const SSL *ssl, int where, int ret);
#endif
};

} // namespace rtc::impl

#endif

#endif
