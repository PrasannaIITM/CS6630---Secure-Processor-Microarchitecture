et_attack: common.o
	gcc -fno-pie -no-pie -I. -Ilib/ -O3 et_attack.c -o et_attack lib/aes_1024.o common.o -lm 
#	gcc -I. -O3 attack.c -o attack lib/aes_2048.o common.o -lm
#	gcc -I. -O3 attack.c -o attack lib/aes8_1024.o common.o -lm



common.o: common.c
	gcc -O3 common.c  -c -o common.o



clean:
	rm -f attack et_attack correlate *.o
