#
# The recommended compile flags.
#
CFLAGS=-g -Wall -Wextra -std=gnu99
#CFLAGS=-g -Wall -std=gnu99
VFLAGS = --leak-check=full --show-leak-kinds=all --track-origins=yes --malloc-fill=0x04 --free-fill=0x23

#
# The following 4 programs are created
#
all: proxy xmlSender binSender anyReceiver

#
# Linkning the 4 programs
#
proxy: proxy.o connection.o record.o recordToFormat.o recordFromFormat.o
	gcc $(CFLAGS) -o proxy proxy.o connection.o record.o recordToFormat.o recordFromFormat.o

xmlSender: xmlSender.o connection.o xmlfile.o
	gcc $(CFLAGS) -o xmlSender xmlSender.o connection.o xmlfile.o

binSender: binSender.o connection.o binfile.o
	gcc $(CFLAGS) -o binSender binSender.o connection.o binfile.o

anyReceiver: anyReceiver.o connection.o
	gcc $(CFLAGS) -o anyReceiver anyReceiver.o connection.o

#
# Compiling from C files to object files
#
xmlSender.o: xmlSender.c connection.h xmlfile.h
	gcc $(CFLAGS) -c $<

binSender.o: binSender.c connection.h binfile.h
	gcc $(CFLAGS) -c $<

anyReceiver.o: anyReceiver.c connection.h
	gcc $(CFLAGS) -c $<

connection.o: connection.c connection.h
	gcc $(CFLAGS) -c $<

xmlfile.o: xmlfile.c xmlfile.h
	gcc $(CFLAGS) -c $<

binfile.o: binfile.c binfile.h record.h
	gcc $(CFLAGS) -c $<

record.o: record.c record.h
	gcc $(CFLAGS) -c $<

recordToFormat.o: recordToFormat.c recordToFormat.h record.h 
	gcc $(CFLAGS) -c $<

recordFromFormat.o: recordFromFormat.c recordFromFormat.h record.h
	gcc $(CFLAGS) -c $<

proxy.o: proxy.c connection.h record.h
	gcc $(CFLAGS) -c $<

#
# Deleting the only object files
#
clean:
	rm -f *.o

#
# Deleting the object files and binary files
#
realclean: clean
	rm -f xmlSender
	rm -f binSender
	rm -f anyReceiver
	rm -f proxy

#test connection
connection_test1:
	cd test-1-xml-to-xml && \
	./RunTest.sh "127.0.0.1" 2023


run_test3:
	cd test-3-2xml-to-xml && \
	./RunTest.sh 
#run
#test1
run_test1_1:
	./proxy 7654 

run_test1_2:
	./anyReceiver X X 127.0.0.1 7654 30	& sleep 5

run_test1_3:
	./xmlSender A 127.0.0.1 7654	

#test2
run_test2_1:
	./proxy 7655

run_test2_2:
	./anyReceiver X X 127.0.0.1 7655 20 & sleep 1

run_test2_3:
	./anyReceiver Y X 127.0.0.1 7655 20 & sleep 5

run_test2_4:
	./xmlSender A 127.0.0.1 7655

#test3
run_test3_1:
	./proxy 7655

run_test3_2:
	./anyReceiver X X 127.0.0.1 7655 20 & sleep 5

run_test3_3:
	./xmlSender A 127.0.0.1 7655 & ./xmlSender B 127.0.0.1 7655
		

#test4
run_test4_1:
	./proxy 7654 

run_test4_2:
	./anyReceiver X B 127.0.0.1 7654 30 & sleep 5

run_test4_3:
	./binSender A 127.0.0.1 7654

#test5
run_test5_1:
	./proxy 7655 

run_test5_2:
	./anyReceiver X B 127.0.0.1 7655 20 & sleep 1

run_test5_3:
	./anyReceiver Y B 127.0.0.1 7655 20 & sleep 5

run_test5_4:
	./binSender A 127.0.0.1 7655

#test6 ###### opprinnelig port 7655
run_test6_1:
	./proxy 7656

run_test6_2:
	./anyReceiver X B 127.0.0.1 7656 20 & sleep 5

run_test6_3:
	./binSender A 127.0.0.1 7656 & ./binSender B 127.0.0.1 7656

	

#test7
run_test7_1:
	./proxy 7653

run_test7_2:
	./anyReceiver X X 127.0.0.1 7653 30 & sleep 1

run_test7_3:
	./anyReceiver Y B 127.0.0.1 7653 30 & sleep 1

run_test7_4:
	./anyReceiver Z B 127.0.0.1 7653 30 & sleep 5

run_test7_5:
	./xmlSender A 127.0.0.1 7653

#test8
run_test8_1:
	./proxy 7653

run_test8_2:
	./anyReceiver W X 127.0.0.1 7653 30 & sleep 1

run_test8_3:
	./anyReceiver X X 127.0.0.1 7653 30 & sleep 1

run_test8_4:
	./anyReceiver Y B 127.0.0.1 7653 30 & sleep 1

run_test8_5:
	./anyReceiver Z B 127.0.0.1 7653 30 & sleep 5

run_test8_6:
	./binSender B 127.0.0.1 7653 & ./xmlSender A 127.0.0.1 7653

	

#test9 #### opprinnelig proxy 7655
run_test9_1:
	./proxy 7656

run_test9_2:
	./anyReceiver X X 127.0.0.1 7656 20 & sleep 5

run_test9_3:
	./binSender B 127.0.0.1 7656 & ./xmlSender A 127.0.0.1 7656 


#test10 ####opprinnelig port 7655
run_test10_1:
	./proxy 7656

run_test10_2:
	./anyReceiver X X 127.0.0.1 7656 10 & sleep 5

run_test10_3:
	nc 127.0.0.1 7656 < nc-send.txt	


#test11
run_test11_1:
	./proxy 7655

run_test11_2:
	./anyReceiver X B 127.0.0.1 7655 10 & sleep 5

run_test11_3:
	nc 127.0.0.1 7655 < nc-send.bin

#valgrind med check
#test1
valgrind_test1_1:
	valgrind $(VFLAGS) ./proxy 7654 

valgrind_test1_2:
	valgrind $(VFLAGS) ./anyReceiver X X 127.0.0.1 7654 30 & sleep 5

valgrind_test1_3:
	valgrind $(VFLAGS) ./xmlSender A 127.0.0.1 7654	

#test2
valgrind_test2_1:
	valgrind $(VFLAGS) ./proxy 7655

valgrind_test2_2:
	valgrind $(VFLAGS) ./anyReceiver X X 127.0.0.1 7655 20 & sleep 1

valgrind_test2_3:
	valgrind $(VFLAGS) ./anyReceiver Y X 127.0.0.1 7655 20 & sleep 5

valgrind_test2_4:
	valgrind $(VFLAGS) ./xmlSender A 127.0.0.1 7655

#test3
valgrind_test3_1:
	valgrind $(VFLAGS) ./proxy 7655

valgrind_test3_2:
	valgrind $(VFLAGS) ./anyReceiver X X 127.0.0.1 7655 20 & sleep 5

valgrind_test3_3:
	valgrind $(VFLAGS) ./xmlSender A 127.0.0.1 7655 & ./xmlSender B 127.0.0.1 7655

	

#test4
valgrind_test4_1:
	valgrind $(VFLAGS) ./proxy 7654

valgrind_test4_2:
	valgrind $(VFLAGS) ./anyReceiver X B 127.0.0.1 7654 30 & sleep 5

valgrind_test4_3:
	valgrind $(VFLAGS) ./binSender A 127.0.0.1 7654



#test5
valgrind_test5_1:
	valgrind $(VFLAGS) ./proxy 7655

valgrind_test5_2:
	valgrind $(VFLAGS) ./anyReceiver X B 127.0.0.1 7655 20 & sleep 1

valgrind_test5_3:
	valgrind $(VFLAGS) ./anyReceiver Y B 127.0.0.1 7655 20 & sleep 5

valgrind_test5_4:
	valgrind $(VFLAGS) ./binSender A 127.0.0.1 7655


#test6
valgrind_test6_1:
	valgrind $(VFLAGS) ./proxy 7655

valgrind_test6_2:
	valgrind $(VFLAGS) ./anyReceiver X B 127.0.0.1 7655 20 & sleep 5

valgrind_test6_3:
	valgrind $(VFLAGS) ./binSender A 127.0.0.1 7655 & ./binSender B 127.0.0.1 7655



#test7
valgrind_test7_1:
	valgrind $(VFLAGS) ./proxy 7653

valgrind_test7_2:
	valgrind $(VFLAGS) ./anyReceiver X X 127.0.0.1 7653 30 & sleep 1

valgrind_test7_3:
	valgrind $(VFLAGS) ./anyReceiver Y B 127.0.0.1 7653 30 & sleep 1

valgrind_test7_4:
	valgrind $(VFLAGS) ./anyReceiver Z B 127.0.0.1 7653 30 & sleep 5

valgrind_test7_5:
	valgrind $(VFLAGS) ./xmlSender A 127.0.0.1 7653


#test8
valgrind_test8_1:
	valgrind $(VFLAGS) ./proxy 7653

valgrind_test8_2:
	valgrind $(VFLAGS) ./anyReceiver W X 127.0.0.1 7653 30 & sleep 1

valgrind_test8_3:
	valgrind $(VFLAGS) ./anyReceiver X X 127.0.0.1 7653 30 & sleep 1

valgrind_test8_4:
	valgrind $(VFLAGS) ./anyReceiver Y B 127.0.0.1 7653 30 & sleep 1

valgrind_test8_5:
	valgrind $(VFLAGS) ./anyReceiver Z B 127.0.0.1 7653 30 & sleep 5

valgrind_test8_6:
	valgrind $(VFLAGS) ./binSender B 127.0.0.1 7653	& ./xmlSender A 127.0.0.1 7653

 


#test9
valgrind_test9_1:
	valgrind $(VFLAGS) ./proxy 7655

valgrind_test9_2:
	valgrind $(VFLAGS) ./anyReceiver X X 127.0.0.1 7655 20 & sleep 5

valgrind_test9_3:
	valgrind $(VFLAGS) ./xmlSender A 127.0.0.1 7655	& ./binSender B 127.0.0.1 7655	



#test10
valgrind_test10_1:
	valgrind $(VFLAGS) ./proxy 7655

valgrind_test10_2:
	valgrind $(VFLAGS) ./anyReceiver X X 127.0.0.1 7655 10 & sleep 5

valgrind_test10_3:
	valgrind $(VFLAGS) nc 127.0.0.1 7655 < nc-send.txt	


#test11
valgrind_test11_1:
	valgrind $(VFLAGS) ./proxy 7655

valgrind_test11_2:
	valgrind $(VFLAGS) ./anyReceiver X B 127.0.0.1 7655 10 & sleep 5

valgrind_test11_3:
	valgrind $(VFLAGS) nc 127.0.0.1 7655 < nc-send.bin
















