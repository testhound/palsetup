#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include "TimeSeriesCsvReader.h"
#include "TimeSeriesCsvWriter.h"
#include "TimeSeriesIndicators.h"
#include "DecimalConstants.h"

using namespace mkc_timeseries;
using std::shared_ptr;
using namespace boost::gregorian;

using Num = num::DefaultNumber;

shared_ptr <TimeSeriesCsvReader<Num>>
createTimeSeriesReader (int fileType, const std::string& fileName, const Num& tick)
{
  switch (fileType)
    {
    case 1:
      return std::make_shared<CSIErrorCheckingFuturesCsvReader<Num>> (fileName, 
								      TimeFrame::DAILY,
								      TradingVolume::CONTRACTS,
								      tick);

    case 2:
      return std::make_shared<CSIErrorCheckingExtendedFuturesCsvReader<Num>> (fileName, 
									     TimeFrame::DAILY,
									      TradingVolume::CONTRACTS, tick);

    case 3:
      return std::make_shared<TradeStationErrorCheckingFormatCsvReader<Num>> (fileName, 
									    TimeFrame::DAILY,
									      TradingVolume::CONTRACTS,
									      tick);

    case 4:
      return std::make_shared<PinnacleErrorCheckingFormatCsvReader<Num>> (fileName, 
									TimeFrame::DAILY,
									  TradingVolume::CONTRACTS,
									  tick);

    case 5:
      return std::make_shared<PALFormatCsvReader<Num>> (fileName, 
						      TimeFrame::DAILY,
							TradingVolume::CONTRACTS,
							tick);
    }

  throw std::out_of_range (std::string ("Invalid file type"));
}

int main(int argc, char** argv) {
    std::vector<std::string> v(argv, argv + argc);
    if ((argc == 3) || (argc == 4)) {
        std::string historicDataFileName(v[1]);
        int fileType = std::stoi(v[2]);

        Num securityTick(DecimalConstants<Num>::EquityTick);

        if (argc == 4)
            securityTick = Num(std::stof(v[3]));

        shared_ptr<TimeSeriesCsvReader<Num>> reader;
        if ((fileType >= 1) && fileType <= 5)
            reader = createTimeSeriesReader(fileType, historicDataFileName, securityTick);

        reader->readFile();

        std::shared_ptr<OHLCTimeSeries<Num>> aTimeSeries = reader->getTimeSeries();

        // 1. Get Ticker Symbol
        std::string tickerSymbol;
        std::cout << "Enter ticker symbol: ";
        std::cin >> tickerSymbol;

        // 3. Split Data
        size_t insampleSize = static_cast<size_t>(aTimeSeries->getNumEntries() * 0.8);
        OHLCTimeSeries<Num> insampleSeries(aTimeSeries->getTimeFrame(), aTimeSeries->getVolumeUnits());
        OHLCTimeSeries<Num> outOfSampleSeries(aTimeSeries->getTimeFrame(), aTimeSeries->getVolumeUnits());

        int count = 0;
        for (auto it = aTimeSeries->beginSortedAccess(); it != aTimeSeries->endSortedAccess(); ++it)
	  {
	    const auto& entry = *it;
            if (static_cast<size_t>(count) < insampleSize) 
	      insampleSeries.addEntry(entry);
	    else
	      outOfSampleSeries.addEntry(entry);

            count++;
	  }

        // 4. Insample Stop Calculation
        NumericTimeSeries<Num> closingPrices(insampleSeries.CloseTimeSeries());
        NumericTimeSeries<Num> rocOfClosingPrices(RocSeries(closingPrices, 1));

        Num medianOfRoc(Median(rocOfClosingPrices));
        Num robustQn = RobustQn<Num>(rocOfClosingPrices).getRobustQn();
	Num MAD (MedianAbsoluteDeviation<Num>(rocOfClosingPrices.getTimeSeriesAsVector()));
	Num StdDev(StandardDeviation<Num>(rocOfClosingPrices.getTimeSeriesAsVector()));
	
        Num stopValue = medianOfRoc + robustQn;

        // 2. Generate Target/Stop Files
        std::ofstream tsFile1(tickerSymbol + "_0_5_.TRS");
        tsFile1 << (stopValue * Num(0.5)) << std::endl << stopValue << std::endl;
        tsFile1.close();

        std::ofstream tsFile2(tickerSymbol + "_1_0_.TRS");
        tsFile2 << stopValue << std::endl << stopValue << std::endl;
        tsFile2.close();

        // 5. Write Insample Data
        PalTimeSeriesCsvWriter<Num> insampleWriter(tickerSymbol + "_IS.txt", insampleSeries);
        insampleWriter.writeFile();

	// 5. Write OOS Data
        PalTimeSeriesCsvWriter<Num> oosampleWriter(tickerSymbol + "_OOS.txt", outOfSampleSeries);
        oosampleWriter.writeFile();

        std::cout << "Median = " << medianOfRoc << std::endl;
        std::cout << "Qn  = " << robustQn << std::endl;
	std::cout << "MAD  = " << MAD << std::endl;
	std::cout << "Standard Deviation  = " << StdDev << std::endl;
	std::cout << "Stop = " << stopValue << std::endl;

    } else {
        std::cout << "Usage (beta):: PalSetup datafile file-type (1 = CSI, 2 = CSI Extended, 3 = TradeStation, 4 = Pinnacle, 5 = PAL)" << std::endl;
    }
    return 0;
}
