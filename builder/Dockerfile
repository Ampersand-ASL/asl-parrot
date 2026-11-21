ARG ARCH=amd64
ARG OS=trixie

FROM docker.io/$ARCH/debian:$OS

ARG ARCH
ARG OS

# Package build tools and the package's Build-Depends (debian/control).
# wget and ca-certificates are needed because the build downloads the
# prebuilt libpiper runtime and the Piper voice/espeak data.
RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get -y install \
	apt-utils build-essential devscripts fakeroot debhelper git lsb-release \
	libdistro-info-perl ca-certificates wget \
	cmake libasound2-dev libcurl4-gnutls-dev adduser \
	--no-install-recommends

COPY ./entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
