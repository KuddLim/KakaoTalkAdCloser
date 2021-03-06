#pragma once

#include <string>
#include <memory>
#include <sstream>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/asio/ssl.hpp>

namespace kudd {
    class VersionChecker {
    public:
        virtual ~VersionChecker();
        static VersionChecker& get();
        bool check(const std::wstring& lastKnownUpdate);

    private:
        VersionChecker();
        void resolveHandler(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator itr);
        bool verifyHandler(bool verified, boost::asio::ssl::verify_context& ctx);
        void connectHandler(const boost::system::error_code& err);
        void handshakeHandler(const boost::system::error_code& err);
        void writeRequestHandler(const boost::system::error_code& err);
        void readHttpStatusHandler(const boost::system::error_code& err);
        void readHttpHeaderHandler(const boost::system::error_code& err);
        void readRemainingContentHandler(const boost::system::error_code& err);

    private:
        std::unique_ptr<boost::asio::io_service> _ioService;
        std::unique_ptr<boost::asio::ssl::context> _sslCtx;
        std::unique_ptr<boost::asio::ip::tcp::resolver> _resolver;
        std::unique_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket> > _sslSocket;
        std::unique_ptr<boost::asio::streambuf> _request, _response;
        std::stringstream _httpDataStream;
    };
}
