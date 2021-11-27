#pragma once

#include "transport_catalogue.h"
#include "domain.h"
#include "geo.h"
#include "map_renderer.h"
#include "transport_catalogue.pb.h"
#include "graph.h"
#include "transport_router.h"

#include <fstream>
#include <iostream>

void Serialization(std::ofstream& out, 
	const InformationForCatalog& query,
	const renderer_for_set::RenderSettings& render_set,
	const transport_router::InitGraph& init
);
 
InformationForCatalog DeserializeCatalog(transport_set::Transport_set& trans_list);

renderer_for_set::RenderSettings DeserializeMap(transport_set::Transport_set& render);

transport_router::detail::InputSerialization DeserializeGraph(transport_set::Transport_set& render);