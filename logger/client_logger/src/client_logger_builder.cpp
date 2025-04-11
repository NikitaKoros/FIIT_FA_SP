#include <filesystem>
#include <utility>
#include <not_implemented.h>
#include "../include/client_logger_builder.h"
#include <not_implemented.h>

using namespace nlohmann;
namespace fs = std::filesystem;

logger_builder& client_logger_builder::add_file_stream(
    std::string const &stream_file_path,
    logger::severity severity) &
{
    fs::path new_path = _destination / stream_file_path;;
    try {
        new_path = fs::weakly_canonical(new_path);
    } catch (...) {
        new_path = _destination / stream_file_path;
    }

    auto& entry = _output_streams[severity];
    auto& streams = entry.first;

    for (const auto& stream : streams)
    {
        fs::path existing_path;
        try {
            existing_path = fs::weakly_canonical(stream._stream.first);
        } catch (const fs::filesystem_error&) {
            existing_path = fs::path(stream._stream.first);
        }

        if (existing_path == new_path) {
            return *this;
        }
    }

    streams.emplace_front(new_path.string());
    return *this;
}

logger_builder& client_logger_builder::add_console_stream(
    logger::severity severity) &
{
    _output_streams[severity].second = true;
    return *this;
}

logger_builder& client_logger_builder::transform_with_configuration(
    std::string const &configuration_file_path,
    std::string const &configuration_path) &
{
    std::ifstream config_file(configuration_file_path);
    if (!config_file.is_open()) 
    {
        throw std::runtime_error("Failed to open config file");
    }

    json config = json::parse(config_file);
    std::string corrected_path = configuration_path.starts_with('/') 
                            ? configuration_path 
                            : "/" + configuration_path;
    json section = config[json::json_pointer(corrected_path)];

    if (section.contains("format")) 
    {
        _format = section["format"].get<std::string>();
    }

    for (auto& [key, value] : section.items()) 
    {
        if (key == "format") continue;

        try 
        {
            auto severity = logger_builder::string_to_severity(key);
            parse_severity(severity, value);
        } 
        catch (...) 
        {
        }
    }

    return *this;
}

logger_builder& client_logger_builder::clear() &
{
    _output_streams.clear();
    _format = "%m";
    return *this;
}

logger *client_logger_builder::build() const
{
    return new client_logger(_output_streams, _format);
}

logger_builder& client_logger_builder::set_format(const std::string &format) &
{
    _format = format;
    return *this;
}

void client_logger_builder::parse_severity(logger::severity sev, nlohmann::json& j)
{
    if (j.contains("console") && j["console"].get<bool>())
    {
        add_console_stream(sev);
    }
    
    if (j.contains("files"))
    {
        for (auto& file : j["files"])
        {
            add_file_stream(file.get<std::string>(), sev);
        }
    }
}

logger_builder& client_logger_builder::set_destination(const std::string &destination) &
{
    _destination = destination;
    return *this;
}
