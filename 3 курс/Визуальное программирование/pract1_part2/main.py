import sys
# Импорт основных виджетов и компонентов пользовательского интерфейса PySide6
from PySide6.QtWidgets import (
    QApplication, QMainWindow, QWidget,
    QVBoxLayout, QHBoxLayout, QGridLayout,
    QLCDNumber, QPushButton, QStatusBar
)
# Импорт вспомогательных классов: размеры, константы кнопок мыши, сигналы
from PySide6.QtCore import QSize, Qt, Signal
# Импорт класса действия для меню
from PySide6.QtGui import QAction


# ==============================================================================
# Класс MinesweeperButton: Кастомная кнопка игрового поля
# Наследуется от QPushButton для перехвата событий мыши (ЛКМ и ПКМ)
# ==============================================================================
class MinesweeperButton(QPushButton):
    # Пользовательские сигналы, передающие координаты ячейки (row, col)
    left_clicked = Signal(int, int)   # Сигнал при клике левой кнопкой мыши
    right_clicked = Signal(int, int)  # Сигнал при клике правой кнопкой мыши

    def __init__(self, row, col, parent=None):
        super().__init__(parent)
        self.row = row            # Номер строки ячейки в сетке
        self.col = col            # Номер столбца ячейки в сетке
        self.is_flagged = False   # Флаг состояния: установлена ли метка 🚩

    def mousePressEvent(self, event):
        """Переопределение обработчика нажатий кнопок мыши"""
        # Проверяем, нажата ли левая кнопка мыши (ЛКМ)
        if event.button() == Qt.MouseButton.LeftButton:
            # Открывать ячейку можно только если на ней нет флажка
            if not self.is_flagged:
                self.left_clicked.emit(self.row, self.col)

        # Проверяем, нажата ли правая кнопка мыши (ПКМ)
        elif event.button() == Qt.MouseButton.RightButton:
            self.right_clicked.emit(self.row, self.col)

        # Вызов базового метода для сохранения визуального эффекта нажатия
        super().mousePressEvent(event)


# ==============================================================================
# Класс MinesweeperWindow: Главное окно приложения
# ==============================================================================
class MinesweeperWindow(QMainWindow):
    def __init__(self, rows=10, cols=10, total_mines=10):
        super().__init__()
        # Инициализация параметров игрового поля
        self.rows = rows                  # Количество строк
        self.cols = cols                  # Количество столбцов
        self.total_mines = total_mines    # Общее количество мин
        self.flags_left = total_mines     # Счетчик доступных флажков

        # Заголовок окна
        self.setWindowTitle("Сапёр")

        # --- 1. Создание Центрального Виджета ---
        # QMainWindow требует единый центральный виджет для размещения элементов
        central_widget = QWidget()
        self.setCentralWidget(central_widget)

        # Главный вертикальный компоновщик (размещает верхнюю панель и сетку друг под другом)
        main_layout = QVBoxLayout()
        central_widget.setLayout(main_layout)

        # --- 2. Создание Верхнего Меню (QMenuBar) ---
        menu_bar = self.menuBar()
        game_menu = menu_bar.addMenu("Игра")  # Выпадающее меню "Игра"

        # Пункт меню "Новая игра"
        new_game_action = QAction("Новая игра", self)
        new_game_action.triggered.connect(self.reset_game)  # Связываем с методом сброса
        game_menu.addAction(new_game_action)

        # Пункт меню "Выход"
        exit_action = QAction("Выход", self)
        exit_action.triggered.connect(self.close)       # Связываем с закрытием окна
        game_menu.addAction(exit_action)

        # --- 3. Верхняя панель (Индикаторы и Смайлик) ---
        top_bar = QHBoxLayout()  # Горизонтальный компоновщик

        # Цифровое табло остатка мин/флагов
        self.mines_count = QLCDNumber()
        self.mines_count.display(self.flags_left)

        # Кнопка сброса (смайлик)
        self.reset_btn = QPushButton("🙂")
        self.reset_btn.setFixedSize(40, 40)
        self.reset_btn.clicked.connect(self.reset_game)

        # Цифровое табло таймера
        self.timer = QLCDNumber()
        self.timer.display(0)

        # Сборка верхней панели с помощью распорок (addStretch) для выравнивания
        top_bar.addWidget(self.mines_count)
        top_bar.addStretch()
        top_bar.addWidget(self.reset_btn)
        top_bar.addStretch()
        top_bar.addWidget(self.timer)

        main_layout.addLayout(top_bar)

        # --- 4. Генерация игрового поля (QGridLayout) ---
        grid_layout = QGridLayout()
        grid_layout.setSpacing(2)  # Расстояние между кнопками в пикселях
        self.buttons = {}          # Словарь для хранения кнопок по ключу (row, col)

        # Двойной цикл для создания сетки кнопок
        for r in range(self.rows):
            for c in range(self.cols):
                # Создаем кастомную кнопку с фиксированным размером
                btn = MinesweeperButton(r, c)
                btn.setFixedSize(QSize(30, 30))

                # Подключаем сигналы кликов к соответствующим слотам
                btn.left_clicked.connect(self.on_left_click)
                btn.right_clicked.connect(self.on_right_click)

                # Добавляем кнопку в сетку Layout и в наш словарь
                grid_layout.addWidget(btn, r, c)
                self.buttons[(r, c)] = btn

        main_layout.addLayout(grid_layout)

        # --- 5. Строка состояния (QStatusBar) ---
        self.setStatusBar(QStatusBar(self))
        self.statusBar().showMessage("ЛКМ — открыть ячейку, ПКМ — поставить флаг")

    # ==========================================================================
    # Слоты (методы-обработчики событий)
    # ==========================================================================
    def on_left_click(self, row, col):
        """Слот обработки открытия ячейки (ЛКМ)"""
        btn = self.buttons[(row, col)]
        if btn.isEnabled():
            btn.setEnabled(False)  # Визуально «прожимаем» кнопку (закрываем доступ)
            btn.setText("")       # В полной игре здесь выводится цифра мин вокруг
            self.statusBar().showMessage(f"Открыта ячейка [{row}, {col}]")

    def on_right_click(self, row, col):
        """Слот обработки установки/снятия флага (ПКМ)"""
        btn = self.buttons[(row, col)]

        # Если ячейка уже открыта — флаг ставить нельзя
        if not btn.isEnabled():
            return

        # Если флаг еще не стоит — устанавливаем
        if not btn.is_flagged:
            if self.flags_left > 0:
                btn.is_flagged = True
                btn.setText("🚩")
                self.flags_left -= 1
                self.statusBar().showMessage(f"Установлен флаг [{row}, {col}]")
        # Если флаг уже стоит — снимаем
        else:
            btn.is_flagged = False
            btn.setText("")
            self.flags_left += 1
            self.statusBar().showMessage(f"Снят флаг [{row}, {col}]")

        # Обновляем счетчик флагов на табло
        self.mines_count.display(self.flags_left)

    def reset_game(self):
        """Слот сброса игрового поля в исходное состояние"""
        self.flags_left = self.total_mines
        self.mines_count.display(self.flags_left)

        # Сбрасываем состояние каждой кнопки
        for btn in self.buttons.values():
            btn.setEnabled(True)
            btn.is_flagged = False
            btn.setText("")

        self.statusBar().showMessage("Поле сброшено. Новая игра")


# ==============================================================================
# Точка входа в программу
# ==============================================================================
if __name__ == "__main__":
    # Создание объекта приложения Qt (обязательно для любого Qt-приложения)
    app = QApplication(sys.argv)

    # Создание и отображение главного окна
    window = MinesweeperWindow(rows=10, cols=10, total_mines=10)
    window.show()

    # Запуск главного цикла обработки событий приложения
    sys.exit(app.exec())