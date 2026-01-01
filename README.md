# Ampersand ASL Server 

Environment Variables
---------------------
export AMP_NODE0_NUMBER=nnnnn
export AMP_NODE0_PASSWORD=xxxxx
export AMP_NODE0_MGR_PORT=5038
export AMP_IAX_PROTO=IPV4
export AMP_IAX_PORT=4569
export AMP_ASL_REG_URL=https://register.allstarlink.org
export AMP_ASL_STAT_URL=http://stats.allstarlink.org/uhandler
export AMP_ASL_DNS_BASE=nodes.allstarlink.org

Building ASL Parrot With Install
--------------------------------

    cd asl-parrot
    cmake -DCMAKE_INSTALL_PREFIX=/tmp -B build
    cmake --build build --target asl-parrot
    cmake --install build --component asl-parrot

Debian Package Notes
---------------------

Making the package for the asl-parrot:

    # Update the change log (new entries at top)
    sudo apt install debmake debhelper
    export ASL_PARROT_VERSION=1.2
    cd asl-parrot
    scripts/make-source-tar-parrot.sh
    cd /tmp
    tar -xzmf asl-parrot-$ASL_PARROT_VERSION.tar.gz
    cd asl-parrot-$ASL_PARROT_VERSION
    # Clean out the CMakeLists.txt file of all other targets
    debmake
    debuild

Looking at the contents:

    dpkg -c asl-parrot_1.0-1_arm64.deb 

Installing from a .deb file:

    sudo apt install ./asl-parrot_1.0-1_arm64.deb

Uninstall:

    sudo apt remove ./asl-parrot_1.0-1_arm64.deb

Service Commands:

    sudo systemctl enable asl-parrot
    sudo systemctl start asl-parrot
    journalctl -u asl-parrot -f

Audio Prompts (AWS Polly)
--------------------------

        <speak><prosody rate="100%">Parrot connected, ready to record.</prosody></speak>
