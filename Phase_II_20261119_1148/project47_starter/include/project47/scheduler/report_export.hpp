#pragma once

#include "project47/scheduler/scheduler.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace project47 {

inline void write_json_string(std::ostream& output, std::string_view value) {
    output << '"';
    constexpr char hex[] = "0123456789abcdef";
    for (const unsigned char ch : value) {
        switch (ch) {
            case '"': output << "\\\""; break;
            case '\\': output << "\\\\"; break;
            case '\b': output << "\\b"; break;
            case '\f': output << "\\f"; break;
            case '\n': output << "\\n"; break;
            case '\r': output << "\\r"; break;
            case '\t': output << "\\t"; break;
            default:
                if (ch < 0x20) {
                    output << "\\u00" << hex[(ch >> 4U) & 0x0fU] << hex[ch & 0x0fU];
                } else {
                    output << static_cast<char>(ch);
                }
        }
    }
    output << '"';
}

inline void write_csv_field(std::ostream& output, std::string_view value) {
    const bool quote = value.find_first_of(",\"\r\n") != std::string_view::npos;
    if (!quote) {
        output << value;
        return;
    }
    output << '"';
    for (const char ch : value) {
        if (ch == '"') output << "\"\"";
        else output << ch;
    }
    output << '"';
}

inline std::string join_route(const std::vector<std::string>& route) {
    std::string value;
    for (const std::string& node : route) {
        if (!value.empty()) value += " -> ";
        value += node;
    }
    return value;
}

inline void write_jobs_csv(const SchedulerResult& result, std::ostream& output) {
    output << "scenario,job_id,tile_id,status,compute_node,source_storage,destination_storage,wave,"
              "estimated_seconds,input_bytes,route_cost,input_route,output_route\n";
    for (const JobExecution& execution : result.completed_jobs) {
        const Job& job = execution.job;
        write_csv_field(output, result.scenario); output << ',' << job.id << ',';
        write_csv_field(output, job.tile_id); output << ",completed,";
        write_csv_field(output, execution.compute_node_id); output << ',';
        write_csv_field(output, execution.source_storage_id); output << ',';
        write_csv_field(output, execution.destination_storage_id);
        output << ',' << execution.wave << ',' << std::setprecision(12) << job.estimated_seconds
               << ',' << job.input_bytes << ',' << execution.route_cost << ',';
        write_csv_field(output, join_route(execution.input_route)); output << ',';
        write_csv_field(output, join_route(execution.output_route)); output << '\n';
    }
    for (const Job& job : result.deferred_jobs) {
        write_csv_field(output, result.scenario); output << ',' << job.id << ',';
        write_csv_field(output, job.tile_id); output << ",deferred,,,,," << job.estimated_seconds
               << ',' << job.input_bytes << ",,,\n";
    }
    if (!output) throw std::runtime_error("Failed while writing CSV job report");
}

inline void write_json_report(const SchedulerResult& result, std::ostream& output) {
    output << "{\n  \"scenario\": ";
    write_json_string(output, result.scenario);
    output << ",\n  \"summary\": {\n"
           << "    \"total_jobs\": " << result.total_jobs << ",\n"
           << "    \"completed_jobs\": " << result.completed_count() << ",\n"
           << "    \"deferred_jobs\": " << result.deferred_count() << ",\n"
           << "    \"waves\": " << result.waves << ",\n"
           << "    \"matching_assignments\": " << result.matching_assignments << ",\n"
           << "    \"estimated_processing_seconds\": " << std::setprecision(12)
           << result.total_estimated_processing_seconds << ",\n"
           << "    \"estimated_makespan_seconds\": " << result.estimated_makespan_seconds << ",\n"
           << "    \"makespan_model\": \"wave_barrier_estimate\",\n"
           << "    \"total_route_cost\": " << result.total_route_cost << "\n  },\n"
           << "  \"jobs\": [\n";
    for (std::size_t i = 0; i < result.completed_jobs.size(); ++i) {
        const JobExecution& execution = result.completed_jobs[i];
        output << "    {\"job_id\": " << execution.job.id << ", \"tile_id\": ";
        write_json_string(output, execution.job.tile_id);
        output << ", \"status\": \"completed\", \"compute_node\": ";
        write_json_string(output, execution.compute_node_id);
        output << ", \"source_storage\": ";
        write_json_string(output, execution.source_storage_id);
        output << ", \"destination_storage\": ";
        write_json_string(output, execution.destination_storage_id);
        output << ", \"wave\": " << execution.wave
               << ", \"estimated_seconds\": " << execution.job.estimated_seconds
               << ", \"input_bytes\": " << execution.job.input_bytes
               << ", \"route_cost\": " << execution.route_cost << ", \"input_route\": [";
        for (std::size_t j = 0; j < execution.input_route.size(); ++j) {
            if (j != 0) output << ", ";
            write_json_string(output, execution.input_route[j]);
        }
        output << "], \"output_route\": [";
        for (std::size_t j = 0; j < execution.output_route.size(); ++j) {
            if (j != 0) output << ", ";
            write_json_string(output, execution.output_route[j]);
        }
        output << "]}" << (i + 1 == result.completed_jobs.size() ? "\n" : ",\n");
    }
    output << "  ],\n  \"deferred_job_ids\": [";
    for (std::size_t i = 0; i < result.deferred_jobs.size(); ++i) {
        if (i != 0) output << ", ";
        output << result.deferred_jobs[i].id;
    }
    output << "],\n  \"final_storage_used_bytes\": {";
    bool first = true;
    for (const auto& [id, used] : result.final_storage_used_bytes) {
        if (!first) output << ',';
        output << "\n    ";
        write_json_string(output, id);
        output << ": " << used;
        first = false;
    }
    if (!first) output << '\n';
    output << "  }\n}\n";
    if (!output) throw std::runtime_error("Failed while writing JSON scheduler report");
}

inline void export_scheduler_reports(const SchedulerResult& result,
                                     const std::filesystem::path& output_directory) {
    std::filesystem::create_directories(output_directory);
    const std::filesystem::path csv_path = output_directory / (result.scenario + "_jobs.csv");
    const std::filesystem::path json_path = output_directory / (result.scenario + "_report.json");
    std::ofstream csv(csv_path);
    if (!csv) throw std::runtime_error("Cannot open CSV report for writing: " + csv_path.string());
    write_jobs_csv(result, csv);
    csv.close();
    if (!csv) throw std::runtime_error("Cannot finish CSV report: " + csv_path.string());

    std::ofstream json(json_path);
    if (!json) throw std::runtime_error("Cannot open JSON report for writing: " + json_path.string());
    write_json_report(result, json);
    json.close();
    if (!json) throw std::runtime_error("Cannot finish JSON report: " + json_path.string());
}

} // namespace project47
