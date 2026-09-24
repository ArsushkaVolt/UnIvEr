import sys
from PySide6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QPushButton,
    QVBoxLayout, QHBoxLayout, QLabel, QTextEdit, QStatusBar
)
from image_viewer import ImageViewer  # Импортируем наш виджет

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Тестирование компонента ImageViewer (Вариант 21)")
        self.resize(800, 600)

        self._init_ui()

    def _init_ui(self):
        central_widget = QWidget()
        self.setCentralWidget(central_widget)

        # 1. Создаем экземпляр нашего пользовательского компонента
        self.viewer = ImageViewer()

        # 2. Внешняя панель управления для проверки взаимодействия
        external_panel = QHBoxLayout()

        lbl_info = QLabel("Внешнее управление зумом:")
        btn_zoom_50 = QPushButton("50%")
        btn_zoom_100 = QPushButton("100%")
        btn_zoom_200 = QPushButton("200%")

        external_panel.addWidget(lbl_info)
        external_panel.addWidget(btn_zoom_50)
        external_panel.addWidget(btn_zoom_100)
        external_panel.addWidget(btn_zoom_200)
        external_panel.addStretch()

        # 3. Лог событий от виджета
        self.log_edit = QTextEdit()
        self.log_edit.setMaximumHeight(80)
        self.log_edit.setReadOnly(True)

        # Главная компоновка
        main_layout = QVBoxLayout(central_widget)
        main_layout.addLayout(external_panel)
        main_layout.addWidget(self.viewer, stretch=1)
        main_layout.addWidget(QLabel("Лог событий виджета:"))
        main_layout.addWidget(self.log_edit)

        # Статус-бар
        self.status_bar = QStatusBar()
        self.setStatusBar(self.status_bar)

        # --- Подключение внешних кнопок к слотам компонента ---
        btn_zoom_50.clicked.connect(lambda: self.viewer.set_zoom_factor(50))
        btn_zoom_100.clicked.connect(lambda: self.viewer.set_zoom_factor(100))
        btn_zoom_200.clicked.connect(lambda: self.viewer.set_zoom_factor(200))

        # --- Подписка на сигналы компонента ---
        self.viewer.imageLoaded.connect(self.on_image_loaded)
        self.viewer.zoomChanged.connect(self.on_zoom_changed)

    # --- Обработчики сигналов ---

    def on_image_loaded(self, path: str):
        msg = f"[Сигнал] Изображение успешно загружено: {path}"
        self.log_edit.append(msg)
        self.status_bar.showMessage(f"Файл: {path}", 5000)

    def on_zoom_changed(self, value: int):
        msg = f"[Сигнал] Масштаб изменен пользователем на: {value}%"
        self.log_edit.append(msg)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())