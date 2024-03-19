from random import randint

import pyqtgraph as pg
import requests
from PyQt5 import QtCore, QtWidgets
from PyQt5.QtWidgets import (
	QHBoxLayout,
	QVBoxLayout, QWidget, QPushButton
)


class MainWindow(QtWidgets.QMainWindow):
	def __init__(self):
		super().__init__()
		main_widget = QWidget()

		# Temperature vs time dynamic plot
		buttons_and_plot_layout = QHBoxLayout()
		buttons_layout = QVBoxLayout()
		plot_layout = QVBoxLayout()

		buttons_and_plot_layout.addLayout(buttons_layout)
		buttons_and_plot_layout.addLayout(plot_layout)

		self.setBaseSize(1080, 920)
		self.plot_graph = pg.PlotWidget()
		plot_layout.addWidget(self.plot_graph)

		download_button = QPushButton(main_widget)
		download_button.setText('Download CSV')
		download_button.move(64, 64)
		download_button.clicked.connect(download_callback)
		buttons_layout.addWidget(download_button)

		start_button = QPushButton(main_widget)
		start_button.setText('Start')
		start_button.move(64, 64)
		start_button.clicked.connect(start_callback)
		buttons_layout.addWidget(start_button)

		main_widget.setLayout(buttons_and_plot_layout)
		self.setCentralWidget(main_widget)

		self.plot_graph.setBackground("w")
		pen = pg.mkPen(color=(255, 0, 0))
		self.plot_graph.setTitle("Temperature vs Time", color="b", size="20pt")
		styles = {"color": "red", "font-size": "18px"}
		self.plot_graph.setLabel("left", "Temperature (°C)", **styles)
		self.plot_graph.setLabel("bottom", "Time (min)", **styles)
		self.plot_graph.addLegend()
		self.plot_graph.showGrid(x=True, y=True)
		self.plot_graph.setYRange(20, 40)
		self.time = list(range(10))
		self.temperature = [randint(20, 40) for _ in range(10)]
		# Get a line reference
		self.line = self.plot_graph.plot(
			self.time,
			self.temperature,
			name="Temperature Sensor",
			pen=pen,
			symbol="+",
			symbolSize=15,
			symbolBrush="b",
		)
		# Add a timer to simulate new temperature measurements
		self.timer = QtCore.QTimer()
		self.timer.setInterval(300)
		self.timer.timeout.connect(self.update_plot)
		self.timer.start()

	def update_plot(self):
		self.time = self.time[1:]
		self.time.append(self.time[-1] + 1)
		self.temperature = self.temperature[1:]
		self.temperature.append(randint(20, 40))
		self.line.setData(self.time, self.temperature)


def download_callback():
	# resp = requests.post('http://192.168.4.1/ledon')
	r = requests.get('http://192.168.4.1/downloadData', allow_redirects=True)
	open('fart.csv', 'wb').write(r.content)


def start_callback():
	resp = requests.post('http://192.168.4.1/ledoff')
	print(resp.text)


app = QtWidgets.QApplication([])
main = MainWindow()
main.show()
app.exec()
