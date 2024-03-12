#include "usegui.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

extern "C" {
    #include <node.h>
}

void Update_SpikeMap(int16_t *nodeActv, int8_t *connWeights, uint32_t count);
//namespace NeuroPlot {

template <typename T>
inline T RandomRange(T min, T max) {
    T scale = rand() / (T) RAND_MAX;
    return min + scale * ( max - min );
}
uint32_t clickedIdx;
void Update_SpikeMap(int16_t *nodeActv, int8_t *connWeights,uint32_t count) {
    static int scale_min       = -80;
    static int scale_max       = 80;

    static ImPlotColormap map = ImPlotColormap_Jet;
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
    ImGui::DragIntRange2("Min / Max",&scale_min, &scale_max, 0.5f, -1500, 1500);

    static ImPlotHeatmapFlags hm_flags = 0;

    static ImPlotAxisFlags axes_flags = ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickMarks;

    ImPlot::PushColormap(map);
    ImPlotPoint mousePos;
    int sqrtCnt = sqrt(count);
    if(ImPlot::BeginPlot("##Heatmap1",ImVec2(500,500),ImPlotFlags_NoLegend)) {
        const char *labels = "%d";
        if(count > sqrtCnt * sqrtCnt) {
            sqrtCnt + 1;
        }
        ImPlot::SetupAxes(nullptr, nullptr, axes_flags, axes_flags);
        ImPlot::SetupAxesLimits(0,sqrtCnt,0,sqrtCnt+1);
        //ImPlot::PlotHeatmap("heat",nodeActv,1,sqrtCnt,scale_min,scale_max,nullptr,ImPlotPoint(0,count+0.025),ImPlotPoint(count,count+1),hm_flags);
        ImPlotRect lims = ImPlot::GetPlotLimits();
        if(lims.X.Max - lims.X.Min > 20 || lims.Y.Max - lims.Y.Min > 20) {
            labels = nullptr;
        }
        if(ImPlot::IsPlotHovered()) {
            mousePos = ImPlot::GetPlotMousePos();
        } else {
            mousePos = ImPlotPoint(-1,-1);
        }
        ImPlot::PlotHeatmap("heat1",nodeActv,sqrtCnt,sqrtCnt,scale_min,scale_max,labels,ImPlotPoint(0,sqrtCnt),ImPlotPoint(sqrtCnt,0),hm_flags);
        ImPlot::EndPlot();
    }

    ImGui::SameLine();
    ImPlot::ColormapScale("##HeatScale",scale_min, scale_max, ImVec2(60,225));

    ImPlot::PopColormap();

    if(ImGui::IsMouseClicked(ImGuiMouseButton_Left) && mousePos.x >= 0 && mousePos.y >=0) {
        clickedIdx = ((int)mousePos.y * sqrtCnt + (int)mousePos.x);
    }
    if(clickedIdx < count) {
        node_t *node;
        ImGui::Text("Node %d === \n -v: %d\n",clickedIdx,nodeActv[clickedIdx]);
        if(SpikeSim_GetNode(clickedIdx, &node)) {
            ImGui::Text("Active Inputs: %d -State: %d\n", node->inputUsed, node->time);
            for(int ii = 0; ii < node->inputUsed; ii++) {
                connect_t *source = node->inputs[ii].source;
                event_t *event = node->inputs + ii;
                ImGui::Text("[%d] -t:%d/%d -v:%d/%d",source->source,event->time, source->time, event->value, source->weight);
            }

        }
    }
}

//}