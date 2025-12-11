# Ampersand ASL Server 

Building ASL Parrot With Install
--------------------------------

    cd sw/ml2
    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=/tmp
    cmake --build . --target asl-parrot
    cmake --install . --component asl-parrot

Debian Package Notes
---------------------

Making the package for the asl-parrot:

    sudo apt install debmake debhelper
    VERSION=1.1
    cd microlink2/sw/ml2
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
