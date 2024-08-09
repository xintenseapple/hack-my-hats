FROM tophat:base

USER root
ADD src/ /root/heap_miner
RUN cd /root/heap_miner && make
RUN cp /root/heap_miner/heap_miner /home/hatman/heap_miner
RUN chmod 711 /home/hatman/heap_miner
RUN ln -s /home/hatman/heap_miner /usr/bin/heap_miner

RUN cp /root/heap_miner/server.py /home/hatman/server.py
RUN chown root:hatman /home/hatman/heap_miner
RUN chmod 755 /home/hatman/heap_miner

USER hatman
ENTRYPOINT python3 /home/hatman/server.py