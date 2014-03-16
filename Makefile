
all: diff build

build:
	@./setup_check.sh
	cd param; make
	cd data_rw; make
	cd data_sk; make
	cd data_ioctl; make
	cd fifo_rw; make
	cd fifo_sysfs; make
	cd fifo_xxx; make

diff:
	cd param; make diff
	cd data_rw; make diff
	cd data_sk; make diff
	cd data_ioctl; make diff
	cd fifo_rw; make diff
	cd fifo_sysfs; make diff
	cd fifo_xxx; make diff

clean:
	cd param; make clean
	cd data_rw; make clean
	cd data_sk; make clean
	cd data_ioctl; make clean
	cd fifo_rw; make clean
	cd fifo_sysfs; make clean
	cd fifo_xxx; make clean

