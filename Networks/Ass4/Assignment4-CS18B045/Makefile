all : senderSR senderGBN receiverSR receiverGBN

senderSR : senderSR.o
	g++ -o senderSR senderSR.o -pthread
senderSR.o : senderSR.cpp
	g++ -c senderSR.cpp -pthread
receiverSR : receiverSR.o
	g++ -o receiverSR receiverSR.o
receiverSR.o : receiverSR.cpp
	g++ -c receiverSR.cpp
	
senderGBN : senderGBN.o
	g++ -o senderGBN senderGBN.o -pthread
senderGBN.o : senderGBN.cpp
	g++ -c senderGBN.cpp -pthread
receiverGBN : receiverGBN.o
	g++ -o receiverGBN receiverGBN.o
receiverGBN.o : receiverGBN.cpp
	g++ -c receiverGBN.cpp
clean: 
	rm senderSR receiverSR senderGBN receiverGBN *.o 
	



