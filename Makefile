PLUGIN_NAME = DAQ_Calibration

HEADERS = daq_calibration.h

SOURCES = daq_calibration.cpp\
          moc_daq_calibration.cpp\

LIBS = 

### Do not edit below this line ###

include $(shell rtxi_plugin_config --pkgdata-dir)/Makefile.plugin_compile
