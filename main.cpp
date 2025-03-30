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

int main(int argc, char **argv)
{
  std::vector<std::string> v(argv, argv + argc);
  if ((argc == 3) || (argc == 4))
    {
      std::string historicDataFileName (v[1]);
      int fileType = std::stoi(v[2]);

      Num securityTick (DecimalConstants<Num>::EquityTick);

      if (argc == 4)
	securityTick = Num (std::stof(v[3]));

      shared_ptr <TimeSeriesCsvReader<Num>> reader;
      if ((fileType >= 1) && fileType <= 5)
	reader = createTimeSeriesReader (fileType, historicDataFileName, securityTick);

      
      //CSIErrorCheckingExtendedFuturesCsvReader<Num> reader(historicDataFileName, 
      //TimeFrame::DAILY,
      //							 TradingVolume::CONTRACTS);
      reader->readFile();

      std::shared_ptr<OHLCTimeSeries<Num>> aTimeSeries = reader->getTimeSeries();
      //boost::gregorian::date lastDate (aTimeSeries->getLastDate());
      //OHLCTimeSeries<Num>::ConstTimeSeriesIterator entryIterator = aTimeSeries->getTimeSeriesEntry (lastDate);
      //Num lastPrice (aTimeSeries->getCloseValue(entryIterator, 0));
      
      // boost::gregorian::date firstDate (1980, Jan, 1);
      //OHLCTimeSeries<Num> dateFilteredSeries (FilterTimeSeries (*aTimeSeries, 
      //						      DateRange (firstDate,
      //								 aTimeSeries->getLastDate())));
      int rocPeriod = 1;

      
      NumericTimeSeries<Num> closingPrices (aTimeSeries->CloseTimeSeries());
      NumericTimeSeries<Num> rocOfClosingPrices (RocSeries (closingPrices, rocPeriod));

      
      std::vector<Num> aSortedVec (rocOfClosingPrices.getTimeSeriesAsVector());

      Num medianOfRoc (Median (rocOfClosingPrices));
      RobustQn<Num> robustDev (rocOfClosingPrices);

      Num robustQn (robustDev.getRobustQn());

      std::cout << "Median of roc of close = " << medianOfRoc << std::endl;
      std::cout << "Qn of roc of close  = " << robustQn << std::endl;

      PalTimeSeriesCsvWriter<Num> dumpFile("PalTimeSeries.csv", *aTimeSeries);
      dumpFile.writeFile();
      
      //NumericTimeSeries<Num>::ConstTimeSeriesIterator it = rocOfClosingPrices.beginSortedAccess();
      //for (; it != rocOfClosingPrices.endSortedAccess(); it++)
      //	std::cout << it->first << "," << it->second->getValue() << std::endl;
    }
  else
    std::cout << "Usage (beta):: PalSetup datafile file-type (1 = CSI, 2 = CSI Extended, 3 = TradeStation, 4 = Pinnacle, 5 = PAL)" << std::endl;

}


