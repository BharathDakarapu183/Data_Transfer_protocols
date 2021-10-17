all:
	g++ -std=c++11 -pthread SenderGBN.cpp -o SenderGBN
	g++ -std=c++11 -pthread receiverGBN.cpp -o receiverGBN
	g++ -std=c++11 -pthread SenderSR.cpp -o SenderSR
	g++ -std=c++11 -pthread receiverSR.cpp -o receiverSR
clean:
	rm -rf SenderSR receiverSR SenderGBN receiverGBN 
