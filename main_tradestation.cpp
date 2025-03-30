#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <exception>
#include "TimeSeriesCsvReader.h"
#include "TimeSeriesIndicators.h"

using namespace mkc_timeseries;
using std::shared_ptr;

int main(int argc, char **argv)
{
  std::vector<std::string> v(argv, argv + argc);
  if (argc == 2)
    {
      std::string historicDataFileName (v[1]);

	  TradeStationErrorCheckingFormatCsvReader<7> reader(historicDataFileName, 
						TimeFrame::DAILY,
						TradingVolume::CONTRACTS);
	  reader.readFile();
	  
	  std::shared_ptr<OHLCTimeSeries<7>> aTimeSeries = reader.getTimeSeries();
	  NumericTimeSeries<7> closingPrices (aTimeSeries->CloseTimeSeries());
	  NumericTimeSeries<7> rocOfClosingPrices (RocSeries (closingPrices, 1));

	  
	  std::vector<decimal<7>> aSortedVec (rocOfClosingPrices.getTimeSeriesAsVector());

	  decimal<7> medianOfRoc (Median (rocOfClosingPrices));
	  RobustQn<7> robustDev (rocOfClosingPrices);

	  decimal<7> robustQn (robustDev.getRobustQn());

	  std::cout << "Median of roc of close =" << medianOfRoc << std::endl;
	  std::cout << "Qn of roc of close  =" << robustQn << std::endl;
	  std::cout << "Printing ROC values" << std::endl;

 
    }
  else
    std::cout << "Usage (beta):: PalSetup datafile" << std::endl;

}


