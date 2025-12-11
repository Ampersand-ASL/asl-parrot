# One-time install of Ampersand parrot onto clean Debian 13
# EC2 instance.
# Bruce MacKinnon KC1FSZ

# .... more here

# Start in project base
cd microlink2
git submodule update --init --remote
cd sw/ml2
mkdir -p build
cd build
cmake ..
make asl-parrot
cd ../../..

sudo mkdir -p /usr/bin
sudo mkdir -p /usr/media
sudo mkdir -p /usr/etc
sudo cp sw/ml2/build/asl-parrot /usr/bin
sudo cp sw/ml2/media/* /usr/media
sudo cp sw/ml2/etc/asl-parrot.env /usr/etc
sudo cp sw/ml2/etc/asl-parrot.service /lib/systemd/system
# The service runs as asl-parrot
useradd -r -s /sbin/nologin asl-parrot

# Debian Packaging
#    sudo apt-get install build-essential debhelper devscripts dh-make fakeroot lintian