# demo-app
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential git ca-certificates curl wget python3 python3-pip \
    make pkg-config xz-utils \
    gcc-arm-none-eabi binutils-arm-none-eabi \
    libnewlib-arm-none-eabi libnewlib-dev \
 && rm -rf /var/lib/apt/lists/*


# ---- Oh My Posh ----
RUN curl -L \
      https://github.com/JanDeDobbeleer/oh-my-posh/releases/latest/download/posh-linux-amd64 \
      -o /usr/local/bin/oh-my-posh \
 && chmod +x /usr/local/bin/oh-my-posh \
 && printf '\n# oh-my-posh\n%s\n' \
      'eval "$(oh-my-posh init bash --config robbyrussell)"' \
      >> /etc/bash.bashrc

# Workdir inside container (we’ll mount our host repo here)
WORKDIR /workspace

# Nice default
CMD ["/bin/bash"]
