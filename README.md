# Ampersand ASL Server 

Environment Variables
---------------------
export AMP_NODE0_NUMBER=nnnnn
export AMP_NODE0_PASSWORD=xxxxx
export AMP_NODE0_MGR_PORT=5038
export AMP_IAX_PROTO=IPV4
export AMP_IAX_PORT=4569
export AMP_ASL_REG_URL=https://register.allstarlink.org

Building ASL Parrot With Install
--------------------------------

    cd asl-parrot
    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=/tmp
    cmake --build . --target asl-parrot
    cmake --install . --component asl-parrot

Debian Package Notes
---------------------

Making the package for the asl-parrot:

    sudo apt install debmake debhelper
    VERSION=1.0
    cd asl-parrot
    scripts/make-source-tar-parrot.sh
    cd /tmp
    tar -xzmf asl-parrot-$VERSION.tar.gz
    cd asl-parrot-$VERSION
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
