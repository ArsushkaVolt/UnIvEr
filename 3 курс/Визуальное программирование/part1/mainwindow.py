import sys
from PySide6.QtWidgets import (
    QApplication, QPushButton, QLCDNumber,
    QStatusBar, QGridLayout
)
from PySide6.QtCore import QFile, QIODevice, QSize, Qt, Signal
from PySide6.QtUiTools import QUiLoader
from PySide6.QtGui import QAction


# ----------------------------------------------------------------------
# Кастомная кнопка для разделения кликов ЛКМ и ПКМ
# ----------------------------------------------------------------------
class CellButton(QPushButton):
    left_clicked = Signal(int, int)
    right_clicked = Signal(int, int)

    def __init__(self, row, col, parent=None):
        super().__init__(parent)
        self.row = row
        self.col = col
        self.is_flagged = False

    def mousePressEvent(self, event):
        if event.button() == Qt.MouseButton.LeftButton:
            if not self.is_flagged:
                self.left_clicked.emit(self.row, self.col)
        elif event.button() == Qt.MouseButton.RightButton:
            self.right_clicked.emit(self.row, self.col)
        super().mousePressEvent(event)


# ----------------------------------------------------------------------
# Класс управления макетом (без игровой логики)
# ----------------------------------------------------------------------
class MinesweeperLayout:
    def __init__(self, ui_file_path, rows=10, cols=10):
        self.rows = rows
        self.cols = cols

        # 1. Загрузка интерфейса из .ui файла
        loader = QUiLoader()
        file = QFile(ui_file_path)
        if not file.open(QIODevice.ReadOnly):
            print(f"Ошибка открытия файла: {ui_file_path}")
            sys.exit(-1)

        self.window = loader.load(file)
        file.close()

        # 2. Инициализация виджетов и построение сетки
        self.find_widgets()
        self.build_grid()

    def find_widgets(self):
        """Поиск компонентов UI по objectName"""
        self.lcd_mines = self.window.findChild(QLCDNumber, "lcdMines")
        self.lcd_timer = self.window.findChild(QLCDNumber, "lcdTimer")
        self.btn_reset = self.window.findChild(QPushButton, "btnReset")
        self.status_bar = self.window.findChild(QStatusBar, "statusbar")
        self.grid_layout = self.window.findChild(QGridLayout, "gridLayout")

        # Привязка меню
        action_new = self.window.findChild(QAction, "actionNewGame")
        if action_new:
            action_new.triggered.connect(self.reset_field)

        action_exit = self.window.findChild(QAction, "actionExit")
        if action_exit:
            action_exit.triggered.connect(self.window.close)

        # Привязка кнопки-смайлика
        if self.btn_reset:
            self.btn_reset.clicked.connect(self.reset_field)

        if self.lcd_mines:
            self.lcd_mines.display(10)
        if self.lcd_timer:
            self.lcd_timer.display(0)

    def build_grid(self):
        """Динамическое заполнение сетки кнопками 10x10"""
        self.buttons = {}
        if self.grid_layout:
            for r in range(self.rows):
                for c in range(self.cols):
                    btn = CellButton(r, c)
                    btn.setFixedSize(QSize(28, 28))

                    # Подключение сигналов мыши
                    btn.left_clicked.connect(self.on_left_click)
                    btn.right_clicked.connect(self.on_right_click)

                    self.grid_layout.addWidget(btn, r, c)
                    self.buttons[(r, c)] = btn

    def on_left_click(self, r, c):
        """ЛКМ: Простая имитация открытия ячейки"""
        btn = self.buttons[(r, c)]
        if btn.isEnabled():
            btn.setEnabled(False)  # Визуально «зажимаем» кнопку
            btn.setText("")       # Пустая открытая клетка
            if self.status_bar:
                self.status_bar.showMessage(f"Нажата ячейка [{r}, {c}]")

    def on_right_click(self, r, c):
        """ПКМ: Простая переключение флага 🚩"""
        btn = self.buttons[(r, c)]
        if not btn.isEnabled():
            return  # Открытые ячейки не трогаем

        if not btn.is_flagged:
            btn.is_flagged = True
            btn.setText("🚩")
            if self.status_bar:
                self.status_bar.showMessage(f"Установлен флаг [{r}, {c}]")
        else:
            btn.is_flagged = False
            btn.setText("")
            if self.status_bar:
                self.status_bar.showMessage(f"Снят флаг [{r}, {c}]")

    def reset_field(self):
        """Сброс состояния всех кнопок в первоначальный вид"""
        for btn in self.buttons.values():
            btn.setEnabled(True)
            btn.is_flagged = False
            btn.setText("")

        if self.status_bar:
            self.status_bar.showMessage("Макет сброшен в исходное состояние")

    def show(self):
        self.window.show()


# ----------------------------------------------------------------------
# Точка входа
# ----------------------------------------------------------------------
if __name__ == "__main__":
    app = QApplication(sys.argv)

    # Запуск макета
    demo = MinesweeperLayout("part1_design.ui")
    demo.show()

    sys.exit(app.exec())