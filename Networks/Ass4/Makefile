all : SenderSR ReceiverSR SenderGBN ReceiverGBN



SenderSR : SenderSR.o
	g++ -o SenderSR SenderSR.o -pthread
SenderSR.o : SenderSR.cpp
	g++ -c SenderSR.cpp -pthread
ReceiverSR : ReceiverSR.o
	g++ -o ReceiverSR ReceiverSR.o
ReceiverSR.o : ReceiverSR.cpp
	g++ -c ReceiverSR.cpp
	

	
SenderGBN : SenderGBN.o
	g++ -o SenderGBN SenderGBN.o -pthread
SenderGBN.o : SenderGBN.cpp
	g++ -c SenderGBN.cpp -pthread
ReceiverGBN : ReceiverGBN.o
	g++ -o ReceiverGBN ReceiverGBN.o
ReceiverGBN.o : ReceiverGBN.cpp
	g++ -c ReceiverGBN.cpp
	



