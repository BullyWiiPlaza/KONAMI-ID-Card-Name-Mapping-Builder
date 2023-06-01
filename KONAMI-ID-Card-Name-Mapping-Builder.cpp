#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <iosfwd>
#include <sstream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>
#include <boost/algorithm/string/replace.hpp>

/**
 * \brief Downloads file contents from the specified URL. Might throw an exception upon failure.
 * \param url The URL to download from.
 * \return The file contents downloaded from the URL.
 */
std::string download_file_contents(const std::string &url)
{
    const auto request = std::make_shared<curlpp::Easy>();
    request->setOpt(new curlpp::options::Url(url));

    std::ostringstream response_stream;
    request->setOpt(new curlpp::options::WriteStream(&response_stream));

    request->perform();

    return response_stream.str();
}

struct card_id_mapping_t
{
	/**
	 * \brief The KONAMI ID.
	 */
	int card_id;

	/**
	 * \brief The card's English name.
	 */
	std::string card_name;

	/**
	 * \brief Comparison operator for sorting a container of those struct objects.
	 * \param other The other class object to compare to.
	 * \return Whether the current struct comes before the other one.
	 */
	bool operator<(const card_id_mapping_t& other) const
	{
        return card_id < other.card_id;
    }
};

constexpr auto invalid_card_id = 0;

std::string to_cpp_header(const std::vector<card_id_mapping_t> &card_id_mappings)
{
    std::string header_file_contents = "#pragma once\n\n#include <map>\n#include <string>\n\n"
									   "std::map<int, std::wstring> card_id_mapping =\n{\n";

    size_t card_id_mapping_index = 0;
    for (const auto& [card_id, card_name] : card_id_mappings)
    {
        const auto escaped_card_name = boost::replace_all_copy(card_name, "\"", "\\\"");

        header_file_contents += "\t";

        if (card_id == invalid_card_id)
        {
            header_file_contents += "// ";
        }

        header_file_contents += "{" + std::to_string(card_id)
    	+ ", L\"" + escaped_card_name + "\"}";

        if (card_id_mapping_index != card_id_mappings.size() - 1)
        {
            header_file_contents += ",";
        }

        header_file_contents += "\n";

        card_id_mapping_index++;
    }

    header_file_contents += "};";

    return header_file_contents;
}

void write_string_to_file(const std::filesystem::path& file_path, const std::string& file_contents)
{
    std::ofstream file_writer;
    file_writer.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    file_writer.open(file_path.wstring(), std::ios::out | std::ios::binary);
    file_writer << file_contents;
}

constexpr auto include_negative_konami_ids = false;

/*
	Note: Currently not all KONAMI IDs are available in the ygoprodeck API, e.g. https://db.ygoprodeck.com/api/v7/cardinfo.php?misc=yes&name=Lucky%20Trinket
	IDs are retrieved from the "official" KONAMI database: https://www.db.yugioh-card.com/yugiohdb/card_search.action?ope=2&cid=4189
*/
int main()
{
    spdlog::info("KONAMI ID -> Card Name Mapping Builder (C) 2023 by BullyWiiPlaza");

#ifdef _DEBUG
    spdlog::info("Debug Build");
#else
    spdlog::info("Release Build");
#endif

    spdlog::stopwatch stopwatch;

    std::vector<card_id_mapping_t> card_id_mapping;

    spdlog::info("Downloading card details...");

    // Using the misc=yes parameter we can force the JSON of all cards to also contain the KONAMI ID
    const std::string download_url = "https://db.ygoprodeck.com/api/v7/cardinfo.php?misc=yes";

    const auto json_file_contents = download_file_contents(download_url);

    spdlog::info("Parsing card details...");
    // Parse the JSON in order to get the information we need
    const auto parsed_json = nlohmann::json::parse(json_file_contents);

    spdlog::info("Reading card ID mappings...");
    for (const auto &data = parsed_json["data"];
        const auto& data_entry : data)
    {
	    const auto &card_name = data_entry["name"].get<std::string>();

        auto konami_id_found = false;
	    for (const auto &misc_info = data_entry["misc_info"];
            const auto &misc_info_entry : misc_info)
        {
            if (misc_info_entry.contains("konami_id"))
            {
                // spdlog::info("Adding card name: " + card_name);
                const auto konami_id = misc_info_entry["konami_id"].get<int>();

                if constexpr (!include_negative_konami_ids)
                {
                    if (konami_id < 0)
                    {
                        continue;
                    }
                }

                card_id_mapping.push_back({ konami_id, card_name });
                konami_id_found = true;

                break;
            }
        }

        if (!konami_id_found)
        {
            // spdlog::error("KONAMI ID not found for card name " + card_name);
            card_id_mapping.push_back({ invalid_card_id, card_name });
        }
    }

    spdlog::info("Sorting...");
    std::sort(card_id_mapping.begin(), card_id_mapping.end());

    spdlog::info("Building C++ header...");
    const auto cpp_code = to_cpp_header(card_id_mapping);

    spdlog::info("Writing C++ header...");
    write_string_to_file("CardIdMapping.hpp", cpp_code);

    // ReSharper disable once CppRedundantQualifier
    spdlog::info("Process took: {}s", stopwatch);
}