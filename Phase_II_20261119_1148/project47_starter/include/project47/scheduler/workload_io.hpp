#pragma once

#include "project47/models/job.hpp"

#include <charconv>
#include <cstdint>
#include <istream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace project47 {

struct LoadedJobData {
    std::vector<Job> jobs;
    std::vector<std::pair<std::uint64_t, std::uint64_t>> dependencies;
};

inline std::vector<std::string> parse_csv_record(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool quoted = false;
    for (std::size_t i = 0; i < line.size(); ++i) {
        const char ch = line[i];
        if (quoted) {
            if (ch == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') {
                    field.push_back('"');
                    ++i;
                } else {
                    quoted = false;
                }
            } else {
                field.push_back(ch);
            }
        } else if (ch == ',') {
            fields.push_back(field);
            field.clear();
        } else if (ch == '"') {
            if (!field.empty()) throw std::invalid_argument("CSV quote must begin at the start of a field");
            quoted = true;
        } else {
            field.push_back(ch);
        }
    }
    if (quoted) throw std::invalid_argument("CSV record contains an unterminated quoted field");
    fields.push_back(field);
    return fields;
}

template <typename Integer>
inline Integer parse_csv_integer(const std::string& token, const char* field_name) {
    Integer value{};
    const char* first = token.data();
    const char* last = first + token.size();
    const auto parsed = std::from_chars(first, last, value);
    if (token.empty() || parsed.ec != std::errc{} || parsed.ptr != last) {
        throw std::invalid_argument(std::string("Invalid CSV integer in ") + field_name + ": " + token);
    }
    return value;
}

inline double parse_csv_double(const std::string& token, const char* field_name) {
    double value{};
    const char* first = token.data();
    const char* last = first + token.size();
    const auto parsed = std::from_chars(first, last, value);
    if (token.empty() || parsed.ec != std::errc{} || parsed.ptr != last) {
        throw std::invalid_argument(std::string("Invalid CSV number in ") + field_name + ": " + token);
    }
    return value;
}

// CSV schema: id,tile_id,priority,estimated_seconds,input_bytes,prerequisites
// The prerequisites column is optional and contains semicolon-separated job IDs.
inline LoadedJobData load_jobs_csv(std::istream& input) {
    LoadedJobData loaded;
    std::string line;
    std::size_t line_number = 0;
    bool first_record = true;

    while (std::getline(input, line)) {
        ++line_number;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        try {
            const auto fields = parse_csv_record(line);
            if (first_record) {
                first_record = false;
                if (!fields.empty() && (fields[0] == "id" || fields[0] == "job_id")) continue;
            }
            if (fields.size() < 5 || fields.size() > 6) {
                throw std::invalid_argument("Expected 5 or 6 CSV columns");
            }
            Job job{
                parse_csv_integer<std::uint64_t>(fields[0], "id"),
                fields[1],
                parse_csv_integer<int>(fields[2], "priority"),
                parse_csv_double(fields[3], "estimated_seconds"),
                parse_csv_integer<std::uint64_t>(fields[4], "input_bytes"),
                JobStatus::Pending
            };
            job.validate();
            loaded.jobs.push_back(job);

            if (fields.size() == 6 && !fields[5].empty()) {
                std::size_t start = 0;
                while (start <= fields[5].size()) {
                    const std::size_t delimiter = fields[5].find(';', start);
                    const std::size_t end = delimiter == std::string::npos ? fields[5].size() : delimiter;
                    const std::string token = fields[5].substr(start, end - start);
                    if (token.empty()) throw std::invalid_argument("Empty prerequisite ID in prerequisites column");
                    loaded.dependencies.emplace_back(
                        parse_csv_integer<std::uint64_t>(token, "prerequisites"), job.id);
                    if (delimiter == std::string::npos) break;
                    start = delimiter + 1;
                }
            }
        } catch (const std::exception& error) {
            throw std::invalid_argument("CSV line " + std::to_string(line_number) + ": " + error.what());
        }
    }
    if (loaded.jobs.empty()) throw std::invalid_argument("CSV workload contains no job records");
    return loaded;
}

} // namespace project47
