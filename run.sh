#;!/bin/bash

while getopts "cdlifhspeqwo" arg
do
	case $arg in
		c)
			echo "configuration with debug、ftl and fio"
			make clean
			./configure --enable-debug 
		    #./configure --enable-debug --with-ftl
			make -j32
			;;
		s)
			echo "scripts/setup.sh"
		    HUGEMEM=5120 scripts/setup.sh
			;;
		e)
			echo "./hello_bdev"
			./examples/bdev/hello_world/hello_bdev -c /root/bdev.conf -b nvme0
			;;
		d)
			echo "delete_ftl_bdev -b nvme0"
		    scripts/rpc.py delete_ftl_bdev -b nvme0
			;;
		l)
			echo "listen spdk"
			 ./app/spdk_tgt/spdk_tgt
			;;
		i)
			echo "identify"
			examples/nvme/identify/identify
			;;
		o)
			echo "use fio_plugin test"
			fio /root/config.fio '--filename=nvme0'
			;;
		f)
			echo "construct_ftl_bdev -b nvme0 -l 0-31 -a 02:00.0"
			scripts/rpc.py construct_ftl_bdev -b nvme0 -l 0-3 -a 02:00.0
			;;
		p)
			echo "perf test"
			./test/bdev/bdevperf/bdevperf -q 1 -s 1024 -w randwrite -t 90 -o 4096 -m 0x1 -c /root/bdev.conf
			;;
		w)
			echo "PCIe perf test"
			./examples/nvme/perf/perf -q 1 -s 1024 -w randwrite -t 9000 -o 4096 -c 0x1 -r 'trtype:PCIe traddr:0000:01:00.0'
			;;
		q)
			echo "perf test"
			./test/bdev/bdevperf/bdevperf -S 5 -q 128 -s 1024 -w write -t 15 -o 65536 -m 0xF -c /root/bdev_restore.conf
			;;
		h)
			echo "argument list"
			echo "c: configure with debug、ftl and fio"
			echo "e: run hello_bdev"
			echo "d: delete_ftl_bdev"
			echo "l: listen spdk"
			echo "i: nvme identify"
			echo "o: use fio_plugin test"
			echo "f: construct_ftl_bdev"
			echo "s: scripts/setup.sh"
			echo "p: use configuration file and perf to test FTL device "
			echo "w: use PCIe traddr to test device "
			echo "q: use restore configuration file and perf to test FTL device "
			;;
		?)	
			echo "unkown argument"
			exit 1
			;;
	esac
done
