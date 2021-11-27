#include "json.h"
#include "json_reader.h"
#include "domain.h"
#include "serialization.h"

#include <string>
#include <fstream>
#include <filesystem>

using namespace std;

int MakeBase() {

	ifstream f("in.txt");

	json::Document input_doc = json::Load(f);

    InformationForCatalog queres = input_json::JsonReader(input_doc.GetRoot().AsDict().at("base_requests").AsArray());
    queres.routing_settings = input_json::GetRoutingSettings(input_doc.GetRoot().AsDict().at("routing_settings").AsDict());
    renderer_for_set::RenderSettings render_set = input_json::GetRenderSettings(input_doc.GetRoot().AsDict().at("render_settings").AsDict());
    ForSerialization serial = input_json::GetSettingsForSerialization(input_doc.GetRoot().AsDict().at("serialization_settings").AsDict());

    std::ofstream file(serial.path_for_file, ios::binary);

    Serialization(file, std::move(queres));
}