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

/*
 * This is a template header file for a user modules derived from
 * DefaultGUIModel with a custom GUI.
 */

#include <daq.h>
#include <default_gui_model.h>

class DAQCalibration : public DefaultGUIModel {

	Q_OBJECT

	public:
		DAQCalibration(void);
		virtual ~DAQCalibration(void);

		void execute(void);
		void createGUI(DefaultGUIModel::variable_t *, int);
		void customizeGUI(void);

	protected:
		virtual void update(DefaultGUIModel::update_flags_t);

	private:
		DAQ::Device *device;

		QStatusBar *statusBar;
		QComboBox *deviceList;
		QComboBox *analogSubdeviceList;
		QComboBox *analogChannelList;
		QGroupBox *deviceGroup;
		QGroupBox *analogGroup;

		size_t data_idx;
		double offset;
		double period;
		std::vector<double> channel_data;
		void calibrate();

		private slots:
			void updateDevice();
};
