FROM rockylinux/rockylinux:9.7

LABEL org.opencontainers.image.description="Orion Docker Cluster Simulator" \
      org.label-schema.docker.cmd="docker-compose up -d" \
      maintainer="Denis Bertini"

RUN set -ex \
    && dnf makecache \
    && dnf -y update \
    && dnf -y install dnf-plugins-core \
    && dnf config-manager --set-enabled crb \
    && dnf -y install epel-release \
    && dnf -y install \
       emacs \
       wget \
       bzip2 \
       perl \
       gcc \
       gcc-c++ \
       git \
       gnupg \
       make \
       dbus-devel \
       munge \
       munge-devel \
       python3-devel \
       python3-pip \
       python3 \
       mariadb-server \
       mariadb-devel \
       psmisc \
       bash-completion \
       vim-enhanced \
       http-parser-devel \
       json-c-devel \
       libnl3 libnl3-devel \
       libevent libevent-devel \
       hwloc hwloc-devel \
       pmix pmix-devel \
       openmpi \
       openmpi-devel \
       apptainer \
    && dnf clean all \
    && rm -rf /var/cache/dnf


ENV PATH="/usr/lib64/openmpi/bin:${PATH}"
ENV LD_LIBRARY_PATH="/usr/lib64/openmpi/lib:${LD_LIBRARY_PATH}"

RUN alternatives --install /usr/bin/python python /usr/bin/python3 3 && \
    alternatives --set python /usr/bin/python3

RUN pip3 install Cython pytest

ARG GOSU_VERSION=1.17

RUN set -ex \
    && wget -O /usr/local/bin/gosu "https://github.com/tianon/gosu/releases/download/$GOSU_VERSION/gosu-amd64" \
    && wget -O /usr/local/bin/gosu.asc "https://github.com/tianon/gosu/releases/download/$GOSU_VERSION/gosu-amd64.asc" \
    && export GNUPGHOME="$(mktemp -d)" \
    && gpg --batch --keyserver hkps://keys.openpgp.org --recv-keys B42F6819007F00F88E364FD4036A9C25BF357DD4 \
    && gpg --batch --verify /usr/local/bin/gosu.asc /usr/local/bin/gosu \
    && rm -rf "${GNUPGHOME}" /usr/local/bin/gosu.asc \
    && chmod +x /usr/local/bin/gosu \
    && gosu nobody true

ARG SLURM_TAG=slurm-25-11-4-1

RUN set -x \
    && git clone -b ${SLURM_TAG} --single-branch --depth=1 https://github.com/SchedMD/slurm.git \
    && pushd slurm \    
    && ./configure --enable-debug --prefix=/usr --with-pmix=/usr --with-mysql_config=/usr/bin  --libdir=/usr/lib64 --sysconfdir=/etc/slurm \
    && make install \
    && install -D -m644 etc/cgroup.conf.example /etc/slurm/cgroup.conf.example \
    && install -D -m644 etc/slurm.conf.example /etc/slurm/slurm.conf.example \
    && install -D -m644 etc/slurmdbd.conf.example /etc/slurm/slurmdbd.conf.example \
    && install -D -m644 contribs/slurm_completion_help/slurm_completion.sh /etc/profile.d/slurm_completion.sh \
    && popd \
    && rm -rf slurm \
    && groupadd -r --gid=990 slurm \
    && useradd -r -g slurm --uid=990 slurm \
    && mkdir /etc/sysconfig/slurm \
        /var/spool/slurmd \
        /var/run/slurmd \
        /var/run/slurmdbd \
        /var/lib/slurmd \
        /var/log/slurm \
        /data \
    && touch /var/lib/slurmd/node_state \
        /var/lib/slurmd/front_end_state \
        /var/lib/slurmd/job_state \
        /var/lib/slurmd/resv_state \
        /var/lib/slurmd/trigger_state \
        /var/lib/slurmd/assoc_mgr_state \
        /var/lib/slurmd/assoc_usage \
        /var/lib/slurmd/qos_usage \
        /var/lib/slurmd/fed_mgr_state \
    && chown -R slurm:slurm /var/*/slurm* \
    && /sbin/create-munge-key

# Build lustre-mpc C plugin + cache_daemon + preadv_hook
COPY ../mpc /opt/mpc
RUN cd /opt/mpc && mkdir build && cd build && cmake .. && make -j$(nproc) \
    && cp plugin/bb_lustre_mpc.so /usr/lib64/slurm/plugins/ 2>/dev/null || true \
    && cp cache_daemon /usr/local/bin/ \
    && cp preadv_hook.so /usr/local/lib/

COPY slurm.conf /etc/slurm/slurm.conf
COPY slurmdbd.conf /etc/slurm/slurmdbd.conf
COPY cgroup.conf /etc/slurm/cgroup.conf

RUN set -x \
    && chown slurm:slurm /etc/slurm/slurmdbd.conf \
    && chmod 600 /etc/slurm/slurmdbd.conf \
    && chown slurm:slurm /etc/slurm/cgroup.conf \
    && chmod 600 /etc/slurm/cgroup.conf

RUN chmod 755 /etc && \
    chown root:root /etc

COPY docker-entrypoint.sh /usr/local/bin/docker-entrypoint.sh
ENTRYPOINT ["/usr/local/bin/docker-entrypoint.sh"]

# dbus/Dockerfile
CMD ["dbus-daemon", "--nofork", "--address=tcp:host=dbus,bind=0.0.0.0,port=16979"]
# slurmdbd
CMD ["slurmdbd"]
