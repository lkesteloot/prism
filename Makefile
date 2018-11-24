
# This Makefile is really just a bunch of short shell scripts.

# Location of distray binary.
DISTRAY=../distray/build/src/distray

# Proxy hostname. Specify this on the command line.
PROXY=unspecified

.PHONY: help
help:
	@echo "See Makefile for list of targets."

.PHONY: rebuild-all
rebuild-all:
	rm -rf build
	mkdir build
	(cd build && cmake -D BUILD_SIM=YES .. && make -j)

.PHONY: rebuild-no-sim
rebuild-no-sim:
	rm -rf build
	mkdir build
	(cd build && cmake -D BUILD_SIM=NO .. && make -j)

.PHONY: run-proxy
run-proxy:
	$(DISTRAY) proxy

.PHONY: run-worker
run-worker:
	$(DISTRAY) worker $(PROXY)

.PHONY: run-sim
run-sim:
	build/src/sim/sim build/out.scene

.PHONY: run-controller
run-controller:
	mkdir -p anim
	$(DISTRAY) controller --proxy $(PROXY) --in build/out.scene out.scene --out out-%03d.png anim/out-%03d.png 0,199 build/src/ray/ray out.scene %d out 1000

.PHONY: gif
gif:
	convert -loop 0 -delay 3 anim/out-*.png out.gif

.PHONY: mp4
mp4:
	ffmpeg -pattern_type glob -i 'anim/*.png' -pix_fmt yuv420p -y out.mp4

.PHONY: docker-build
docker-build: $(BIN)
	docker build -t ray .

.PHONY: docker-run
docker-run:
	docker run -it --name ray-run ray

.PHONY: docker-get
docker-get:
	docker cp ray-run:out.png out.png

