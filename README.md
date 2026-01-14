This repo builds the ASL Parrot which is based on 
the [Ampersand Linking Project](https://github.com/Ampersand-ASL)
by [Bruce MacKinnon KC1FSZ](https://www.qrz.com/db/KC1FSZ).

This parrot was inspired
by the famous [Texas 55553 Parrot](https://mackinnon.info/ampersand/parrot-55553-notes) created by Patrick N2DYI.

# Capabilities

* Basic record/playback.
* Announces the peak and average audio level in dB.
* Does not require authentication/registration to use.
* Will make an announcement if the calling node is not 
registered in the ASL database (inferred from result of DNS lookup
to nnnnn.nodes.allstarlink.org)
* Will make an announcement if the calling node is unreachable
from the public internet (i.e. if your firewall isn't configured
properly).
* DTMF 1 generates an audio sweep pattern that is used for 
testing/characterizing audio hardware. The sweep is preceded 
by an FSK signal for synchronizing test devices. 
* DTMF 2 generates a fixed 440 Hz tone, 0.5 amplitude, 5 seconds. 

# Sweep Specifics

* Amplitude 0.5 peak.
* Intro marker: FSK between 400 and 800 Hz, 40ms each, x8.
* Sweep from DC to 4kHz or 8kHz (depending on CODEC) in 100 Hz
increments. 100ms at each frequency.

# Building ASL Parrot With Install

    cd asl-parrot
    cmake -DCMAKE_INSTALL_PREFIX=/tmp -B build
    cmake --build build --target asl-parrot
    cmake --install build --component asl-parrot

# Debian Package Notes

Making the package for the asl-parrot:

    # Move the version number forward in src/main-parrot.cpp
    # Update the change log (new entries at top)
    sudo apt install debmake debhelper
    export ASL_PARROT_VERSION=1.3
    cd asl-parrot
    scripts/make-source-tar-parrot.sh
    cd /tmp
    tar -xzmf asl-parrot-$ASL_PARROT_VERSION.tar.gz
    cd asl-parrot-$ASL_PARROT_VERSION
    debmake
    debuild
    # Move the package to the distribution area

Looking at the contents:

    dpkg -c asl-parrot_$ASL_PARROT_VERSION-1_arm64.deb 

Installing from a .deb file:

    wget https://mackinnon.info/ampersand/releases/asl-parrot_1.3-1_arm64.deb
    sudo apt install ./asl-parrot_1.3-1_arm64.deb

Uninstall:

    sudo apt remove ./asl-parrot_1.3-1_arm64.deb

Service Commands:

    sudo systemctl enable asl-parrot
    sudo systemctl start asl-parrot
    journalctl -u asl-parrot -f

# Environment Variables Used At Runtime

export AMP_NODE0_NUMBER=nnnnn
export AMP_NODE0_PASSWORD=xxxxx
export AMP_IAX_PROTO=IPV4
export AMP_IAX_PORT=4569
export AMP_ASL_REG_URL=https://register.allstarlink.org
export AMP_ASL_STAT_URL=http://stats.allstarlink.org/uhandler
export AMP_ASL_DNS_BASE=nodes.allstarlink.org
# Pointer to Piper TTS files (voice and the espeak runtime files)
AMP_PIPER_DIR=/usr/etc
