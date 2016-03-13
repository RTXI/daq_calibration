/*
	 Copyright (C) 2011 Georgia Institute of Technology, University of Utah, Weill Cornell Medical College

	 This program is free software: you can redistribute it and/or modify
	 it under the terms of the GNU General Public License as published by
	 the Free Software Foundation, either version 3 of the License, or
	 (at your option) any later version.

	 This program is distributed in the hope that it will be useful,
	 but WITHOUT ANY WARRANTY; without even the implied warranty of
	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	 GNU General Public License for more details.

	 You should have received a copy of the GNU General Public License
	 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "daq_calibration.h"
#include <main_window.h>
#include <iostream>

extern "C" Plugin::Object *createRTXIPlugin(void)
{
	return new DAQCalibration();
}

struct find_daq_t
{
	int index;
	DAQ::Device *device;
};

static void findDAQDevice(DAQ::Device *dev,void *arg)
{
	struct find_daq_t *info = static_cast<struct find_daq_t *>(arg);
	if(!info->index)
		info->device = dev;
	info->index--;
}

static void buildDAQDeviceList(DAQ::Device *dev,void *arg)
{
	QComboBox *deviceList = static_cast<QComboBox *>(arg);
	deviceList->addItem(QString::fromStdString(dev->getName()));
}

static DefaultGUIModel::variable_t vars[] = {
	{ "Channel", "Data for channel to calibrate", DefaultGUIModel::INPUT, },
	{ "Offset", "Calibration offset value", DefaultGUIModel::STATE, }, 
};

static size_t num_vars = sizeof(vars) / sizeof(DefaultGUIModel::variable_t);

DAQCalibration::DAQCalibration(void) : DefaultGUIModel("DAQ Calibration Utility", ::vars, ::num_vars) {
	setWhatsThis("<p><b>DAQCalibration:</b><br>QWhatsThis description.</p>");
	DefaultGUIModel::createGUI(vars, num_vars);
	customizeGUI();
	update(INIT);
	refresh();
	QTimer::singleShot(0, this, SLOT(resizeMe()));
}

DAQCalibration::~DAQCalibration(void) { }

void DAQCalibration::execute(void)
{
	if(data_idx < 1/period)
	{
		channel_data.push_back(input(0));
		data_idx++;

		if(data_idx == 1/period)
			calibrate();
	}
	return;
}

void DAQCalibration::calibrate()
{
	// Offset for the channel
	offset = std::accumulate(channel_data.begin(), channel_data.end(), 0.0) / channel_data.size();

	// Apply channel settings
	DAQ::Device *dev;
	{
		struct find_daq_t info = { deviceList->currentIndex(), 0, };
		DAQ::Manager::getInstance()->foreachDevice(findDAQDevice, &info);
		dev = info.device;
	}

	if(dev)
	{
		// Get channel
		DAQ::index_t a_chan = analogChannelList->currentIndex();
		DAQ::type_t a_type = static_cast<DAQ::type_t>(analogSubdeviceList->currentIndex());

		// Store offset in DAQ
		dev->setAnalogCalibrationValue(a_type, a_chan, offset);
	}

	// Pause
	statusBar->showMessage(tr("Status: Calibration complete."));
	pauseButton->setChecked(true);
}

void DAQCalibration::update(DefaultGUIModel::update_flags_t flag) {
	switch (flag) {
		case INIT:
			period = RT::System::getInstance()->getPeriod() * 1e-9; // s
			setState("Offset", offset);
			offset = 0;
			break;

		case MODIFY:
			break;

		case UNPAUSE:
			data_idx = 0;
			offset = 0;
			break;

		case PAUSE:
			data_idx = 0;
			channel_data.clear();
			break;

		case PERIOD:
			period = RT::System::getInstance()->getPeriod() * 1e-9; // s
			break;

		default:
			break;
	}
}

void DAQCalibration::customizeGUI(void) {
	QGridLayout *customlayout = DefaultGUIModel::getLayout();

	DefaultGUIModel::pauseButton->setText("Calibrate");

	QGroupBox *daq_group = new QGroupBox;
	QHBoxLayout *daqlayout = new QHBoxLayout;
	daqlayout->addWidget(new QLabel(tr("Device:")));
	deviceList = new QComboBox;
	daqlayout->addWidget(deviceList);
	DAQ::Manager::getInstance()->foreachDevice(buildDAQDeviceList,deviceList);
	QObject::connect(deviceList,SIGNAL(activated(int)),this,SLOT(updateDevice(void)));

	analogSubdeviceList = new QComboBox;
	analogSubdeviceList->addItem("Input");
	analogSubdeviceList->addItem("Output");
	QObject::connect(analogSubdeviceList,SIGNAL(activated(int)),this,SLOT(updateDevice(void)));
	daqlayout->addWidget(analogSubdeviceList);
	analogChannelList = new QComboBox;
	daq_group->setLayout(daqlayout);
	daqlayout->addWidget(analogChannelList);

	QGroupBox *status_group = new QGroupBox;
	QHBoxLayout *status_layout = new QHBoxLayout;
	statusBar = new QStatusBar();
	statusBar->setSizeGripEnabled(false);
	statusBar->showMessage(tr("Status: Please specify a channel..."));
	status_group->setLayout(status_layout);
	status_layout->addWidget(statusBar);

	updateDevice();

	customlayout->addWidget(daq_group, 0, 0);
	customlayout->addWidget(status_group, 2, 0);
	setLayout(customlayout);
}

void DAQCalibration::updateDevice(void)
{
	DAQ::Device *dev;
	DAQ::type_t type;
	{
		struct find_daq_t info = { deviceList->currentIndex(), 0, };
		DAQ::Manager::getInstance()->foreachDevice(findDAQDevice,&info);
		dev = info.device;
	}

	if(!dev)
		return;

	analogChannelList->clear();

	type = static_cast<DAQ::type_t>(analogSubdeviceList->currentIndex());
	for(size_t i=0; i<dev->getChannelCount(type); ++i)
		analogChannelList->addItem(QString::number(i));
}
