
all: diff build

build:
	@./setup_check.sh
	cd data_chr; make
	cd data_ioctl; make
	cd data_rw; make
	cd data_sk; make
	cd fifo_fix; make
	cd fifo_rw; make
	cd fifo_sysfs; make
	cd fifo_xxx; make
	cd hello; make
	cd null; make
	cd param; make
	cd sysx_file; make
	cd sysx_file2; make
	cd sysx_group; make
	cd sysx_ktype; make
	cd sysx_ktype2; make
	cd zero; make

diff:
	cd param; make diff
	cd data_rw; make diff
	cd data_sk; make diff
	cd data_ioctl; make diff
	cd fifo_rw; make diff
	cd fifo_sysfs; make diff
	cd fifo_xxx; make diff

clean:
	cd data_chr; make clean
	cd data_ioctl; make clean
	cd data_rw; make clean
	cd data_sk; make clean
	cd fifo_fix; make clean
	cd fifo_rw; make clean
	cd fifo_sysfs; make clean
	cd fifo_xxx; make clean
	cd hello; make clean
	cd null; make clean
	cd param; make clean
	cd sysx_file; make clean
	cd sysx_file2; make clean
	cd sysx_group; make clean
	cd sysx_ktype; make clean
	cd sysx_ktype2; make clean
	cd zero; make clean

