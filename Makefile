build:
	pio run
update: build
	curl -F "image=@.pioenvs/nodemcuv2/firmware.bin" 192.168.0.178/update

