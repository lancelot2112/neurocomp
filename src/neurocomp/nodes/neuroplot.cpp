#include "implot.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

extern "C" {
    void Update_SpikeMap(int32_t *nodeActv, int8_t *connWeights, uint32_t count);
}
//namespace NeuroPlot {

template <typename T>
inline T RandomRange(T min, T max) {
    T scale = rand() / (T) RAND_MAX;
    return min + scale * ( max - min );
}

void Update_SpikeMap(int32_t *nodeActv, int8_t *connWeights,uint32_t count) {
    static int scale_min       = -20;
    static int scale_max       = 20;
    static const char* xlabels[] = {"1","2","3","4"};
    static const char* ylabels[] = {"Actv"};

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

    ImGui::CheckboxFlags("Column Major", (unsigned int*)&hm_flags, ImPlotHeatmapFlags_ColMajor);

    static ImPlotAxisFlags axes_flags = ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickMarks;

    ImPlot::PushColormap(map);

    if (ImPlot::BeginSubplots("##Heatmap1",2,1,ImVec2(225,285),ImPlotFlags_NoLegend)) {
        //ImPlot::SetupAxisTicks(ImAxis_X1,0 + 1.0/4.0, 1 - 1.0/4.0, 4, xlabels);
        //ImPlot::SetupAxisTicks(ImAxis_Y1,1 - 1.0/4.0, 0 + 1.0/4.0, 1, ylabels);
        ImPlot::BeginPlot("##Heatmap1",ImVec2(225,60),ImPlotFlags_NoLegend);
        ImPlot::SetupAxes(nullptr, nullptr, axes_flags, ImPlotAxisFlags_NoDecorations);
        ImPlot::PlotHeatmap("heat",nodeActv,1,count,scale_min,scale_max,"%d",ImPlotPoint(0,0),ImPlotPoint(1,1),hm_flags);
        ImPlot::EndPlot();
        ImPlot::BeginPlot("##Heatmap2",ImVec2(225,225),ImPlotFlags_NoLegend);
        ImPlot::SetupAxes(nullptr, nullptr, axes_flags, ImPlotAxisFlags_NoDecorations);
        ImPlot::PlotHeatmap("heat1",connWeights,count,count,scale_min,scale_max,"%d");
        ImPlot::EndPlot();
    }
    ImGui::SameLine();
    ImPlot::ColormapScale("##HeatScale",scale_min, scale_max, ImVec2(60,225));

    ImPlot::PopColormap();
}

//}