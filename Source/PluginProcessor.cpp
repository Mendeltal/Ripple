#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

// ============================================================ DSP helpers
void rp::Biquad::set (double B0, double B1, double B2, double A0, double A1, double A2)
{
    b0 = (float) (B0 / A0); b1 = (float) (B1 / A0); b2 = (float) (B2 / A0); a1 = (float) (A1 / A0); a2 = (float) (A2 / A0);
}
void rp::Biquad::lowpass (double sr, double f, double q)
{
    f = jlimit (20.0, sr * 0.45, f);
    const double w = 2.0 * MathConstants<double>::pi * f / sr, c = std::cos (w), s = std::sin (w), al = s / (2.0 * q);
    set ((1.0 - c) * 0.5, 1.0 - c, (1.0 - c) * 0.5, 1.0 + al, -2.0 * c, 1.0 - al);
}
void rp::Biquad::highpass (double sr, double f, double q)
{
    f = jlimit (10.0, sr * 0.45, f);
    const double w = 2.0 * MathConstants<double>::pi * f / sr, c = std::cos (w), s = std::sin (w), al = s / (2.0 * q);
    set ((1.0 + c) * 0.5, -(1.0 + c), (1.0 + c) * 0.5, 1.0 + al, -2.0 * c, 1.0 - al);
}
void rp::Biquad::bandpass (double sr, double f, double q)
{
    f = jlimit (20.0, sr * 0.45, f);
    const double w = 2.0 * MathConstants<double>::pi * f / sr, c = std::cos (w), s = std::sin (w), al = s / (2.0 * q);
    set (al, 0.0, -al, 1.0 + al, -2.0 * c, 1.0 - al);
}
void rp::Biquad::lowShelf (double sr, double f, double gdb)
{
    f = jlimit (20.0, sr * 0.45, f);
    const double A = std::pow (10.0, gdb / 40.0), w = 2.0 * MathConstants<double>::pi * f / sr, c = std::cos (w), s = std::sin (w);
    const double al = s * 0.5 * std::sqrt (2.0), sa = 2.0 * std::sqrt (A) * al;
    set (A * ((A + 1) - (A - 1) * c + sa), 2 * A * ((A - 1) - (A + 1) * c), A * ((A + 1) - (A - 1) * c - sa),
         (A + 1) + (A - 1) * c + sa, -2 * ((A - 1) + (A + 1) * c), (A + 1) + (A - 1) * c - sa);
}
void rp::Biquad::highShelf (double sr, double f, double gdb)
{
    f = jlimit (20.0, sr * 0.45, f);
    const double A = std::pow (10.0, gdb / 40.0), w = 2.0 * MathConstants<double>::pi * f / sr, c = std::cos (w), s = std::sin (w);
    const double al = s * 0.5 * std::sqrt (2.0), sa = 2.0 * std::sqrt (A) * al;
    set (A * ((A + 1) + (A - 1) * c + sa), -2 * A * ((A - 1) + (A + 1) * c), A * ((A + 1) + (A - 1) * c - sa),
         (A + 1) - (A - 1) * c + sa, 2 * ((A - 1) - (A + 1) * c), (A + 1) - (A - 1) * c - sa);
}

void rp::FDN::prepare (double s)
{
    sampleRate = s;
    static const int base[N] = { 1327, 1559, 1787, 2053, 2371, 2749, 3109, 3539 };
    for (int i = 0; i < N; ++i)
    {
        len[i] = (int) (base[i] * (s / 44100.0));
        line[i].assign ((size_t) ((float) len[i] * 1.8f) + 96, 0.f);
        wpos[i] = 0; lp[i] = 0.f; phase[i] = (float) i / (float) N;
        rate[i] = (0.17f + 0.043f * (float) i) / (float) s;
    }
    static const int apBase[4] = { 142, 107, 379, 277 };
    for (int i = 0; i < 4; ++i)
    {
        apLen[i] = jmax (8, (int) (apBase[i] * (s / 44100.0)));
        apBuf[i].assign ((size_t) apLen[i] * 2, 0.f); apPos[i] = 0;
    }
    preLen = (int) (0.012 * s);
    for (auto& p : pre) p.assign ((size_t) preLen + 1, 0.f);
    prePos = 0; scale = 1.f; setDecay (3.f);
}

void rp::FDN::setDecay (float t)
{
    for (int i = 0; i < N; ++i)
        gain[i] = std::pow (10.f, -3.f * ((float) len[i] * scale / (float) sampleRate) / jmax (0.05f, t));
}

void rp::FDN::tick (float inL, float inR, float& outL, float& outR)
{
    pre[0][(size_t) prePos] = inL; pre[1][(size_t) prePos] = inR;
    const int rpp = (prePos + 1) % (preLen + 1);
    float x[2] = { pre[0][(size_t) rpp], pre[1][(size_t) rpp] };
    prePos = (prePos + 1) % (preLen + 1);
    for (int a = 0; a < 4; ++a)
        for (int c = 0; c < 2; ++c)
        {
            float& slot = apBuf[a][(size_t) (c * apLen[a] + apPos[a])];
            const float d = slot, y = -0.6f * x[c] + d;
            slot = x[c] + 0.6f * y; x[c] = y;
        }
    for (int a = 0; a < 4; ++a) apPos[a] = (apPos[a] + 1) % apLen[a];

    float v[N];
    for (int i = 0; i < N; ++i)
    {
        phase[i] += rate[i]; if (phase[i] >= 1.f) phase[i] -= 1.f;
        const float d = (float) len[i] * scale - 3.f + 3.f * std::sin (6.2831853f * phase[i]);
        const int n = (int) line[i].size();
        float rp2 = (float) wpos[i] - d; while (rp2 < 0) rp2 += (float) n;
        const int i0 = (int) rp2; const float fr = rp2 - (float) i0;
        const float s0 = line[i][(size_t) (i0 % n)], s1 = line[i][(size_t) ((i0 + 1) % n)];
        float y = s0 + (s1 - s0) * fr;
        lp[i] += (y - lp[i]) * (1.f - damp); y = lp[i];
        v[i] = y;
    }
    outL = 0.32f * (v[0] - v[2] + v[4] - v[6] + v[1]);
    outR = 0.32f * (v[1] - v[3] + v[5] - v[7] + v[0]);
    float m[N]; for (int i = 0; i < N; ++i) m[i] = v[i];
    for (int h = 1; h < N; h <<= 1)
        for (int i = 0; i < N; i += h << 1)
            for (int j = i; j < i + h; ++j) { const float a = m[j], b = m[j + h]; m[j] = a + b; m[j + h] = a - b; }
    const float norm = 0.35355339f;
    for (int i = 0; i < N; ++i)
    {
        float w = x[i & 1] * 0.35f + gain[i] * m[i] * norm;
        if (std::abs (w) < 1.0e-20f) w = 0.f;
        line[i][(size_t) wpos[i]] = w;
        wpos[i] = (wpos[i] + 1) % (int) line[i].size();
    }
}

float rp::PitchShifter::process (float x)
{
    buf[(size_t) wp] = x;
    float out = 0.f;
    for (int h = 0; h < 2; ++h)
    {
        float p = ph + 0.5f * (float) h; if (p >= 1.f) p -= 1.f;
        const float d = p * (float) win + 2.f, rpos = (float) wp - d;
        const int i0 = (int) std::floor (rpos); const float fr = rpos - (float) i0;
        const float a = buf[(size_t) (i0 & mask)], b = buf[(size_t) ((i0 + 1) & mask)];
        const float s = std::sin (3.14159265f * p);
        out += (a + (b - a) * fr) * s * s;
    }
    ph += (1.f - ratio) / (float) win; ph -= std::floor (ph);
    wp = (wp + 1) & mask;
    return out;
}

// ============================================================ Parameters
AudioProcessorValueTreeState::ParameterLayout RippleProcessor::createLayout()
{
    AudioProcessorValueTreeState::ParameterLayout l;
    auto B = [&] (const char* id, const char* n, bool d) { l.add (std::make_unique<AudioParameterBool> (ParameterID { id, 1 }, n, d)); };
    auto F = [&] (const char* id, const char* n, float mx, float d)
        { l.add (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, n, NormalisableRange<float> (0.f, mx), d)); };
    auto C = [&] (const char* id, const char* n, StringArray s, int d)
        { l.add (std::make_unique<AudioParameterChoice> (ParameterID { id, 1 }, n, s, d)); };

    F ("master", "Master Mix", 1.f, 1.f);
    B ("link", "Link FX Mix", false); B ("sync", "Tempo Sync", true); B ("bypass", "Bypass", false);
    C ("vibe", "Vibe", { "Ice", "Hot", "Warm", "Air" }, 0);
    B ("dlOn", "Delay On", true); B ("rvOn", "Reverb On", true); B ("tpOn", "Tape On", true); B ("lfOn", "Lo-Fi On", true); B ("modOn", "Mod On", true);
    F ("dlTime", "Delay Time", 1.f, 10.f / 13.f); F ("dlFb", "Delay Feedback", 0.95f, 0.4f); F ("dlTone", "Delay Tone", 1.f, 0.5f); F ("dlMix", "Delay Mix", 1.f, 0.45f);
    F ("rvSize", "Reverb Size", 1.f, 0.55f); F ("rvDecay", "Reverb Decay", 1.f, 0.55f); F ("rvShim", "Reverb Shimmer", 1.f, 0.f); F ("rvMix", "Reverb Mix", 1.f, 0.25f);
    F ("tpWow", "Tape Wow", 1.f, 0.f); F ("tpFlut", "Tape Flutter", 1.f, 0.f); F ("tpDrive", "Tape Drive", 1.f, 0.f); F ("tpMix", "Tape Mix", 1.f, 0.f);
    F ("lfCrush", "Lo-Fi Crush", 1.f, 0.f); F ("lfRate", "Lo-Fi Rate", 1.f, 0.f); F ("lfDust", "Lo-Fi Dust", 1.f, 0.f);
    F ("lfRadio", "Lo-Fi Radio", 1.f, 0.f); F ("lfWarp", "Lo-Fi Warp", 1.f, 0.f);
    F ("l1Rate", "LFO1 Rate", 1.f, 0.4f); F ("l1Depth", "LFO1 Depth", 1.f, 0.f); C ("l1Dest", "LFO1 Dest", rp::destNames(), 1);
    F ("l2Rate", "LFO2 Rate", 1.f, 0.3f); F ("l2Depth", "LFO2 Depth", 1.f, 0.f); C ("l2Dest", "LFO2 Dest", rp::destNames(), 4);
    F ("foAmt", "Follower Amount", 1.f, 0.f); F ("foSpeed", "Follower Speed", 1.f, 0.35f); C ("foDest", "Follower Dest", rp::destNames(), 8);
    F ("drAmt", "Drift Amount", 1.f, 0.f); F ("drRate", "Drift Rate", 1.f, 0.3f); C ("drDest", "Drift Dest", rp::destNames(), 17);
    return l;
}

RippleProcessor::RippleProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", AudioChannelSet::stereo(), true).withOutput ("Output", AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "RIPPLE", createLayout())
{
}

bool RippleProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    return l.getMainInputChannelSet() == AudioChannelSet::stereo() && l.getMainOutputChannelSet() == AudioChannelSet::stereo();
}

void RippleProcessor::prepareToPlay (double s, int)
{
    sr = s;
    for (auto& t : tapeLine) t.init ((int) (0.05 * s));
    for (auto& d : dly) d.init ((int) (6.6 * s));
    for (auto& w : warpLine) w.init ((int) (0.04 * s));
    fdn.prepare (s);
    for (auto& p : ps) p.prepare();
    for (int c = 0; c < 2; ++c)
    {
        tapeLp[c].reset(); tapeBump[c].reset(); fbLp[c].reset(); fbHp[c].reset(); rvHp[c].reset(); radHp[c].reset(); radLp[c].reset();
        vibeA[c].reset(); vibeB[c].reset(); tapeDc[c] = 0.f; warpPh[c] = 0.f;
        tapePh[c][0] = 0.25f * (float) c; tapePh[c][1] = 0.31f * (float) c; tapePh[c][2] = 0.4f * (float) c;
        rvHp[c].highpass (s, 120.0); radHp[c].highpass (s, 20.0); radLp[c].lowpass (s, 20000.0);
    }
    dustHp.highpass (s, 800.0);
    shimL = shimR = 0.f; holdL = holdR = holdPh = 0.f; follower = 0.f; drift = driftTarget = 0.f;
    lastVibe = -1; lastTone = lastDecay = -1.f;
    for (auto* m : { &smDlMix, &smRvMix, &smTpMix, &smMaster, &smDlOn, &smRvOn, &smTpOn, &smLfOn, &smBypass, &smFb, &smDrive, &smDepthTape, &smShim })
        m->setTime (s, 0.02f);
    smDlTime.setTime (s, 0.09f);
    for (auto& m : smLf) m.setTime (s, 0.03f);
    smMaster.snap (raw ("master")); smDlMix.snap (0); smRvMix.snap (0); smTpMix.snap (0);
    smDlOn.snap (1); smRvOn.snap (1); smTpOn.snap (1); smLfOn.snap (1); smBypass.snap (raw ("bypass"));
    smFb.snap (0.4f); smDrive.snap (0); smDepthTape.snap (0); smShim.snap (0);
    smDlTime.snap ((float) (0.5 * s)); dlTimeCur = (float) (0.5 * s);
    for (auto& m : smLf) m.snap (0);
}

// ============================================================ process
void RippleProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer&)
{
    ScopedNoDenormals nd;
    const int N = buffer.getNumSamples();
    if (buffer.getNumChannels() < 2) return;
    float* L = buffer.getWritePointer (0);
    float* R = buffer.getWritePointer (1);

    double bpm = 120.0;
    if (auto* ph = getPlayHead()) if (auto pos = ph->getPosition()) if (auto b = pos->getBpm()) if (*b > 20.0) bpm = *b;

    const bool sync = raw ("sync") > 0.5f, modOn = raw ("modOn") > 0.5f;
    const int vibe = (int) std::lround (raw ("vibe"));
    smDlOn.tgt = raw ("dlOn") > 0.5f ? 1.f : 0.f; smRvOn.tgt = raw ("rvOn") > 0.5f ? 1.f : 0.f;
    smTpOn.tgt = raw ("tpOn") > 0.5f ? 1.f : 0.f; smLfOn.tgt = raw ("lfOn") > 0.5f ? 1.f : 0.f;
    smBypass.tgt = raw ("bypass") > 0.5f ? 1.f : 0.f;

    const float pDlTime = raw ("dlTime"), pFb = raw ("dlFb") / 0.95f, pTone = raw ("dlTone"), pDlMix = raw ("dlMix");
    const float pSize = raw ("rvSize"), pDecay = raw ("rvDecay"), pShim = raw ("rvShim"), pRvMix = raw ("rvMix");
    const float pWow = raw ("tpWow"), pFlut = raw ("tpFlut"), pDrive = raw ("tpDrive"), pTpMix = raw ("tpMix");
    const float pLf[5] = { raw ("lfCrush"), raw ("lfRate"), raw ("lfDust"), raw ("lfRadio"), raw ("lfWarp") };
    const float pMaster = raw ("master");
    const float l1Hz = rp::lfoHz (raw ("l1Rate")), l2Hz = rp::lfoHz (raw ("l2Rate")), l1D = raw ("l1Depth"), l2D = raw ("l2Depth");
    const int l1Dst = (int) std::lround (raw ("l1Dest")), l2Dst = (int) std::lround (raw ("l2Dest")),
              foDst = (int) std::lround (raw ("foDest")), drDst = (int) std::lround (raw ("drDest"));
    const float foAmt = raw ("foAmt"), drAmt = raw ("drAmt");
    const float foRel = 0.03f + 1.5f * raw ("foSpeed") * raw ("foSpeed");
    const float foK = std::exp (-1.f / (float) (sr * foRel));
    const float drivePk = 0.f; juce::ignoreUnused (drivePk);

    // vibe voicing
    float vibeDamp = 0.f, vibeDecayMul = 1.f, vibeWowMul = 1.f;
    if (vibe != lastVibe)
    {
        lastVibe = vibe;
        for (int c = 0; c < 2; ++c)
        {
            vibeA[c].b0 = 1; vibeA[c].b1 = vibeA[c].b2 = vibeA[c].a1 = vibeA[c].a2 = 0;
            vibeB[c] = vibeA[c];
            if (vibe == 1) { vibeA[c].highpass (sr, 110.0); vibeB[c].highShelf (sr, 3500.0, 1.5); }
            if (vibe == 2) { vibeA[c].lowShelf (sr, 180.0, 1.2); vibeB[c].highShelf (sr, 5000.0, -3.2); }
            if (vibe == 3) { vibeA[c].highShelf (sr, 9000.0, 2.2); vibeB[c].lowShelf (sr, 200.0, -1.0); }
        }
        vibeDrive = vibe == 1 ? 1.15f : (vibe == 2 ? 1.08f : 0.f);
    }
    if (vibe == 1) vibeDamp = 0.04f; if (vibe == 2) { vibeDamp = 0.07f; vibeWowMul = 1.2f; }
    if (vibe == 3) { vibeDamp = -0.07f; vibeDecayMul = 1.1f; }

    float pk = 0.f, pkOut = 0.f;
    const double twoPi = 6.283185307179586;
    float modWow = 0.f, modFlut = 0.f, pitchMod = 0.f;   // control-rate values used per-sample
    float crushV = 0.f, rateV = 0.f, dustV = 0.f, radV = 0.f, warpV = 0.f;
    float toneLast = lastTone; float lastRadio = -1.f, lastDriveForLp = -1.f;
    juce::ignoreUnused (lastRadio, lastDriveForLp);

    for (int n = 0; n < N; ++n)
    {
        // -------------------------------- control rate (every 16 samples)
        if ((n & 15) == 0)
        {
            float acc[rp::numDest] {};
            if (modOn)
            {
                const float v1 = std::sin ((float) (twoPi * lfoPh[0])), v2 = std::sin ((float) (twoPi * lfoPh[1]));
                lfoPh[0] += (float) (16.0 * l1Hz / sr); lfoPh[0] -= std::floor (lfoPh[0]);
                lfoPh[1] += (float) (16.0 * l2Hz / sr); lfoPh[1] -= std::floor (lfoPh[1]);
                const float fN = jlimit (0.f, 1.f, (20.f * std::log10 (jmax (follower, 1.0e-5f)) + 50.f) / 50.f);
                if (--driftCount <= 0)
                {
                    driftCount = (int) (sr * (0.15 + 2.2 * (1.0 - raw ("drRate"))));
                    driftTarget = rng.nextFloat() * 2.f - 1.f;
                }
                drift += (driftTarget - drift) * (float) (16.0 / (sr * (0.1 + 1.6 * (1.0 - raw ("drRate")))));
                acc[l1Dst] += v1 * l1D * 0.5f; acc[l2Dst] += v2 * l2D * 0.5f;
                acc[foDst] += fN * foAmt * 0.8f; acc[drDst] += drift * drAmt * 0.5f;
            }
            acc[0] = 0.f;
            auto m = [&] (int d, float base) { return jlimit (0.f, 1.f, base + acc[d]); };

            pitchMod = acc[17];
            // delay time
            const int idx = jlimit (0, 13, (int) std::lround (pDlTime * 13.f));
            double secs = sync ? rp::divBeats()[(size_t) idx] * 60.0 / bpm : rp::freeMs (pDlTime) * 0.001;
            secs *= std::pow (2.f, acc[1] * 2.f) * (1.f + pitchMod * 0.02f);
            smDlTime.tgt = (float) jlimit (0.002, 6.4, secs) * (float) sr;
            const float fbN = m (2, pFb); smFb.tgt = fbN * 0.95f;
            const float toneN = m (3, pTone);
            if (std::abs (toneN - toneLast) > 0.002f)
            {
                toneLast = toneN;
                const double lpF = toneN < 0.5f ? 18000.0 * std::pow (0.08, (0.5 - toneN) * 2.0) : 18000.0;
                const double hpF = toneN > 0.5f ? 20.0 * std::pow (30.0, (toneN - 0.5) * 2.0) : 20.0;
                for (int c = 0; c < 2; ++c) { fbLp[c].lowpass (sr, lpF); fbHp[c].highpass (sr, hpF); }
            }
            smDlMix.tgt = m (9, pDlMix); smRvMix.tgt = m (10, pRvMix); smTpMix.tgt = m (11, pTpMix);
            smMaster.tgt = m (15, pMaster);
            // reverb
            fdn.setSize (0.6f + 1.1f * m (4, pSize));
            const float dN = m (5, pDecay);
            fdn.setDecay (rp::decaySeconds (dN) * vibeDecayMul);
            fdn.setDamping (jlimit (0.02f, 0.9f, 0.30f + vibeDamp));
            smShim.tgt = m (6, pShim);
            // tape
            modWow = m (7, pWow) * 7.f * vibeWowMul + pitchMod * 4.f;
            modFlut = pFlut * 1.4f;
            smDrive.tgt = m (8, pDrive);
            // lo-fi
            smLf[0].tgt = m (12, pLf[0]); smLf[1].tgt = m (13, pLf[1]); smLf[2].tgt = m (14, pLf[2]);
            smLf[3].tgt = pLf[3]; smLf[4].tgt = m (16, pLf[4]);
            // slow-changing filters
            const float dr = smDrive.cur;
            for (int c = 0; c < 2; ++c)
            {
                tapeLp[c].lowpass (sr, 16000.0 - 7500.0 * dr);
                tapeBump[c].lowShelf (sr, 100.0, 1.6 * dr);
                radHp[c].highpass (sr, 20.0 * std::pow (40.0, smLf[3].cur));
                radLp[c].lowpass (sr, 20000.0 * std::pow (0.14, smLf[3].cur));
            }
            fdn.setDecay (rp::decaySeconds (dN) * vibeDecayMul);
        }

        float xl = L[n], xr = R[n];
        const float dryL = xl, dryR = xr;
        pk = jmax (pk, std::abs (xl), std::abs (xr));
        follower = jmax (follower * (std::abs (xl) + std::abs (xr) > 0.f ? foK : foK), 0.5f * (std::abs (xl) + std::abs (xr)));

        // smooth
        const float tpMix = smTpMix.next() * smTpOn.next(), dlMix = smDlMix.next() * smDlOn.next(), rvMix = smRvMix.next() * smRvOn.next();
        const float master = smMaster.next(), fb = smFb.next(), drive = smDrive.next(), shimAmt = smShim.next();
        const float dlOn = smDlOn.cur, rvOn = smRvOn.cur, lfOn = smLfOn.next(), byp = smBypass.next();
        dlTimeCur = smDlTime.next();
        float lf[5]; for (int i = 0; i < 5; ++i) lf[i] = smLf[i].next();

        // -------------------------------- TAPE
        {
            const float g = 1.f + 7.f * std::pow (drive, 1.4f), mk = 1.f / (1.f + 0.35f * (g - 1.f));
            float* in[2] = { &xl, &xr };
            for (int c = 0; c < 2; ++c)
            {
                float x = *in[c];
                float s = std::tanh (g * x + 0.10f * drive * g * x * x) * mk * (g > 1.f ? 1.f : 1.f);
                if (drive < 0.001f) s = x;
                tapeDc[c] += (s - tapeDc[c]) * 0.0008f; s -= tapeDc[c];
                // modulation
                for (int k = 0; k < 3; ++k) { const float hz = k == 0 ? 0.55f : (k == 1 ? 8.5f : 1.37f); tapePh[c][k] += (float) (hz / sr); tapePh[c][k] -= std::floor (tapePh[c][k]); }
                const float lfoMs = modWow * (0.7f * std::sin ((float) (twoPi * tapePh[c][0])) + 0.3f * std::sin ((float) (twoPi * tapePh[c][2])))
                                   + modFlut * (0.8f * std::sin ((float) (twoPi * tapePh[c][1])) + 0.5f * flutNoise);
                const float dMs = 14.f + lfoMs;
                tapeLine[c].write (s);
                float t = tapeLine[c].read (jmax (1.f, dMs * 0.001f * (float) sr));
                t = tapeBump[c].process (tapeLp[c].process (t));
                *in[c] = x + tpMix * (t - x);
            }
            if ((n & 63) == 0) { flutNoise += (rng.nextFloat() * 2.f - 1.f - flutNoise) * 0.3f; }
        }

        // -------------------------------- DELAY (stereo ping-pong)
        {
            const float o0 = dly[0].read (dlTimeCur), o1 = dly[1].read (dlTimeCur);
            const float mono = 0.5f * (xl + xr) * dlOn;
            const float ping = 0.65f;
            float f0 = fb * (ping * o1 + (1.f - ping) * o0), f1 = fb * (ping * o0 + (1.f - ping) * o1);
            f0 = fbHp[0].process (fbLp[0].process (f0)); f1 = fbHp[1].process (fbLp[1].process (f1));
            f0 = std::tanh (f0 * 0.9f) / 0.9f; f1 = std::tanh (f1 * 0.9f) / 0.9f;
            dly[0].write (mono + f0); dly[1].write (f1);
            xl += dlMix * o0; xr += dlMix * o1;
        }

        // -------------------------------- REVERB
        {
            float inL = rvHp[0].process (xl * rvOn) + shimL * 0.6f, inR = rvHp[1].process (xr * rvOn) + shimR * 0.6f;
            float oL, oR; fdn.tick (inL, inR, oL, oR);
            const float sL = ps[0].process (oL), sR = ps[1].process (oR);
            shimL = sL * shimAmt * 0.8f; shimR = sR * shimAmt * 0.8f;
            xl += rvMix * (oL + sL * shimAmt * 0.5f) * 1.6f; xr += rvMix * (oR + sR * shimAmt * 0.5f) * 1.6f;
        }

        // -------------------------------- LO-FI
        if (lfOn > 0.001f)
        {
            float y[2] = { xl, xr };
            for (int c = 0; c < 2; ++c)
            {
                // warp (slow pitch wobble)
                warpPh[c] += (float) ((0.45 + 0.1 * c) / sr); warpPh[c] -= std::floor (warpPh[c]);
                const float wd = lf[4] * 9.f;
                const float wm = wd * (std::sin ((float) (twoPi * warpPh[c])) + 0.5f * std::sin ((float) (twoPi * warpPh[c] * 2.31f)));
                warpLine[c].write (y[c]);
                y[c] = warpLine[c].read (jmax (1.f, (wd * 1.6f + 0.02f + wm) * 0.001f * (float) sr));
                // radio band-limit
                if (lf[3] > 0.002f) y[c] = (radLp[c].process (radHp[c].process (y[c]))) * (1.f + 0.6f * lf[3]);
                else { radLp[c].process (radHp[c].process (y[c])); }
            }
            // crush
            if (lf[0] > 0.002f)
            {
                const float q = std::pow (2.f, 15.f - 12.f * lf[0]);
                y[0] = std::floor (y[0] * q + 0.5f) / q; y[1] = std::floor (y[1] * q + 0.5f) / q;
            }
            // sample-rate reduction
            if (lf[1] > 0.002f)
            {
                const float factor = 1.f + lf[1] * lf[1] * 40.f;
                holdPh += 1.f;
                if (holdPh >= factor) { holdPh -= factor; holdL = y[0]; holdR = y[1]; }
                y[0] = holdL; y[1] = holdR;
            }
            // dust: crackle + hiss
            if (lf[2] > 0.002f)
            {
                static thread_local float env = 0.f;
                if (rng.nextFloat() < 0.00004f + lf[2] * 0.0011f) env = (rng.nextFloat() * 2.f - 1.f) * (0.15f + 0.5f * lf[2]) * rng.nextFloat();
                env *= 0.55f;
                const float hiss = (rng.nextFloat() * 2.f - 1.f) * 0.0025f * lf[2];
                const float d = dustHp.process (env) + hiss;
                y[0] += d; y[1] += d * 0.8f + (rng.nextFloat() * 2.f - 1.f) * 0.0006f * lf[2];
            }
            const float act = lfOn * jlimit (0.f, 1.f, (lf[0] + lf[1] + lf[2] + lf[3] + lf[4]) * 40.f);
            xl += act * (y[0] - xl); xr += act * (y[1] - xr);
        }

        // -------------------------------- vibe voicing on the wet signal, master, bypass
        float wl = xl - dryL, wr = xr - dryR;
        wl = vibeB[0].process (vibeA[0].process (wl)); wr = vibeB[1].process (vibeA[1].process (wr));
        if (vibeDrive > 0.f) { wl = std::tanh (wl * vibeDrive) / vibeDrive; wr = std::tanh (wr * vibeDrive) / vibeDrive; }
        float ol = dryL + master * wl, orr = dryR + master * wr;
        ol += byp * (dryL - ol); orr += byp * (dryR - orr);
        if (! std::isfinite (ol) || ! std::isfinite (orr)) { ol = orr = 0.f; }
        L[n] = ol; R[n] = orr;
        pkOut = jmax (pkOut, std::abs (ol), std::abs (orr));
    }
    lastTone = toneLast;
    auto fall = [] (std::atomic<float>& a, float v) { const float c = a.load(); a.store (v > c ? v : c * 0.85f + v * 0.15f); };
    fall (inLevel, pk); fall (outLevel, pkOut);
}

// ============================================================ state / editor
void RippleProcessor::getStateInformation (MemoryBlock& dest)
{
    auto state = apvts.copyState();
    if (auto xml = state.createXml()) { xml->setAttribute ("rpPreset", presetIndex.load()); copyXmlToBinary (*xml, dest); }
}
void RippleProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
        {
            presetIndex = xml->getIntAttribute ("rpPreset", 0);
            apvts.replaceState (ValueTree::fromXml (*xml));
        }
}
AudioProcessorEditor* RippleProcessor::createEditor() { return new RippleEditor (*this); }
AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new RippleProcessor(); }
