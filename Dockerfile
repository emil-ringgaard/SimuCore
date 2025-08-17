# syntax=docker/dockerfile:1.7
FROM debian:bookworm-slim

ARG DEBIAN_FRONTEND=noninteractive
# Pin or override the PlatformIO version at build time if you want:
#   docker build --build-arg PIO_VERSION==6.1.15 -t my-pio .
ARG PIO_VERSION=

# Create a non-root user that can access serial devices
ARG USERNAME=pio
ARG UID=1000
ARG GID=1000

# Where PlatformIO stores its core (packages, toolchains, etc.)
ENV PLATFORMIO_CORE_DIR=/opt/platformio
# Make `pio` the default entrypoint command
ENV PATH=/home/${USERNAME}/.local/bin:${PATH}

# Recommended: turn off telemetry in CI/containers
ENV PLATFORMIO_SETTING_ENABLE_TELEMETRY=no

# Install OS deps then PlatformIO via pip
RUN set -eux; \
    apt-get update; \
    apt-get install -y --no-install-recommends \
      python3 python3-pip python3-venv \
      git ca-certificates curl unzip \
      build-essential \
      udev usbutils \
    ; \
    rm -rf /var/lib/apt/lists/*; \
    # Create user and groups (dialout for serial, plugdev/uucp common on some distros)
    groupadd -g "${GID}" "${USERNAME}"; \
    useradd -m -u "${UID}" -g "${GID}" -s /bin/bash "${USERNAME}"; \
    usermod -aG dialout "${USERNAME}" || true; \
    usermod -aG plugdev "${USERNAME}" || true; \
    usermod -aG uucp "${USERNAME}" || true; \
    # Prepare cache dir
    mkdir -p "${PLATFORMIO_CORE_DIR}"; \
    chown -R "${USERNAME}:${USERNAME}" "${PLATFORMIO_CORE_DIR}"

# Create and use a virtual environment
RUN python3 -m venv /opt/pio-venv && \
    /opt/pio-venv/bin/pip install --upgrade pip && \
    /opt/pio-venv/bin/pip install platformio

ENV PATH="/opt/pio-venv/bin:$PATH"


USER ${USERNAME}
WORKDIR /workspace


# Install platformio (optionally version-pinned)
RUN set -eux; \
    python3 -m pip install --upgrade pip; \
    if [ -n "${PIO_VERSION}" ]; then \
      python3 -m pip install "platformio==${PIO_VERSION}"; \
    else \
      python3 -m pip install platformio; \
    fi; \
    # Warm up basic caches (optional, keeps image small if you skip)
    pio --version

# Default to the PlatformIO CLI
ENTRYPOINT ["pio"]
CMD ["--help"]
