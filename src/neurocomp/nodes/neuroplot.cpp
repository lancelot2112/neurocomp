#include "usegui.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

void Update_SpikeMap(int32_t *nodeActv, int8_t *connWeights, uint32_t count);
//namespace NeuroPlot {

template <typename T>
inline T RandomRange(T min, T max) {
    T scale = rand() / (T) RAND_MAX;
    return min + scale * ( max - min );
}

void Update_SpikeMap(int32_t *nodeActv, int8_t *connWeights,uint32_t count) {
    static int scale_min       = -100;
    static int scale_max       = 250;

    static ImPlotColormap map = ImPlotColormap_Viridis;
    if (ImPlot::ColormapButton(ImPlot::GetColormapName(map),ImVec2(225,0),map)) {
        map = (map + 1) % ImPlot::GetColormapCount();
        // We bust the color cache of our plots so that item colors will
        // resample the new colormap in the event that they have already
        // been created. See documentation in implot.h.
        ImPlot::BustColorCache("##Heatmap1");
        ImPlot::BustColorCache("##Heatmap2");
    }

    ImGui::SameLine();
    ImGui::LabelText("##Colormap Index", "%s", "Change Colormap");
    ImGui::SetNextItemWidth(225);
    ImGui::DragIntRange2("Min / Max",&scale_min, &scale_max, 0.01f, -100, 200);

    static ImPlotHeatmapFlags hm_flags = 0;

    static ImPlotAxisFlags axes_flags = ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickMarks;

    ImPlot::PushColormap(map);

    if(ImPlot::BeginPlot("##Heatmap1",ImVec2(500,500),ImPlotFlags_NoLegend)) {
        ImPlot::SetupAxes(nullptr, nullptr, axes_flags, axes_flags);
        ImPlot::SetupAxesLimits(0,count,0,count+1);
        ImPlot::PlotHeatmap("heat",nodeActv,1,count,scale_min,scale_max,nullptr,ImPlotPoint(0,count+0.025),ImPlotPoint(count,count+1),hm_flags);
        ImPlot::PlotHeatmap("heat1",connWeights,count,count,scale_min,scale_max,nullptr,ImPlotPoint(0,0),ImPlotPoint(count,count),hm_flags);
        ImPlot::EndPlot();
    }

    ImGui::SameLine();
    ImPlot::ColormapScale("##HeatScale",scale_min, scale_max, ImVec2(60,225));

    ImPlot::PopColormap();
}

//}