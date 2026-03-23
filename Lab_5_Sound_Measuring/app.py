import sys
import csv
from datetime import datetime

import serial
import serial.tools.list_ports

from PyQt6.QtCore import QTimer

from PyQt6.QtWidgets import (
    QApplication,
    QWidget,
    QVBoxLayout,
    QLabel,
    QProgressBar,
    QListWidget
)

BAUD_RATE = 9600
CSV_FILE = "sound_events_db.csv"


def find_serial_port():
    ports = list(serial.tools.list_ports.comports())

    for p in ports:
        desc = (p.description or "").lower()
        if "arduino" in desc or "ch340" in desc or "usb serial" in desc:
            return p.device

    if ports:
        return ports[0].device

    return None


class SoundGUI(QWidget):
    def __init__(self):
        super().__init__()

        self.setWindowTitle("Sound dB Monitor GUI")
        self.resize(800, 400)

        self.serial_port = None
        self.prev_above = False

        self.status_label = QLabel("Status: starting...")
        self.db_label = QLabel("Current Sound Level: 0.0 dB")
        self.event_label = QLabel("Interrupt Event Count: 0")
        self.threshold_label = QLabel("Threshold State: below")

        self.bar = QProgressBar()
        self.bar.setRange(0, 60)
        self.bar.setValue(0)
        self.bar.setFormat("dB: %v")

        self.log_list = QListWidget()

        layout = QVBoxLayout()
        layout.addWidget(self.status_label)
        layout.addWidget(self.db_label)
        layout.addWidget(self.event_label)
        layout.addWidget(self.threshold_label)
        layout.addWidget(self.bar)
        layout.addWidget(QLabel("Logged Threshold Events (dB)"))
        layout.addWidget(self.log_list)
        self.setLayout(layout)

        self.csv_file = open(CSV_FILE, "w", newline="", encoding="utf-8")
        self.csv_writer = csv.writer(self.csv_file)

        if self.csv_file.tell() == 0:
            self.csv_writer.writerow(["pc_time", "arduino_ms", "db_level"])

        port = find_serial_port()
        if port is None:
            self.status_label.setText("Status: No serial port found")
        else:
            try:
                self.serial_port = serial.Serial(port, BAUD_RATE, timeout=0.05)
                self.status_label.setText(f"Status: Connected to {port}")
            except Exception as e:
                self.status_label.setText(f"Status: Error opening port: {e}")

        self.timer = QTimer()
        self.timer.timeout.connect(self.read_serial)
        self.timer.start(50)

    def read_serial(self):
        if not self.serial_port:
            return

        while self.serial_port.in_waiting:
            try:
                raw = self.serial_port.readline().decode(errors="ignore").strip()
                if not raw:
                    continue

                parts = raw.split(",")
                if len(parts) != 4:
                    continue

                arduino_ms = int(parts[0])
                db_level = float(parts[1])
                above = int(parts[2])
                event_count = int(parts[3])

                self.db_label.setText(f"Current Sound Level: {db_level:.1f} dB")
                self.event_label.setText(f"Interrupt Event Count: {event_count}")

                if above == 1:
                    self.threshold_label.setText("Threshold State: ABOVE")
                    self.bar.setStyleSheet("QProgressBar::chunk { background-color: red; }")
                else:
                    self.threshold_label.setText("Threshold State: below")
                    self.bar.setStyleSheet("QProgressBar::chunk { background-color: green; }")

                bar_value = max(0, int(db_level))
                self.bar.setMaximum(max(60, bar_value + 5))
                self.bar.setValue(bar_value)

                if above == 1 and not self.prev_above:
                    pc_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                    self.csv_writer.writerow([pc_time, arduino_ms, f"{db_level:.1f}"])
                    self.csv_file.flush()

                    self.log_list.insertItem(0, f"{pc_time} | {db_level:.1f} dB")

                    if self.log_list.count() > 15:
                        self.log_list.takeItem(15)

                self.prev_above = (above == 1)

            except Exception:
                pass

    def closeEvent(self, event):
        try:
            if self.serial_port:
                self.serial_port.close()
        except Exception:
            pass

        try:
            self.csv_file.close()
        except Exception:
            pass

        event.accept()


def main():
    app = QApplication(sys.argv)
    window = SoundGUI()
    window.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()
