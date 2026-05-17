import sys
import csv
import os
from datetime import datetime

import serial
import matplotlib.pyplot as plt

from PyQt6.QtCore import QTimer
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QLabel, QLineEdit, QPushButton,
    QMessageBox, QVBoxLayout, QHBoxLayout, QGridLayout, QGroupBox,
    QPlainTextEdit
)

# file to save round data with columns:
# timestamp, player1, player2, round_winner, reaction1_ms, reaction2_ms, false_start, score1, score2
ROUNDS_CSV = "rounds.csv"
# file to save match data with columns: 
# timestamp, player1, player2, match_winner
MATCHES_CSV = "matches.csv"


def ensure_csv_files():
    if not os.path.exists(ROUNDS_CSV):
        with open(ROUNDS_CSV, "w", newline="", encoding="utf-8") as f:
            writer = csv.writer(f)
            writer.writerow([
                "timestamp",
                "player1",
                "player2",
                "round_winner",
                "reaction1_ms",
                "reaction2_ms",
                "false_start",
                "score1",
                "score2"
            ])

    if not os.path.exists(MATCHES_CSV):
        with open(MATCHES_CSV, "w", newline="", encoding="utf-8") as f:
            writer = csv.writer(f)
            writer.writerow([
                "timestamp",
                "player1",
                "player2",
                "match_winner"
            ])


class ReactionGameWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Reaction Game - PyQt6")
        self.resize(760, 560)

        ensure_csv_files() # make sure CSV files exist before we try to write to them later

        self.ser = None         # serial port object
        self.connected = False  # whether we are currently connected to Arduino

        self.current_player1 = ""
        self.current_player2 = ""

        self.build_ui() # build the user interface
    
        # timer to poll serial port for incoming data every 100 ms
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.poll_serial)
        self.timer.start(100)

    def build_ui(self):
        central = QWidget()
        self.setCentralWidget(central)

        main_layout = QVBoxLayout()
        central.setLayout(main_layout)

        # -------- Connection --------
        conn_box = QGroupBox("Connection")
        conn_layout = QHBoxLayout()

        self.port_label = QLabel("COM Port:")
        self.port_edit = QLineEdit("COM21")

        self.connect_btn = QPushButton("Connect")
        self.disconnect_btn = QPushButton("Disconnect")
        self.ping_btn = QPushButton("Ping")

        self.status_label = QLabel("Not connected")

        self.connect_btn.clicked.connect(self.connect_serial)
        self.disconnect_btn.clicked.connect(self.disconnect_serial)
        self.ping_btn.clicked.connect(lambda: self.send_line("PING"))

        conn_layout.addWidget(self.port_label)
        conn_layout.addWidget(self.port_edit)
        conn_layout.addWidget(self.connect_btn)
        conn_layout.addWidget(self.disconnect_btn)
        conn_layout.addWidget(self.ping_btn)
        conn_layout.addWidget(self.status_label)

        conn_box.setLayout(conn_layout)
        main_layout.addWidget(conn_box)

        # -------- Players --------
        players_box = QGroupBox("Players")
        players_layout = QGridLayout()

        self.p1_label = QLabel("Player 1:")
        self.p1_edit = QLineEdit("Player1")

        self.p2_label = QLabel("Player 2:")
        self.p2_edit = QLineEdit("Player2")

        self.start_btn = QPushButton("Start Match")
        self.reset_btn = QPushButton("Reset")
        self.stats_btn = QPushButton("Show Stats")

        self.start_btn.clicked.connect(self.start_match)
        self.reset_btn.clicked.connect(self.reset_match)
        self.stats_btn.clicked.connect(self.show_stats)

        players_layout.addWidget(self.p1_label, 0, 0)
        players_layout.addWidget(self.p1_edit, 0, 1)
        players_layout.addWidget(self.p2_label, 0, 2)
        players_layout.addWidget(self.p2_edit, 0, 3)

        players_layout.addWidget(self.start_btn, 1, 0, 1, 2)
        players_layout.addWidget(self.reset_btn, 1, 2)
        players_layout.addWidget(self.stats_btn, 1, 3)

        players_box.setLayout(players_layout)
        main_layout.addWidget(players_box)

        # -------- Score / Status --------
        score_box = QGroupBox("Score")
        score_layout = QGridLayout()

        self.score1_title = QLabel("Player 1 Score:")
        self.score1_value = QLabel("0")

        self.score2_title = QLabel("Player 2 Score:")
        self.score2_value = QLabel("0")

        self.game_status_title = QLabel("Status:")
        self.game_status_value = QLabel("Idle")

        score_layout.addWidget(self.score1_title, 0, 0)
        score_layout.addWidget(self.score1_value, 0, 1)

        score_layout.addWidget(self.score2_title, 0, 2)
        score_layout.addWidget(self.score2_value, 0, 3)

        score_layout.addWidget(self.game_status_title, 1, 0)
        score_layout.addWidget(self.game_status_value, 1, 1, 1, 3)

        score_box.setLayout(score_layout)
        main_layout.addWidget(score_box)

        # -------- Log --------
        log_box = QGroupBox("Log")
        log_layout = QVBoxLayout()

        self.log_edit = QPlainTextEdit()
        self.log_edit.setReadOnly(True)

        log_layout.addWidget(self.log_edit)
        log_box.setLayout(log_layout)
        main_layout.addWidget(log_box)

    def log(self, text):
        timestamp = datetime.now().strftime("%H:%M:%S")
        self.log_edit.appendPlainText(f"[{timestamp}] {text}")

    def connect_serial(self):
        if self.connected:
            QMessageBox.information(self, "Info", "Already connected.")
            return

        port = self.port_edit.text().strip()
        if not port:
            QMessageBox.warning(self, "Warning", "Enter COM port first.")
            return

        try:
            self.ser = serial.Serial(port, 115200, timeout=0.05)
            self.connected = True
            self.status_label.setText(f"Connected: {port}")
            self.game_status_value.setText("Connected")
            self.log(f"Connected to {port}")
        except Exception as e:
            QMessageBox.critical(self, "Serial Error", str(e))

    def disconnect_serial(self):
        try:
            if self.ser and self.ser.is_open:
                self.ser.close()
        except:
            pass

        self.ser = None
        self.connected = False
        self.status_label.setText("Not connected")
        self.game_status_value.setText("Disconnected")
        self.log("Disconnected")

    def send_line(self, text):
        if not self.connected or not self.ser or not self.ser.is_open:
            QMessageBox.warning(self, "Warning", "Serial is not connected.")
            return

        try:
            self.ser.write((text + "\n").encode("utf-8"))
            self.log(f"TX -> {text}")
        except Exception as e:
            QMessageBox.critical(self, "Serial Write Error", str(e))

    def start_match(self):
        if not self.connected:
            QMessageBox.warning(self, "Warning", "Connect to Arduino first.")
            return

        p1 = self.p1_edit.text().strip() or "Player1"
        p2 = self.p2_edit.text().strip() or "Player2"

        self.current_player1 = p1
        self.current_player2 = p2

        self.score1_value.setText("0")
        self.score2_value.setText("0")
        self.game_status_value.setText("Match started")

        self.send_line(f"START,{p1},{p2}")

    def reset_match(self):
        if self.connected:
            self.send_line("RESET")

        self.score1_value.setText("0")
        self.score2_value.setText("0")
        self.game_status_value.setText("Idle")
        self.log("UI reset")

    # This function is called every 100 ms by the timer to check if there is any incoming data from the serial port and process it
    def poll_serial(self):
        if self.connected and self.ser and self.ser.is_open:
            try:
                while self.ser.in_waiting:
                    raw = self.ser.readline().decode("utf-8", errors="ignore").strip()
                    if raw:
                        self.handle_line(raw)
            except Exception as e:
                self.log(f"Serial read error: {e}")

    # This function processes a single line of text received from the Arduino and updates the UI accordingly. 
    # It also saves round and match data to CSV files when relevant lines are received.
    def handle_line(self, line):
        self.log(f"RX <- {line}")

        if line == "READY":
            self.game_status_value.setText("Arduino ready")
            return

        if line == "PONG":
            self.game_status_value.setText("Connection OK")
            return

        if line == "START_OK":
            self.game_status_value.setText("Match accepted")
            return

        if line == "ROUND_WAIT":
            self.game_status_value.setText("Waiting random delay...")
            return

        if line == "GO":
            self.game_status_value.setText("GO! Press buttons!")
            return

        if line == "RESET_OK":
            self.game_status_value.setText("Reset done")
            self.score1_value.setText("0")
            self.score2_value.setText("0")
            return

        if line.startswith("ROUND,"):
            self.handle_round_line(line)
            return

        if line.startswith("MATCH,"):
            self.handle_match_line(line)
            return

        if line.startswith("ERR,"):
            self.game_status_value.setText(line)

    # This function processes a line that starts with "ROUND," which contains round results and updates the score and status in the UI. It also saves the round data to the rounds.csv file.
    def handle_round_line(self, line):
        parts = line.split(",")
        if len(parts) != 7:
            return

        winner = int(parts[1])
        reaction1 = int(parts[2])
        reaction2 = int(parts[3])
        false_start = int(parts[4])
        score1 = int(parts[5])
        score2 = int(parts[6])

        self.score1_value.setText(str(score1))
        self.score2_value.setText(str(score2))

        if winner == 1:
            winner_name = self.current_player1
        elif winner == 2:
            winner_name = self.current_player2
        else:
            winner_name = "DRAW"

        if false_start:
            self.game_status_value.setText(f"False start. Winner: {winner_name}")
        else:
            self.game_status_value.setText(f"Round winner: {winner_name}")

        self.save_round(
            player1=self.current_player1,
            player2=self.current_player2,
            round_winner=winner_name,
            reaction1_ms=reaction1,
            reaction2_ms=reaction2,
            false_start=false_start,
            score1=score1,
            score2=score2
        )

    # This function processes a line that starts with "MATCH," which contains the match winner and updates the status in the UI. It also saves the match data to the matches.csv file.
    def handle_match_line(self, line):
        parts = line.split(",", 1)
        if len(parts) != 2:
            return

        match_winner = parts[1].strip()
        self.game_status_value.setText(f"Match winner: {match_winner}")

        self.save_match(
            player1=self.current_player1,
            player2=self.current_player2,
            match_winner=match_winner
        )

        QMessageBox.information(self, "Match Over", f"Winner: {match_winner}")

    # This function saves the round data to the rounds.csv file with the following columns:
    # timestamp, player1, player2, round_winner, reaction1_ms, reaction2_ms, false_start, score1, score2
    def save_round(self, player1, player2, round_winner, reaction1_ms, reaction2_ms, false_start, score1, score2):
        with open(ROUNDS_CSV, "a", newline="", encoding="utf-8") as f:
            writer = csv.writer(f)
            writer.writerow([
                datetime.now().isoformat(timespec="seconds"),
                player1,
                player2,
                round_winner,
                reaction1_ms,
                reaction2_ms,
                false_start,
                score1,
                score2
            ])

    # This function saves the match data to the matches.csv file with the following columns:
    # timestamp, player1, player2, match_winner
    def save_match(self, player1, player2, match_winner):
        with open(MATCHES_CSV, "a", newline="", encoding="utf-8") as f:
            writer = csv.writer(f)
            writer.writerow([
                datetime.now().isoformat(timespec="seconds"),
                player1,
                player2,
                match_winner
            ])

    # This function reads the rounds.csv and matches.csv files to calculate statistics for the currently entered player names and shows a bar chart comparing their average reaction times, as well as their round wins and match wins against each other.
    def show_stats(self):
        p1 = self.p1_edit.text().strip()
        p2 = self.p2_edit.text().strip()

        if not p1 or not p2:
            QMessageBox.warning(self, "Warning", "Enter both player names first.")
            return

        r1_list = []
        r2_list = []
        round_wins_p1 = 0
        round_wins_p2 = 0
        match_wins_p1 = 0
        match_wins_p2 = 0

        if os.path.exists(ROUNDS_CSV):
            with open(ROUNDS_CSV, "r", newline="", encoding="utf-8") as f:
                reader = csv.DictReader(f)
                for row in reader:
                    a = row["player1"]
                    b = row["player2"]

                    if {a, b} != {p1, p2}:      # only consider rounds between the two currently entered players, ignore all other data
                        continue

                    rw = row["round_winner"]

                    try:
                        ra = int(row["reaction1_ms"])
                        rb = int(row["reaction2_ms"])
                    except:
                        continue

                    if a == p1:
                        if ra >= 0:
                            r1_list.append(ra)
                        if rb >= 0:
                            r2_list.append(rb)
                    else:
                        if rb >= 0:
                            r1_list.append(rb)
                        if ra >= 0:
                            r2_list.append(ra)

                    if rw == p1:
                        round_wins_p1 += 1
                    elif rw == p2:
                        round_wins_p2 += 1

        if os.path.exists(MATCHES_CSV):
            with open(MATCHES_CSV, "r", newline="", encoding="utf-8") as f:
                reader = csv.DictReader(f)
                for row in reader:
                    a = row["player1"]
                    b = row["player2"]

                    if {a, b} != {p1, p2}:      # only consider matches between the two currently entered players, ignore all other data
                        continue

                    mw = row["match_winner"]
                    if mw == p1:
                        match_wins_p1 += 1
                    elif mw == p2:
                        match_wins_p2 += 1

        if not r1_list and not r2_list:
            QMessageBox.information(self, "No Data", "No saved data for this player pair yet.")
            return

        # calculate average reaction times, ignoring -1 values which indicate no valid reaction time (e.g. due to false start or not pressing in time)
        avg1 = sum(r1_list) / len(r1_list) if r1_list else 0
        avg2 = sum(r2_list) / len(r2_list) if r2_list else 0

        plt.figure(figsize=(7, 4))
        plt.bar([p1, p2], [avg1, avg2])
        plt.ylabel("Average Reaction Time (ms)")
        plt.title(
            f"Head-to-Head Stats\n"
            f"Round Wins: {p1}={round_wins_p1}, {p2}={round_wins_p2} | "
            f"Match Wins: {p1}={match_wins_p1}, {p2}={match_wins_p2}"
        )
        plt.tight_layout()
        plt.show()

    def closeEvent(self, event):
        self.disconnect_serial()
        event.accept()


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = ReactionGameWindow()
    window.show()
    sys.exit(app.exec())