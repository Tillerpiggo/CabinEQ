/*
  ==============================================================================

    SliderCalibrationManager.cpp
    Created: 10 Jul 2024 3:39:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "PlaybackManager.h"
#include <cmath>
#include <random>

PlaybackManager::PlaybackManager()
    : filter (FFT_SIZE),
      arbitrarySequencer (std::make_unique<SineWaveGenerator> (SineWaveGenerator())),
      arbitrarySequencer2 (std::make_unique<SineWaveGenerator> (SineWaveGenerator())),
      isTesting (false),
      isSweeping (false),
      isCalibrating (false),
      isProcessing (false),
      hasPreparedFilter (false)
{
    dryGainProcessor.setGainDecibels (0.0f);
    wetGainProcessor.setGainDecibels (0.0f);
    
    // Initialize target curve
    std::vector<double> frequencies = {42.065, 67.065, 92.065, 117.065, 142.065, 167.065, 192.065, 217.065, 242.065, 267.065, 292.065, 317.065, 342.065, 367.065, 392.065, 417.065, 442.065, 467.065, 492.065, 517.065, 542.065, 567.065, 592.065, 617.065, 642.065, 667.065, 692.065, 717.065, 742.065, 767.065, 792.065, 817.065, 842.065, 867.065, 892.065, 917.065, 942.065, 967.065, 992.065, 1017.065, 1042.065, 1067.065, 1092.065, 1117.065, 1142.065, 1167.065, 1192.065, 1217.065, 1242.065, 1267.065, 1292.065, 1317.065, 1342.065, 1367.065, 1392.065, 1417.065, 1442.065, 1467.065, 1492.065, 1517.065, 1542.065, 1567.065, 1592.065, 1617.065, 1642.065, 1667.065, 1692.065, 1717.065, 1742.065, 1767.065, 1792.065, 1817.065, 1842.065, 1867.065, 1892.065, 1917.065, 1942.065, 1967.065, 1992.065, 2017.065, 2042.065, 2067.065, 2092.065, 2117.065, 2142.065, 2167.065, 2192.065, 2217.065, 2242.065, 2267.065, 2292.065, 2317.065, 2342.065, 2367.065, 2392.065, 2417.065, 2442.065, 2467.065, 2492.065, 2517.065, 2542.065, 2567.065, 2592.065, 2617.065, 2642.065, 2667.065, 2692.065, 2717.065, 2742.065, 2767.065, 2792.065, 2817.065, 2842.065, 2867.065, 2892.065, 2917.065, 2942.065, 2967.065, 2992.065, 3017.065, 3042.065, 3067.065, 3092.065, 3117.065, 3142.065, 3167.065, 3192.065, 3217.065, 3242.065, 3267.065, 3292.065, 3317.065, 3342.065, 3367.065, 3392.065, 3417.065, 3442.065, 3467.065, 3492.065, 3517.065, 3542.065, 3567.065, 3592.065, 3617.065, 3642.065, 3667.065, 3692.065, 3717.065, 3742.065, 3767.065, 3792.065, 3817.065, 3842.065, 3867.065, 3892.065, 3917.065, 3942.065, 3967.065, 3992.065, 4017.065, 4042.065, 4067.065, 4092.065, 4117.065, 4142.065, 4167.065, 4192.065, 4217.065, 4242.065, 4267.065, 4292.065, 4317.065, 4342.065, 4367.065, 4392.065, 4417.065, 4442.065, 4467.065, 4492.065, 4517.065, 4542.065, 4567.065, 4592.065, 4617.065, 4642.065, 4667.065, 4692.065, 4717.065, 4742.065, 4767.065, 4792.065, 4817.065, 4842.065, 4867.065, 4892.065, 4917.065, 4942.065, 4967.065, 4992.065, 5017.065, 5042.065, 5067.065, 5092.065, 5117.065, 5142.065, 5167.065, 5192.065, 5217.065, 5242.065, 5267.065, 5292.065, 5317.065, 5342.065, 5367.065, 5392.065, 5417.065, 5442.065, 5467.065, 5492.065, 5517.065, 5542.065, 5567.065, 5592.065, 5617.065, 5642.065, 5667.065, 5692.065, 5717.065, 5742.065, 5767.065, 5792.065, 5817.065, 5842.065, 5867.065, 5892.065, 5917.065, 5942.065, 5967.065, 5992.065, 6017.065, 6042.065, 6067.065, 6092.065, 6117.065, 6142.065, 6167.065, 6192.065, 6217.065, 6242.065, 6267.065, 6292.065, 6317.065, 6342.065, 6367.065, 6392.065, 6417.065, 6442.065, 6467.065, 6492.065, 6517.065, 6542.065, 6567.065, 6592.065, 6617.065, 6642.065, 6667.065, 6692.065, 6717.065, 6742.065, 6767.065, 6792.065, 6817.065, 6842.065, 6867.065, 6892.065, 6917.065, 6942.065, 6967.065, 6992.065, 7017.065, 7042.065, 7067.065, 7092.065, 7117.065, 7142.065, 7167.065, 7192.065, 7217.065, 7242.065, 7267.065, 7292.065, 7317.065, 7342.065, 7367.065, 7392.065, 7417.065, 7442.065, 7467.065, 7492.065, 7517.065, 7542.065, 7567.065, 7592.065, 7617.065, 7642.065, 7667.065, 7692.065, 7717.065, 7742.065, 7767.065, 7792.065, 7817.065, 7842.065, 7867.065, 7892.065, 7917.065, 7942.065, 7967.065, 7992.065, 8017.065, 8042.065, 8067.065, 8092.065, 8117.065, 8142.065, 8167.065, 8192.064, 8217.064, 8242.064, 8267.064, 8292.064, 8317.064, 8342.064, 8367.064, 8392.064, 8417.064, 8442.064, 8467.064, 8492.064, 8517.064, 8542.064, 8567.064, 8592.064, 8617.064, 8642.064, 8667.064, 8692.064, 8717.064, 8742.064, 8767.064, 8792.064, 8817.064, 8842.064, 8867.064, 8892.064, 8917.064, 8942.064, 8967.064, 8992.064, 9017.064, 9042.064, 9067.064, 9092.064, 9117.064, 9142.064, 9167.064, 9192.064, 9217.064, 9242.064, 9267.064, 9292.064, 9317.064, 9342.064, 9367.064, 9392.064, 9417.064, 9442.064, 9467.064, 9492.064, 9517.064, 9542.064, 9567.064, 9592.064, 9617.064, 9642.064, 9667.064, 9692.064, 9717.064, 9742.064, 9767.064, 9792.064, 9817.064, 9842.064, 9867.064, 9892.064, 9917.064, 9942.064, 9967.064, 9992.064, 10017.064, 10042.064, 10067.064, 10092.064, 10117.064, 10142.064, 10167.064, 10192.064, 10217.064, 10242.064, 10267.064, 10292.064, 10317.064, 10342.064, 10367.064, 10392.064, 10417.064, 10442.064, 10467.064, 10492.064, 10517.064, 10542.064, 10567.064, 10592.064, 10617.064, 10642.064, 10667.064, 10692.064, 10717.064, 10742.064, 10767.064, 10792.064, 10817.064, 10842.064, 10867.064, 10892.064, 10917.064, 10942.064, 10967.064, 10992.064, 11017.064, 11042.064, 11067.064, 11092.064, 11117.064, 11142.064, 11167.064, 11192.064, 11217.064, 11242.064, 11267.064, 11292.064, 11317.064, 11342.064, 11367.064, 11392.064, 11417.064, 11442.064, 11467.064, 11492.064, 11517.064, 11542.064, 11567.064, 11592.064, 11617.064, 11642.064, 11667.064, 11692.064, 11717.064, 11742.064, 11767.064, 11792.064, 11817.064, 11842.064, 11867.064, 11892.064, 11917.064, 11942.064, 11967.064, 11992.064, 12017.064, 12042.064, 12067.064, 12092.064, 12117.064, 12142.064, 12167.064, 12192.064, 12217.064, 12242.064, 12267.064, 12292.064, 12317.064, 12342.064, 12367.064, 12392.064, 12417.064, 12442.064, 12467.064, 12492.064, 12517.064, 12542.064, 12567.064, 12592.064, 12617.064, 12642.064, 12667.064, 12692.064, 12717.064, 12742.064, 12767.064, 12792.064, 12817.064, 12842.064, 12867.064, 12892.064, 12917.064, 12942.064, 12967.064, 12992.064, 13017.064, 13042.064, 13067.064, 13092.064, 13117.064, 13142.064, 13167.064, 13192.064, 13217.064, 13242.064, 13267.064, 13292.064, 13317.064, 13342.064, 13367.064, 13392.064, 13417.064, 13442.064, 13467.064, 13492.064, 13517.064, 13542.064, 13567.064, 13592.064, 13617.064, 13642.064, 13667.064, 13692.064, 13717.064, 13742.064, 13767.064, 13792.064, 13817.064, 13842.064, 13867.064, 13892.064, 13917.064, 13942.064, 13967.064, 13992.064, 14017.064, 14042.064, 14067.064, 14092.064, 14117.064, 14142.064, 14167.064, 14192.064, 14217.064, 14242.064, 14267.064, 14292.064, 14317.064, 14342.064, 14367.064, 14392.064, 14417.064, 14442.064, 14467.064, 14492.064, 14517.064, 14542.064, 14567.064, 14592.064, 14617.064, 14642.064, 14667.064, 14692.064, 14717.064, 14742.064, 14767.064, 14792.064, 14817.064, 14842.064, 14867.064, 14892.064, 14917.064, 14942.064, 14967.064, 14992.064, 15017.064, 15042.064, 15067.064, 15092.064, 15117.064, 15142.064, 15167.064, 15192.064, 15217.064, 15242.064, 15267.064, 15292.064, 15317.064, 15342.064, 15367.064, 15392.064, 15417.064, 15442.064, 15467.064, 15492.064, 15517.064, 15542.064, 15567.064, 15592.064, 15617.064, 15642.064, 15667.064, 15692.064, 15717.064, 15742.064, 15767.064, 15792.064, 15817.064, 15842.064, 15867.064, 15892.064, 15917.064, 15942.064, 15967.064, 15992.064, 16017.064, 16042.064, 16067.064, 16092.064, 16117.064, 16142.064, 16167.064, 16192.064, 16217.064, 16242.064, 16267.064, 16292.064, 16317.064, 16342.064, 16367.064, 16392.064, 16417.064, 16442.064, 16467.064, 16492.064, 16517.064, 16542.064, 16567.064, 16592.064, 16617.064, 16642.064, 16667.064, 16692.064, 16717.064, 16742.064, 16767.064, 16792.064, 16817.064, 16842.064, 16867.064, 16892.064, 16917.064, 16942.064, 16967.064, 16992.064, 17017.064, 17042.064, 17067.064, 17092.064, 17117.064, 17142.064, 17167.064, 17192.064, 17217.064, 17242.064, 17267.064, 17292.064, 17317.064, 17342.064, 17367.064, 17392.064, 17417.064, 17442.064, 17467.064, 17492.064, 17517.064, 17542.064, 17567.064, 17592.064, 17617.064, 17642.064, 17667.064, 17692.064, 17717.064, 17742.064, 17767.064, 17792.064, 17817.064, 17842.064, 17867.064, 17892.064, 17917.064, 17942.064, 17967.064, 17992.064, 18017.064, 18042.064, 18067.064, 18092.064, 18117.064, 18142.064, 18167.064, 18192.064, 18217.064, 18242.064, 18267.064, 18292.064, 18317.064, 18342.064, 18367.064, 18392.064, 18417.064, 18442.064, 18467.064, 18492.064, 18517.064, 18542.064, 18567.064, 18592.064, 18617.064, 18642.064, 18667.064, 18692.064, 18717.064, 18742.064, 18767.064, 18792.064, 18817.064, 18842.064, 18867.064};
    std::vector<double> amplitudes = {6.243, 5.563, 3.856, 2.078, 0.847, 0.207, -0.120, -0.293, -0.344, -0.443, -0.478, -0.474, -0.476, -0.495, -0.445, -0.467, -0.541, -0.611, -0.613, -0.617, -0.635, -0.643, -0.643, -0.635, -0.635, -0.678, -0.739, -0.770, -0.764, -0.750, -0.748, -0.755, -0.778, -0.824, -0.870, -0.900, -0.902, -0.899, -0.899, -0.914, -0.933, -0.954, -0.974, -0.991, -1.007, -1.021, -1.025, -1.027, -1.030, -1.035, -1.055, -1.082, -1.110, -1.137, -1.158, -1.165, -1.169, -1.173, -1.178, -1.190, -1.211, -1.234, -1.257, -1.278, -1.297, -1.302, -1.306, -1.308, -1.310, -1.313, -1.319, -1.332, -1.346, -1.361, -1.375, -1.390, -1.403, -1.410, -1.416, -1.422, -1.427, -1.432, -1.437, -1.442, -1.450, -1.458, -1.466, -1.474, -1.482, -1.491, -1.499, -1.508, -1.517, -1.525, -1.534, -1.542, -1.550, -1.557, -1.564, -1.568, -1.572, -1.575, -1.578, -1.581, -1.585, -1.589, -1.594, -1.600, -1.611, -1.624, -1.637, -1.651, -1.664, -1.678, -1.691, -1.703, -1.715, -1.723, -1.727, -1.730, -1.733, -1.735, -1.737, -1.739, -1.740, -1.743, -1.745, -1.748, -1.757, -1.767, -1.777, -1.787, -1.798, -1.808, -1.819, -1.829, -1.838, -1.847, -1.856, -1.860, -1.862, -1.863, -1.864, -1.864, -1.864, -1.865, -1.865, -1.865, -1.866, -1.867, -1.869, -1.873, -1.881, -1.889, -1.898, -1.907, -1.916, -1.925, -1.934, -1.943, -1.952, -1.961, -1.969, -1.976, -1.984, -1.985, -1.986, -1.986, -1.986, -1.986, -1.985, -1.985, -1.984, -1.983, -1.983, -1.982, -1.982, -1.983, -1.983, -1.986, -1.993, -1.999, -2.006, -2.014, -2.021, -2.029, -2.036, -2.044, -2.052, -2.060, -2.067, -2.075, -2.082, -2.089, -2.095, -2.099, -2.100, -2.102, -2.103, -2.104, -2.104, -2.105, -2.105, -2.106, -2.106, -2.107, -2.107, -2.108, -2.108, -2.109, -2.110, -2.112, -2.115, -2.120, -2.126, -2.132, -2.138, -2.144, -2.151, -2.157, -2.164, -2.170, -2.177, -2.184, -2.190, -2.197, -2.204, -2.210, -2.216, -2.222, -2.228, -2.233, -2.236, -2.239, -2.242, -2.245, -2.247, -2.250, -2.252, -2.254, -2.256, -2.258, -2.260, -2.262, -2.264, -2.266, -2.268, -2.270, -2.271, -2.273, -2.275, -2.277, -2.280, -2.282, -2.285, -2.288, -2.291, -2.294, -2.297, -2.300, -2.303, -2.306, -2.309, -2.312, -2.314, -2.317, -2.320, -2.323, -2.326, -2.329, -2.332, -2.335, -2.338, -2.341, -2.344, -2.346, -2.349, -2.352, -2.355, -2.357, -2.360, -2.363, -2.365, -2.368, -2.371, -2.373, -2.376, -2.378, -2.381, -2.383, -2.386, -2.388, -2.390, -2.393, -2.395, -2.397, -2.399, -2.401, -2.403, -2.405, -2.406, -2.408, -2.409, -2.411, -2.412, -2.414, -2.415, -2.417, -2.418, -2.420, -2.421, -2.423, -2.424, -2.426, -2.427, -2.429, -2.431, -2.432, -2.434, -2.436, -2.437, -2.439, -2.441, -2.443, -2.445, -2.448, -2.451, -2.455, -2.458, -2.461, -2.464, -2.467, -2.470, -2.474, -2.477, -2.480, -2.483, -2.486, -2.489, -2.493, -2.496, -2.499, -2.502, -2.504, -2.507, -2.510, -2.513, -2.515, -2.518, -2.520, -2.523, -2.525, -2.527, -2.528, -2.528, -2.529, -2.529, -2.529, -2.529, -2.529, -2.529, -2.529, -2.529, -2.529, -2.529, -2.529, -2.529, -2.529, -2.529, -2.528, -2.528, -2.528, -2.528, -2.528, -2.527, -2.527, -2.527, -2.527, -2.527, -2.527, -2.527, -2.527, -2.527, -2.527, -2.528, -2.530, -2.531, -2.533, -2.534, -2.536, -2.538, -2.539, -2.541, -2.543, -2.544, -2.546, -2.548, -2.550, -2.552, -2.554, -2.556, -2.558, -2.560, -2.562, -2.564, -2.566, -2.568, -2.570, -2.573, -2.575, -2.577, -2.579, -2.581, -2.583, -2.585, -2.588, -2.590, -2.592, -2.594, -2.596, -2.598, -2.600, -2.603, -2.605, -2.607, -2.609, -2.611, -2.613, -2.615, -2.617, -2.619, -2.621, -2.623, -2.625, -2.626, -2.628, -2.630, -2.632, -2.634, -2.635, -2.637, -2.639, -2.640, -2.642, -2.644, -2.645, -2.647, -2.648, -2.649, -2.651, -2.652, -2.653, -2.654, -2.656, -2.656, -2.657, -2.657, -2.658, -2.658, -2.658, -2.658, -2.659, -2.659, -2.659, -2.659, -2.659, -2.659, -2.659, -2.659, -2.659, -2.659, -2.659, -2.659, -2.659, -2.658, -2.658, -2.658, -2.658, -2.658, -2.657, -2.657, -2.657, -2.657, -2.656, -2.656, -2.656, -2.655, -2.655, -2.655, -2.654, -2.654, -2.653, -2.653, -2.653, -2.652, -2.652, -2.651, -2.650, -2.650, -2.649, -2.649, -2.648, -2.648, -2.647, -2.647, -2.646, -2.646, -2.645, -2.645, -2.645, -2.644, -2.644, -2.644, -2.643, -2.643, -2.643, -2.643, -2.643, -2.643, -2.643, -2.643, -2.643, -2.643, -2.643, -2.643, -2.644, -2.644, -2.644, -2.645, -2.645, -2.646, -2.647, -2.647, -2.648, -2.649, -2.650, -2.651, -2.652, -2.655, -2.658, -2.660, -2.663, -2.666, -2.670, -2.673, -2.676, -2.679, -2.682, -2.686, -2.689, -2.692, -2.696, -2.699, -2.703, -2.706, -2.710, -2.713, -2.717, -2.720, -2.724, -2.727, -2.731, -2.734, -2.738, -2.742, -2.745, -2.749, -2.752, -2.756, -2.759, -2.763, -2.766, -2.769, -2.773, -2.776, -2.779, -2.783, -2.786, -2.789, -2.792, -2.795, -2.798, -2.801, -2.804, -2.807, -2.809, -2.810, -2.811, -2.812, -2.813, -2.814, -2.815, -2.816, -2.817, -2.818, -2.818, -2.819, -2.820, -2.820, -2.821, -2.821, -2.822, -2.822, -2.822, -2.823, -2.823, -2.823, -2.824, -2.824, -2.824, -2.824, -2.824, -2.824, -2.824, -2.824, -2.824, -2.824, -2.824, -2.824, -2.824, -2.824, -2.824, -2.824, -2.823, -2.823, -2.823, -2.823, -2.822, -2.822, -2.822, -2.822, -2.821, -2.821, -2.821, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.820, -2.821, -2.823, -2.825, -2.826, -2.828, -2.830, -2.832, -2.834, -2.836, -2.838, -2.840, -2.842, -2.844, -2.846, -2.848, -2.850, -2.852, -2.855, -2.857, -2.859, -2.862, -2.864, -2.866, -2.869, -2.871, -2.873, -2.876, -2.878, -2.881, -2.883, -2.886, -2.888, -2.891, -2.894, -2.896, -2.899, -2.901, -2.904, -2.907, -2.909, -2.912, -2.915, -2.917, -2.920, -2.923, -2.926, -2.928, -2.931, -2.934, -2.937, -2.939, -2.942, -2.945, -2.948, -2.951, -2.953, -2.956, -2.959, -2.962, -2.965, -2.968};
    
    std::vector<CurvePt> curvePts;
    for (int i = 0; i < frequencies.size(); ++i)
    {
        curvePts.emplace_back (-1, frequencies[i], amplitudes[i]);
    }
    
    targetCurve.updateWithCurvePts (curvePts);
}

void PlaybackManager::processBlock (juce::AudioBuffer<float>& ioBuffer)
{
    auto* leftChannel = ioBuffer.getWritePointer(0);
    auto* rightChannel = ioBuffer.getNumChannels() > 1 ? ioBuffer.getWritePointer(1) : nullptr;
    
    if (isCalibrating || isTesting)
    {
        for (int sample = 0; sample < ioBuffer.getNumSamples(); ++sample)
        {
            std::pair<float, float> value = getNextSample();
            value.first += pinkNoise.generate() * 8.0;
            value.second += pinkNoise.generate() * 8.0;
            leftChannel[sample] = value.first * 0.05 * 0.5  * juce::Decibels::decibelsToGain (referenceVolume);
            
            if (rightChannel)
                rightChannel[sample] = value.second * 0.05 * 0.5  * juce::Decibels::decibelsToGain (referenceVolume);
        }
    }
    else if (isSweeping)
    {
        for (int sample = 0; sample < ioBuffer.getNumSamples(); ++sample)
        {
            const std::pair<float, float> value = sineSweepGenerator.getNextSample();
            float referenceGain = juce::Decibels::decibelsToGain (referenceVolume);
            referenceGain *= juce::Decibels::decibelsToGain (getCompensationDBAtFrequency (sineSweepGenerator.getCurrFreq()));
            
            leftChannel[sample] = value.first * 0.05 * 0.5 * referenceGain;
            
            if (rightChannel)
                rightChannel[sample] = value.second * 0.05 * 0.5 * referenceGain;
        }
    }
    else
    {
        auto numSamples = ioBuffer.getNumSamples();
        
        juce::dsp::AudioBlock<float> ioBlock (ioBuffer);
        auto ioContext = juce::dsp::ProcessContextReplacing<float> (ioBlock);
        if ((isProcessing && hasPreparedFilter) || isSweeping)
        {
            filter.process (ioContext);
            wetGainProcessor.process (ioContext);
        }
        else
        {
            dryGainProcessor.process (ioContext);
        }
    }
}

void PlaybackManager::updateFilterWithCurves (Curve& amplCurve, Curve& panCurve)
{
    filter.updateWithCurves (amplCurve, panCurve, FFT_SIZE);
}

void PlaybackManager::prepare (const juce::dsp::ProcessSpec& spec)
{
    filter.prepare (spec);
    arbitrarySequencer.setSampleRate (spec.sampleRate);
    arbitrarySequencer2.setSampleRate (spec.sampleRate);
    hasPreparedFilter = true;
}

float PlaybackManager::getCurrPlayingFreq() const
{
    return arbitrarySequencer.currentlyPlayingFrequency();
}

float PlaybackManager::getCurrTestingFreq() const
{
    return testingFreq;
}

float PlaybackManager::getCurrSineSweepFreq() const
{
    return sineSweepGenerator.getCurrFreq();
}

void PlaybackManager::setIsTesting (bool isTesting)
{
    this->isTesting = isTesting;
}

void PlaybackManager::setIsSweeping (bool isSweeping)
{
    this->isSweeping = isSweeping;
}

void PlaybackManager::setIsCalibrating (bool isCalibrating)
{
    this->isCalibrating = isCalibrating;
}

void PlaybackManager::setIsProcessing (bool isProcessing)
{
    this->isProcessing = isProcessing;
}

void PlaybackManager::setDryWetVolumeBalance (float balance)
{
    dryGainProcessor.setGainDecibels (-balance);
    wetGainProcessor.setGainDecibels (+balance);
}

void PlaybackManager::setSineSweepCenterFrequency (float centerFreq, std::optional<float> ampl)
{
//    sineSweepGenerator.setCenterFrequency (centerFreq, ampl);
}

void PlaybackManager::updateSineSweepCenterFrequency (float centerFreq, std::optional<float> ampl)
{
//    sineSweepGenerator.updateCenterFrequency (centerFreq, ampl);
}

void PlaybackManager::startPlayingFreq (float freq, Curve& amplCurve, Curve& panCurve)
{
    // Play the reference note and controlled note, alternating between left and right
    int noteDurationInSamples = 8000;
    float ampl = amplCurve.valueAtFrequency (freq);
    float pan = panCurve.valueAtFrequency (freq);
    ampl = juce::Decibels::gainToDecibels (ampl);
    pan = juce::Decibels::gainToDecibels (pan);
    ampl += getCompensationDBAtFrequency (freq);
    
    float dbDifference = getReferenceCompensationDBAtFrequency (freq);
    
    SequenceableNote refNote (referenceNote.frequency, referenceNote.amplitude + dbDifference, 0.0f, noteDurationInSamples);
    SequenceableNote controlledNote (freq, ampl, pan, noteDurationInSamples);
//    SequenceableNote silentNote (0.0f, 0.0f, 0.0f, noteDurationInSamples, StereoGainEnvelope::silent());
    arbitrarySequencer.setNotesForSpatialCalibration ({ refNote, controlledNote });
}

// TODO: Add panning here
void PlaybackManager::updatePlayingFreq (float freq, Curve& amplCurve, Curve& panCurve)
{
    int noteDurationInSamples = 8000;
    float ampl = amplCurve.valueAtFrequency (freq);
    float pan = panCurve.valueAtFrequency (freq);
    ampl = juce::Decibels::gainToDecibels (ampl);
    pan = juce::Decibels::gainToDecibels (pan);
    ampl += getCompensationDBAtFrequency (freq);
    
    float dbDifference = getReferenceCompensationDBAtFrequency (freq);
    
    SequenceableNote refNote (referenceNote.frequency, referenceNote.amplitude + dbDifference, 0.0f, noteDurationInSamples);
    SequenceableNote controlledNote (freq, ampl, pan, noteDurationInSamples);
    arbitrarySequencer.updateNotesForSpatialCalibration({ refNote, controlledNote });
}

void PlaybackManager::startTestingFreq (float freq, Curve& curve)
{
//    if (isTesting)
//    {
//        updateTestingFreq (freq, curve);
//        return;
//    }
//    
//    int noteDurationInSamples = 20000;
//    isTesting = true;
//    
//    float ampl = juce::Decibels::gainToDecibels (curve.valueAtFrequency (freq));
//    ampl += getCompensationDBAtFrequency (freq);
//    
//    Note referenceNoteCompensated = referenceNote;
//    referenceNoteCompensated.amplitude += getCompensationDBAtFrequency (freq);
//    referenceNoteCompensated.amplitude += getReferenceCompensationDBAtFrequency (freq);
//    
//    auto nodeBelow = curve.nodeBelowFreq (freq);
//    auto nodeAbove = curve.nodeAboveFreq (freq);
//    
//    if (! nodeBelow.has_value() || ! nodeAbove.has_value())
//    {
//        // for now, do nothing
//        return;
//    }
//    
//    auto [freqBelow, amplBelow] = nodeBelow.value();
//    auto [freqAbove, amplAbove] = nodeAbove.value();
//    amplBelow += getCompensationDBAtFrequency (freqBelow);
//    amplAbove += getCompensationDBAtFrequency (freqAbove);
//    
//    SequenceableNote noteBelow (freqBelow, amplBelow, 0.0f, noteDurationInSamples);
//    SequenceableNote noteMid (freq, ampl, 0.0f, noteDurationInSamples);
//    SequenceableNote noteAbove (freqAbove, amplAbove, 0.0f, noteDurationInSamples);
//    SequenceableNote silentNote (0.0f, 0.0f, noteDurationInSamples, StereoGainEnvelope (StereoGainEnvelopeType::SILENT));
//    arbitrarySequencer.setNotes ({ noteBelow, noteMid, noteAbove, silentNote });
//    
//    testingFreq = freq;
}

void PlaybackManager::updateTestingFreq (float freq, Curve& curve)
{
    /*
    int noteDurationInSamples = 20000;
    
    float ampl = juce::Decibels::gainToDecibels (curve.valueAtFrequency (freq).first.real());
    ampl += getCompensationDBAtFrequency (freq);
    
    Note referenceNoteCompensated = referenceNote;
    referenceNoteCompensated.amplitude += getCompensationDBAtFrequency (freq);
    referenceNoteCompensated.amplitude += getReferenceCompensationDBAtFrequency (freq);
    
//    SequenceableNote note1 (referenceNoteCompensated, noteDurationInSamples);
//    SequenceableNote note2 (freq, ampl, 0.0f, 0.0f, noteDurationInSamples);
//    arbitrarySequencer.changeNoteAtIdx (0, note1.note());
//    arbitrarySequencer.changeNoteAtIdx (1, note2.note());
    
//    float bandwidth = 1.05;
//    float freqBelow = freq / bandwidth;
//    float freqAbove = freq * bandwidth;
//    float amplBelow = juce::Decibels::gainToDecibels (curve.valueAtFrequency (freqBelow).first.real()) + getCompensationDBAtFrequency (freqBelow);
//    float amplAbove = juce::Decibels::gainToDecibels (curve.valueAtFrequency (freqAbove).first.real()) + getCompensationDBAtFrequency (freqAbove);
//    
//    SequenceableNote noteBelow (freqBelow, amplBelow, 0.0f, 0.0f, noteDurationInSamples);
//    SequenceableNote noteMid (freq, ampl, 0.0f, 0.0f, noteDurationInSamples);
//    SequenceableNote noteAbove (freqAbove, amplAbove, 0.0f, 0.0f, noteDurationInSamples);
//    
//    arbitrarySequencer.setNotes ({ noteBelow, noteMid, noteAbove });
    
    auto nodeBelow = curve.nodeBelowFreq (freq);
    auto nodeAbove = curve.nodeAboveFreq (freq);
    
    if (! nodeBelow.has_value() || ! nodeAbove.has_value())
    {
        // for now, do nothing
        return;
    }
    
    auto [freqBelow, amplBelow] = nodeBelow.value();
    auto [freqAbove, amplAbove] = nodeAbove.value();
    amplBelow += getCompensationDBAtFrequency (freqBelow);
    amplAbove += getCompensationDBAtFrequency (freqAbove);
    
    SequenceableNote noteBelow (freqBelow, amplBelow, noteDurationInSamples);
    SequenceableNote noteMid (freq, ampl, noteDurationInSamples);
    SequenceableNote noteAbove (freqAbove, amplAbove, noteDurationInSamples);
    SequenceableNote silentNote (0.0f, 0.0f, noteDurationInSamples, StereoGainEnvelope (StereoGainEnvelopeType::SILENT));
    arbitrarySequencer.setNotes ({ noteBelow, noteMid, noteAbove, silentNote });
    
    testingFreq = freq;
    */
}

void PlaybackManager::startSineSweep (float centerFreq, Curve amplCurve, Curve panCurve)
{
    sineSweepGenerator.setSweep (centerFreq, amplCurve, panCurve);
}

void PlaybackManager::updateSineSweep (float centerFreq, Curve amplCurve, Curve panCurve)
{
    sineSweepGenerator.updateSweep (centerFreq, amplCurve, panCurve);
}

void PlaybackManager::stopTestingFreq()
{
    isTesting = false;
//    arbitrarySequencer.setNotes ({ SequenceableNote (referenceNote, 25000) });
}

void PlaybackManager::setReferenceVolume (float volume)
{
    this->referenceVolume = volume;
}

void PlaybackManager::setReferencePan (float pan)
{
    this->referencePan = pan;
    this->leftRefNote.amplitude = referenceNote.amplitude - 0.5 * pan;
    this->rightRefNote.amplitude = referenceNote.amplitude + 0.5 * pan;
}

std::pair<float, float> PlaybackManager::getNextSample()
{
    auto [leftSample1, rightSample1] = arbitrarySequencer.getNextSample();
    auto [leftSample2, rightSample2] = arbitrarySequencer2.getNextSample();
    return { leftSample1 + leftSample2, rightSample1 + rightSample2 };
}

float PlaybackManager::getCompensationDBAtFrequency (float frequency)
{
    return -4.5f * std::log2 (frequency / 1000.0f);
}

float PlaybackManager::getReferenceCompensationDBAtFrequency (float frequency)
{
    return juce::Decibels::gainToDecibels (targetCurve.valueAtFrequency (frequency));
}

juce::dsp::IIR::Coefficients<float>::Ptr PlaybackManager::createDelayCoefficients(float sampleRate, float delaytime) const
{
    // Basic first order all pass filter
    float a = (1.0f - delaytime * 0.5f * sampleRate) / (1.0f + delaytime * 0.5f * sampleRate);
    juce::dsp::IIR::Coefficients<float>::Ptr coefs(new juce::dsp::IIR::Coefficients<float>(a * a, 2.0f * a, 1.0f, 1.0f, 2.0f * a, a * a));
    return coefs;
}
