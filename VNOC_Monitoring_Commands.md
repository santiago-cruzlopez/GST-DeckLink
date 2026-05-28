# VNOC Stream Monitoring — Command Reference

Single-line, copy-paste commands for monitoring video quality, audio levels, and
stream errors on macOS using open-source tools. Built for shared use across
multiple operators and machines — **no admin rights and no installs required**
(beyond the tools already on the box: `ffmpeg`, `ffprobe`, `ffplay`, `mpv`, `tsmonitor`).

All monitoring commands target the feed **`239.1.16.47:1234`**. To monitor a
different feed, replace that address everywhere it appears.

The resilient UDP query string (`overrun_nonfatal=1&fifo_size=1000000&reuse=1`) is
baked into the input URLs so commands tolerate network bursts/overruns and several
operators can join the same group at once.

---

## Quick index

| # | Command | What it detects |
|---|---------|-----------------|
| 1 | Resilient playback (mpv) | Eyes-on confidence monitoring that survives bursts |
| 2 | TR 101 290 P1/P2/P3 (tsmonitor) | Standardized transport-stream error counters |
| 3 | EBU R128 loudness meter | Live BS.1770 loudness / true-peak |
| 4 | Black & frozen frame detection | Video gone to black or stopped moving |
| 5 | Audio silence detection | Dead audio (5s+ below -30 dB) |
| 6 | Stream inventory | Captions, color space, field order, channels, languages |
| 7 | Continuity / corruption monitor | TS continuity errors & corrupt packets (IP-loss proxy) |
| 8 | Interlace / telecine / cadence | Wrong field order, broken 3:2 pulldown |
| 9 | Static-content detection | Near-frozen content finer than freezedetect |
| 10 | Per-channel audio levels | Dead leg, swapped L/R, mono-in-stereo |
| 11 | Backup-path validation | Confirms a backup route has bitrate headroom |

---

## Playback & error monitoring (existing tools)

### 1. Resilient playback
Buffered playback that survives bursts/overruns instead of dying.

```bash
mpv --demuxer-lavf-o=overrun_nonfatal=1,fifo_size=1000000 --cache=yes --network-timeout=5 udp://239.1.16.47:1234
```

### 2. TR 101 290 P1/P2/P3
Standardized transport-error counters (the one layer FFmpeg cannot replace).

```bash
tsmonitor udp://239.1.16.47:1234
```

### 3. EBU R128 loudness meter
Live BS.1770 loudness / true-peak visualizer.

```bash
ffmpeg -hide_banner -i "udp://239.1.16.47:1234?overrun_nonfatal=1&fifo_size=1000000&reuse=1" -filter_complex "ebur128=peak=true:video=1:meter=9" -pix_fmt yuv420p -preset ultrafast -r 30 -c:v libx264 -c:a aac -f mpegts - | ffplay -i - -window_title "EBU R128 Loudness, using BS.1770"
```

### 4. Black & frozen frame detection
Flags video that has gone to black or stopped moving (2s threshold).

```bash
ffmpeg -hide_banner -i "udp://239.1.16.47:1234?overrun_nonfatal=1&fifo_size=1000000&reuse=1" -filter_complex "[0:v]blackdetect=d=2:pix_th=0.00;[0:v]freezedetect=d=2" -f null -
```

### 5. Audio silence detection
Flags dead audio lasting 5s+ below -30 dB.

```bash
ffmpeg -hide_banner -i "udp://239.1.16.47:1234?overrun_nonfatal=1&fifo_size=1000000&reuse=1" -af silencedetect=noise=-30dB:d=5 -f null -
```

---

## Transport & structural checks (new)

### 6. Stream inventory
Captions flag, color space, field order, channel layout, and languages in one shot.
Run on a known-good feed to capture your baseline.

```bash
ffprobe -hide_banner -i "udp://239.1.16.47:1234?overrun_nonfatal=1" -show_programs -show_streams -of flat | grep -iE "codec_name|color_|field_order|channel_layout|sample_rate|closed_captions|TAG:language"
```

### 7. Continuity / corruption monitor
Live console readout of TS continuity errors and corrupt packets. Also the best
IP-loss proxy available without admin rights or packet capture.

```bash
ffmpeg -hide_banner -loglevel verbose -i "udp://239.1.16.47:1234?overrun_nonfatal=1&fifo_size=1000000&reuse=1" -f null - 2>&1 | grep --line-buffered -iE "continuity|corrupt|error"
```

---

## Video forensics (new)

### 8. Interlace / telecine / cadence
Catches wrong field order and broken 3:2 pulldown via `idet` field counts.

```bash
ffmpeg -hide_banner -i "udp://239.1.16.47:1234?overrun_nonfatal=1&fifo_size=1000000&reuse=1" -vf idet -an -f null -
```

### 9. Static-content detection
YDIF (frame-to-frame luma difference) trending near zero indicates near-frozen
content that a 2-second freezedetect threshold misses.

```bash
ffmpeg -hide_banner -i "udp://239.1.16.47:1234?overrun_nonfatal=1&fifo_size=1000000&reuse=1" -vf "signalstats,metadata=print:key=lavfi.signalstats.YDIF" -an -f null -
```

---

## Audio forensics (new)

### 10. Per-channel audio levels
Per-channel RMS exposes a dead leg, swapped L/R, or mono-in-a-stereo-pair that
program-summed loudness (command 3) hides. Bump `.1.` to `.2.`, `.3.` … to read
each channel.

```bash
ffmpeg -hide_banner -i "udp://239.1.16.47:1234?overrun_nonfatal=1&fifo_size=1000000&reuse=1" -af "astats=metadata=1:reset=1,ametadata=print:key=lavfi.astats.1.RMS_level" -vn -f null -
```

---

## Network (new)

### 11. Backup-path validation
Pushes ~15 Mbps of test pattern down a backup route; run command 7 on the far end
to confirm headroom.

> **Run on a test path only — never production.** This command *sends* a stream to
> a destination. The target is intentionally **`RECEIVER_IP`**, not the monitoring
> address — pointing it at `239.1.16.47:1234` would blast a test pattern onto the
> live feed. Replace `RECEIVER_IP` with the receive-end address of the backup path.

```bash
ffmpeg -re -f lavfi -i "testsrc=size=1920x1080:rate=30" -c:v libx264 -b:v 15M -f mpegts "udp://RECEIVER_IP:1234?pkt_size=1316"
```

---

## Operator notes

- **Do not add `-loglevel error` to commands 9 and 10.** They print at *info* level
  and go silent if the log level is suppressed.
- **Buffer sizing:** `fifo_size=1000000` is FFmpeg's internal ring buffer (~188 MB,
  measured in 188-byte packets). The kernel socket-buffer raise (`sudo sysctl …`)
  needs admin and is intentionally omitted, so `buffer_size` would be clamped by the
  OS anyway — rely on `fifo_size` for burst absorption.
- **Stop a command** with `Ctrl-C`. The `-f null -` commands run until stopped.
- **`reuse=1`** lets multiple operators join the same multicast group simultaneously.

## Intentionally omitted

Removed per the no-file / no-admin requirements:

- File-based loudness logger and PCR-PTS CSV dump (exist only to write logs).
- PAT/PMT diff loop (writes scratch files).
- `tshark` / `iperf3` passive capture (require admin rights or installation).

A few capabilities genuinely cannot be reproduced without TSDuck: full TR 101 290
P1/P2/P3 counters (covered by command 2), precise per-PID bitrate accounting,
decoded PCR accuracy/repetition figures, and clean SCTE-35 splice decoding. If
ad-avail verification or conformance-grade PCR numbers are needed, that is the case
for getting TSDuck installed (on macOS, `brew install tsduck` typically works
without admin if Homebrew is already set up).
