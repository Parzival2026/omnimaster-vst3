# OmniMaster Pro — All-in-One Mixing & Mastering VST3 Plugin
**Target OS:** Windows 11 (64-bit)  
**Target DAW:** Fender Studio Pro / Studio One Pro (and any VST3 host)

## 6-Stage Signal Chain Architecture
1. **5-Band Mastering Parametric EQ:**
   - Resonant 12dB/oct Sub-Bass High-Pass Filter (`15 Hz – 120 Hz`) to clean up rumble
   - Warm Low Shelf (`40 Hz – 250 Hz`), Mud Cleanup Bell (`150 Hz – 800 Hz`), Presence Bell (`1 kHz – 6 kHz`), and Silk Air High Shelf (`8 kHz – 18 kHz`)
2. **3-Band Linkwitz-Riley Multiband Glue Compressor:**
   - Phase-coherent 24dB/oct Linkwitz-Riley crossovers splitting Sub/Low, Midrange, and Highs
   - Independent Low, Mid, and High thresholds with unified Glue Ratio and Makeup Gain
3. **Analog Harmonic Exciter (Tube & Tape Saturation):**
   - Blendable Even-Order (Asymmetric Triode Tube) and Odd-Order (`tanh` Magnetic Tape) harmonic generation with built-in DC offset filtering and parallel Dry/Wet mix
4. **Mid/Side Stereo Imager + Mono Sub-Bass Maker:**
   - Encodes stereo to Mid/Side, high-passes the Side channel below your chosen cutoff (`20 Hz – 300 Hz`) so sub-bass stays 100% phase-locked in mono, and expands high-frequency stereo width up to `200%`
5. **Soft Clipper + True-Peak Brickwall Maximizer:**
   - Pre-limiter Arctangent Soft Clipper shaves transient spikes transparently before hitting the Brickwall Lookahead Maximizer (`-0.8 dBTP` default streaming ceiling)
6. **Real-Time LUFS, True-Peak & Stereo Phase Correlation Metering**

---

## How to Get Your `.vst3` File on Windows 11 (Two Easy Options)

### Option A: Zero-Install Cloud Build via GitHub (Recommended if you don't have Visual Studio installed)
1. Create a free repository on [GitHub](https://github.com) and drag-and-drop all files from this ZIP into the repository (making sure the `.github/workflows/build_vst3_windows.yml` file is included).
2. Click the **Actions** tab in your GitHub repository. GitHub's cloud Windows server will automatically compile the C++ code into a `.vst3` binary in ~3 minutes.
3. Click the completed workflow run and download **`OmniMasterPro-Windows11-VST3.zip`**.
4. Extract **`OmniMaster Pro.vst3`** into `C:\Program Files\Common Files\VST3\`.

### Option B: One-Click Local Build on Windows 11
1. Install **Visual Studio 2022 Community** (with "Desktop development with C++") and **CMake**.
2. Double-click **`BUILD_WINDOWS_VST3.bat`**.
3. Copy the generated `build\OmniMasterPro_artefacts\Release\VST3\OmniMaster Pro.vst3` folder to:
   `C:\Program Files\Common Files\VST3\`
4. Open **Fender Studio Pro** — it will scan `C:\Program Files\Common Files\VST3\` on startup and list **OmniMaster Pro** under **Effects -> StudioProAudio**.
