PalSetup: main.o
	g++ -o PalSetup $< -L/usr/local/lib -lboost_date_time -lpthread  -lboost_filesystem -lboost_system -lbacktest -lboost_filesystem -lboost_system -lpthread

PalSetupTradeStation: main_tradestation.o
	g++ -o PalSetupTradeStation $< -L/usr/local/lib -lboost_date_time -lpthread  -lboost_filesystem -lboost_system -lbacktest -lboost_filesystem -lboost_system -lpthread

install: PalSetup
	install -c PalSetup /usr/local/bin

main.o: /usr/local/include/backtester/*.h
main_tradestation.o: /usr/local/include/backtester/*.h

CCFLAGS := -O2 -c -Wall -std=c++17 -I/usr/local/include/backtester -I/usr/local/include/priceactionlab

%.o: %.cpp
	g++ $(CCFLAGS) $<
