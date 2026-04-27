This repo builds the ASL Parrot which is based on 
the [Ampersand Linking Project](https://github.com/Ampersand-ASL)
by [Bruce MacKinnon KC1FSZ](https://www.qrz.com/db/KC1FSZ).

This parrot was inspired
by the famous [Texas 55553 Parrot](https://mackinnon.info/ampersand/parrot-55553-notes) created by Patrick N2DYI.

> [!Important]
>
> If you are looking for install instructions [start here](docs/parrot-deploy.md).

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
* Supports a network testing API (ASL internal use only) that can 
provide the results of the network test for an arbitrary node.
* DTMF 1 generates an audio sweep pattern that is used for 
testing/characterizing audio hardware. The sweep is preceded 
by an FSK signal for synchronizing test devices. 
* DTMF 2 generates a fixed 440 Hz tone, 0.5 amplitude, 5 seconds. 
* DTMF 3 generate 5 seconds of white noise.
* Supports a pre-recorded program mode.

# Sweep Specifics

* Amplitude 0.5 peak.
* Intro marker: FSK between 400 and 800 Hz, 40ms each, x8.
* Sweep from DC to 4kHz or 8kHz (depending on CODEC) in 100 Hz
increments. 100ms at each frequency.

# Network Test API

Ping:

    curl asl-parrot:8080/ping

Example request:
    
    curl asl-parrot:8080/network-test?node=2002

Example response (good case):

    {"ipv4":{"addr":"18.226.187.225","pingms":49,"port":4569,"rc":0,"status":"ok"}}

Example response (bad case):

    {"ipv4":{"rc":-9,"status":"unreachable"}}

# Building ASL Parrot With Install

    # Make sure you have all the packages needed to build
    sudo apt install cmake build-essential git xxd libasound2-dev libcurl4-gnutls-dev Libusb-1.0-0-dev emacs-nox
    git clone https://github.com/Ampersand-ASL/asl-parrot.git
    cd asl-parrot
    git submodule update --init
    # Move the version number forward in src/main-parrot.cpp
    cmake -DCMAKE_INSTALL_PREFIX=/tmp -B build
    cmake --build build --target asl-parrot
    cmake --install build --component asl-parrot

# Debian Package Notes

Making the package for the asl-parrot:

    # Requirements for build
    sudo apt install debmake debhelper cmake build-essential git xxd libasound2-dev libcurl4-gnutls-dev Libusb-1.0-0-dev
    export ASL_PARROT_VERSION=1.11
    # Pull the source
    git clone https://github.com/Ampersand-ASL/asl-parrot.git
    cd asl-parrot
    git submodule update --init
    # IMPORTANT!! 
    # Update debian/changelog for new version.
    # IMPORTANT!! 
    # Move the source down to /tmp for production of the tarball. This
    # script also makes some tweaks to the source tree.
    scripts/make-source-tar-parrot.sh
    cd /tmp
    tar -xzmf asl-parrot-$ASL_PARROT_VERSION.tar.gz
    cd asl-parrot-$ASL_PARROT_VERSION
    debmake
    debuild
    # Move the package to the distribution area
    scp bruce@pi5:/tmp/asl-parrot_${ASL_PARROT_VERSION}-1_arm64.deb (distribution area)

Looking at the contents:

    dpkg -c asl-parrot_${ASL_PARROT_VERSION}-1_arm64.deb 

Installing from a .deb file:

    wget https://mackinnon.info/ampersand/releases/asl-parrot_${AMP_PARROT_VERSION}-1_arm64.deb
    sudo apt install ./asl-parrot_${AMP_PARROT_VERSION}-1_arm64.deb

Or:

    wget https://mackinnon.info/ampersand/releases/asl-parrot_${AMP_PARROT_VERSION}-1_amd64.deb
    sudo apt install ./asl-parrot_${AMP_PARROT_VERSION}-1_amd64.deb

_(There may be a "Notice" displayed during the install related to a permission issue. This
can safely be ignored.)_

Uninstall:

    sudo apt remove ./asl-parrot_1.11-1_arm64.deb

Service Commands:

    sudo systemctl enable asl-parrot
    sudo systemctl start asl-parrot
    journalctl -u asl-parrot -f

# Environment Variables Used At Runtime

    export AMP_NODE0_NUMBER=61057
    export AMP_NODE0_PASSWORD=
    export AMP_IAX_PROTO=IPV4
    export AMP_IAX_PORT=4569
    export AMP_HTTP_PORT=8080
    export AMP_ASL_REG_URL=https://register.allstarlink.org
    export AMP_ASL_STAT_URL=http://stats.allstarlink.org/uhandler
    export AMP_ASL_DNS_BASE=nodes.allstarlink.org
    # Pointer to Piper TTS files (voice and the espeak runtime files)
    export AMP_PIPER_DIR=/usr/etc
    # Network interface (IPv4) that is used to initiate network tests.
    # Needs to be different from the interface accepting IAX connections
    # for the test to be fully effective
    export AMP_NET_TEST_BIND_ADDR4=0.0.0.0
    # These parameter is used to set the parrot thresholds.
    AMP_PARROT_LEVEL_MEASURE=DBFS_PEAK
    # These are the lower bounds of "very high", "high", "good", and "low".
    # Anything below the last number is "very low".
    AMP_PARROT_LEVEL_THRESHOLDS=-2,-5,-9,-12
    # Uncomment this variable to turn on "program mode." This points to the root of the 
    # program directory.
    #export AMP_PROGRAM_ROOT=/home/bruce/program
    # Uncomment this to join the parrot into the conference so that everyone
    # can hear the recording/playback
    #export AMP_PARROT_MODE=CONFERENCE

# A Useful AWS Command

This command could be used to determine the private IP address of the 
diagnostic interface:

    aws ec2 describe-network-interfaces