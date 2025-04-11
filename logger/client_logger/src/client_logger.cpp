#include <string>
#include <sstream>
#include <algorithm>
#include <utility>
#include "../include/client_logger.h"
#include <not_implemented.h>

std::unordered_map<std::string, std::pair<size_t, std::ofstream>> client_logger::refcounted_stream::_global_streams;


logger& client_logger::log(
    const std::string &text,
    logger::severity severity) &
{
    std::string formatted = make_format(text, severity);
    auto it = _output_streams.find(severity);
    if (it != _output_streams.end())
    {
        const auto& streams = it->second.first;
        bool console = it->second.second;

        for (const auto& stream : streams)
        {
            if (stream._stream.second)
            {
                *stream._stream.second << formatted << std::endl;
            }
        }

        if (console)
        {
            std::cout << formatted << std::endl;
        }
    }
    return *this;
}

std::string client_logger::make_format(const std::string &message, severity sev) const
{
    std::string result;
    size_t pos = 0;
    while (pos < _format.size())
    {
        if (_format[pos] == '%' && pos + 1 < _format.size())
        {
            char c = _format[pos + 1];
            flag f = char_to_flag(c);
            switch (f)
            {
                case flag::DATE:
                    result += current_date_to_string();
                    break;
                case flag::TIME:
                    result += current_time_to_string();
                    break;
                case flag::SEVERITY:
                    result += severity_to_string(sev);
                    break;
                case flag::MESSAGE:
                    result += message;
                    break;
                default:
                    result += std::string("%") + c;
                    break;
            }
            pos += 2;
        }
        else
        {
            result += _format[pos++];
        }
    }
    return result;
}

client_logger::flag client_logger::char_to_flag(char c) noexcept
{
    switch (c)
    {
        case 'd': return flag::DATE;
        case 't': return flag::TIME;
        case 's': return flag::SEVERITY;
        case 'm': return flag::MESSAGE;
        default: return flag::NO_FLAG;
    }
}

client_logger::client_logger(
        const std::unordered_map<logger::severity, std::pair<std::forward_list<refcounted_stream>, bool>> &streams,
        std::string format)
        : _output_streams(streams), _format(std::move(format))
{
    for (auto& [severity, stream_data] : _output_streams)
    {
        for (auto& stream : stream_data.first)
        {
            stream.open();
        }
    }
}

client_logger::client_logger(const client_logger &other)
    : _output_streams(other._output_streams), _format(other._format)
{
    for (auto& [severity, stream_data] : _output_streams)
    {
        for (auto& stream : stream_data.first)
        {
            stream.open();
        }
    }
}

client_logger &client_logger::operator=(const client_logger &other)
{
    if (this != &other)
    {
        _output_streams = other._output_streams;
        _format = other._format;
        for (auto& [severity, stream_data] : _output_streams)
        {
            for (auto& stream : stream_data.first)
            {
                stream.open();
            }
        }
    }
    return *this;
}

client_logger::client_logger(client_logger &&other) noexcept
    : _output_streams(std::move(other._output_streams)), _format(std::move(other._format)){}

client_logger &client_logger::operator=(client_logger &&other) noexcept
{
    if (this != &other)
    {
        _output_streams = std::move(other._output_streams);
        _format = std::move(other._format);
    }
    return *this;
}

client_logger::~client_logger() noexcept
{
    for (auto& [severity, stream_data] : _output_streams)
    {
        for (auto& stream : stream_data.first)
        {
            stream.close();
        }
    }
}

client_logger::refcounted_stream::refcounted_stream(const std::string &path)
{
    _stream.first = path;
    _stream.second = nullptr;
}

client_logger::refcounted_stream::refcounted_stream(const client_logger::refcounted_stream &other)
    : _stream(other._stream)
{
    open();
}

client_logger::refcounted_stream&
client_logger::refcounted_stream::operator=(const client_logger::refcounted_stream &other)
{
    if (this != &other)
    {
        close();
        _stream = other._stream;
        open();
    }
    return *this;
}

client_logger::refcounted_stream::refcounted_stream(client_logger::refcounted_stream &&other) noexcept
    : _stream(std::move(other._stream))
{
    other._stream.second = nullptr;
}

client_logger::refcounted_stream& client_logger::refcounted_stream::operator=(client_logger::refcounted_stream &&other) noexcept
{
    if (this != &other)
    {
        close();
        _stream = std::move(other._stream);
        other._stream.second = nullptr;
    }
    return *this;
}

void client_logger::refcounted_stream::open()
{
    if (_stream.first.empty())
        return;

    auto& global_entry = _global_streams[_stream.first];
    if (global_entry.first == 0)
    {
        global_entry.second.open(_stream.first, std::ios::app);
        if (!global_entry.second.is_open())
        {
            throw std::runtime_error("Failed to open log file: " + _stream.first);
        }
    }
    global_entry.first++;
    _stream.second = &global_entry.second;
}

client_logger::refcounted_stream::~refcounted_stream()
{
    close();
}

void client_logger::refcounted_stream::close()
{
    if (!_stream.first.empty() && _stream.second != nullptr)
    {
        auto it = _global_streams.find(_stream.first);
        if (it != _global_streams.end())
        {
            if (--it->second.first == 0)
            {
                it->second.second.close();
                _global_streams.erase(it);
            }
        }
        _stream.second = nullptr;
    }
}
