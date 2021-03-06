#include "stdafx.h"
#include "VersionChecker.h"

#include <iostream>
#include <codecvt>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using namespace kudd;
using namespace boost::asio;

// https://github.com/alexandruc/SimpleHttpsClient/blob/master/https_client.cpp,
// https://www.boost.org/doc/libs/1_47_0/doc/html/boost_asio/example/ssl/client.cpp 참조하여 작성.

namespace {
    const std::string PATH("/KuddLim/KakaoTalkAdCloser/releases.atom");
    const std::string SERVER("github.com");
}

VersionChecker& VersionChecker::get()
{
    static VersionChecker __instance__;
    return __instance__;
}

VersionChecker::VersionChecker()
{
    _ioService = std::make_unique<io_service>();
    _resolver = std::make_unique<ip::tcp::resolver>(*_ioService);
    _sslCtx = std::make_unique<ssl::context>(ssl::context::sslv23);
    _sslCtx->set_default_verify_paths();
    _sslSocket = std::make_unique<decltype(_sslSocket)::element_type>(*_ioService, *_sslCtx);
    _request = std::make_unique<boost::asio::streambuf>();
    _response = std::make_unique<boost::asio::streambuf>();
}

VersionChecker::~VersionChecker()
{
    _sslSocket->shutdown();
    _ioService->stop();
}

void VersionChecker::resolveHandler(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator itr)
{
    if (!err) {
        _sslSocket->set_verify_mode(boost::asio::ssl::verify_none);
        //_sslSocket->set_verify_callback(boost::bind(&VersionChecker::verifyHandler, this, _1, _2));
        async_connect(_sslSocket->lowest_layer(), itr, boost::bind(&VersionChecker::connectHandler, this, placeholders::error));
    }
    else {
        // TODO: handle resolving failure
    }
}

bool VersionChecker::verifyHandler(bool verified, boost::asio::ssl::verify_context& ctx)
{
    // do nothing here...
    return true;
}

void VersionChecker::connectHandler(const boost::system::error_code& err)
{
    if (!err) {
        _sslSocket->async_handshake(ssl::stream_base::client, boost::bind(&VersionChecker::handshakeHandler, this, placeholders::error));
    }
    else {
        // TODO: handle connection failure
    }
}

void VersionChecker::handshakeHandler(const boost::system::error_code& err)
{
    if (!err) {
        async_write(*_sslSocket, *_request, boost::bind(&VersionChecker::writeRequestHandler, this, placeholders::error));
    }
    else {
        // TODO: handle SSL handshake failure
    }
}

void VersionChecker::writeRequestHandler(const boost::system::error_code& err)
{
    if (!err) {
        async_read_until(*_sslSocket, *_response, "\r\n", boost::bind(&VersionChecker::readHttpStatusHandler, this, placeholders::error));
    }
    else {
        // TODO: handle write failure
    }
}

void VersionChecker::readHttpStatusHandler(const boost::system::error_code& err)
{
    if (!err) {
        // Check that response is OK.
        std::istream responseStream(_response.get());

        std::string version;
        responseStream >> version;
        uint32_t statusCode;
        responseStream >> statusCode;
        std::string statusMessage;
        std::getline(responseStream, statusMessage);
        if (!responseStream || version.substr(0, 5) != "HTTP/" || statusCode != 200)
        {
            return;
        }

        // Read the response headers, which are terminated by a blank line.
        boost::asio::async_read_until(*_sslSocket, *_response, "\r\n\r\n",
            boost::bind(&VersionChecker::readHttpHeaderHandler, this,
                boost::asio::placeholders::error));
    }
    else {
        // TODO: handle read failure
    }
}

void VersionChecker::readHttpHeaderHandler(const boost::system::error_code& err)
{
    if (!err) {
        // Process the response headers.
        std::istream responseStream(_response.get());
        std::string header;
        while (std::getline(responseStream, header) && header != "\r") {
            ; // NOOP
        }

        // Write whatever content we already have to output.
        if (_response->size() > 0) {
            //std::cout << &response_;
            _httpDataStream << _response.get();
        }

        // Start reading remaining data until EOF.
        boost::asio::async_read(*_sslSocket, *_response,
            boost::asio::transfer_at_least(1),
            boost::bind(&VersionChecker::readRemainingContentHandler, this,
                boost::asio::placeholders::error));
    }
    else {
        // TODO: handle read failure
    }
}

void VersionChecker::readRemainingContentHandler(const boost::system::error_code& err)
{
    if (!err) {
        // Write all of the data that has been read so far.
        _httpDataStream << _response.get();

        // Continue reading remaining data until EOF.
        boost::asio::async_read(*_sslSocket, *_response,
            boost::asio::transfer_at_least(1),
            boost::bind(&VersionChecker::readRemainingContentHandler, this,
                boost::asio::placeholders::error));
    }
    else {
        // TODO: handle read failure
    }
}

namespace {
    std::wstring toUtf(const std::string& utf)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        std::wstring s = converter.from_bytes(utf.c_str());
        return s;
    }
}


bool VersionChecker::check(const std::wstring& lastKnownUpdate)
{
    io_service ioService;

    std::ostream requestStream(_request.get());

    requestStream << "GET " << PATH << " HTTP/1.0\r\n"
                  << "Host: " << SERVER << "\r\n"
                  << "Accept: */*\r\n"
                  << "Connection: close\r\n\r\n";

    ip::tcp::resolver::query dnsQuery(SERVER, "https");
    ip::tcp::resolver dnsResolve(ioService);
    //dnsResolve.async_resolve(dnsQuery, boost::bind(&VersionChecker::resolveHandler, this, placeholders::error, placeholders::iterator));
    _resolver->async_resolve(dnsQuery, boost::bind(&VersionChecker::resolveHandler, this, placeholders::error, placeholders::iterator));
    //auto r = dnsResolve.resolve(dnsQuery);

    _ioService->run();

    boost::property_tree::ptree emptyTree;

    std::wstring updateStr;
    int32_t index = 0;
    bool found = false;

    // FIXME: lastKnownUpdate 를 찾아서 인덱스가 0인지 비교.
    try {
        boost::property_tree::ptree xml;
        boost::property_tree::read_xml(_httpDataStream, xml);
        const auto& feed = xml.get_child("feed", emptyTree);

        for (const auto& entry : feed) {
            if (entry.first != "entry") {
                continue;
            }

            const auto& content = entry.second.get_child("content", emptyTree);
            const auto& updated = entry.second.get_child("updated", emptyTree);

            std::wstring s = toUtf(updated.get_value<std::string>());

            if (s == lastKnownUpdate) {
                found = true;
                ++index;
                break;
            }

            ++index;
        }
    }
    catch (...) {
        updateStr.clear();
    }

    // 내가 알고 있는 최신 정보는, 실제 Release 버전보다 먼저 나온 마지막 버전이다.
    return found && index > 2;
}