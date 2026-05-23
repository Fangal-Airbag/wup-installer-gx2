FROM ghcr.io/wiiu-env/devkitppc:20260504

WORKDIR /app
CMD make -j$(nproc)