import os
from PySide6.QtCore import Qt, Signal, Slot
from PySide6.QtGui import QPixmap
from PySide6.QtWidgets import (
    QWidget, QLabel, QSlider, QPushButton, QScrollArea,
    QVBoxLayout, QHBoxLayout, QFileDialog, QMessageBox
)

class ImageViewer(QWidget):
    """
    Пользовательский компонент просмотра изображений.
    Вариант 21: Область отображения, слайдер зума (1-300%),
    кнопки "Открыть" и "Сохранить как".
    """

    # Пользовательские сигналы для оповещения внешнего приложения
    imageLoaded = Signal(str)  # Передает путь к загруженному файлу
    zoomChanged = Signal(int)  # Передает текущий процент масштаба

    def __init__(self, parent=None):
        super().__init__(parent)

        self._original_pixmap = QPixmap()
        self._current_zoom = 100  # Значение по умолчанию: 100%

        self._init_ui()

    def _init_ui(self):
        # 1. Виджет отображения картинки
        self.image_label = QLabel()
        self.image_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.image_label.setScaledContents(True)

        # 2. Область прокрутки (ScrollArea) для просмотра больших картинок
        self.scroll_area = QScrollArea()
        self.scroll_area.setWidget(self.image_label)
        self.scroll_area.setAlignment(Qt.AlignmentFlag.AlignCenter)

        # 3. Кнопки управления
        self.btn_open = QPushButton("Открыть")
        self.btn_save_as = QPushButton("Сохранить как")

        # 4. Слайдер масштаба (1 - 300%)
        self.zoom_slider = QSlider(Qt.Orientation.Horizontal)
        self.zoom_slider.setRange(1, 300)
        self.zoom_slider.setValue(100)

        self.zoom_value_label = QLabel("100%")

        # --- Компоновка интерфейса ---
        top_layout = QHBoxLayout()
        top_layout.addWidget(self.btn_open)
        top_layout.addWidget(self.btn_save_as)
        top_layout.addSpacing(20)
        top_layout.addWidget(QLabel("Масштаб:"))
        top_layout.addWidget(self.zoom_slider)
        top_layout.addWidget(self.zoom_value_label)

        main_layout = QVBoxLayout()
        main_layout.addLayout(top_layout)
        main_layout.addWidget(self.scroll_area)

        self.setLayout(main_layout)

        # --- Привязка сигналов и слотов внутри компонента ---
        self.btn_open.clicked.connect(self._on_open_clicked)
        self.btn_save_as.clicked.connect(self._on_save_as_clicked)
        self.zoom_slider.valueChanged.connect(self._on_slider_value_changed)

    # --- Внешний публичный интерфейс (Методы / Слоты) ---

    @Slot(str)
    def load_image(self, file_path: str):
        """Внешний метод/слот для загрузки изображения по пути."""
        if not file_path or not os.path.exists(file_path):
            return

        pixmap = QPixmap(file_path)
        if not pixmap.isNull():
            self._original_pixmap = pixmap
            self._update_display()
            self.imageLoaded.emit(file_path)
        else:
            QMessageBox.warning(self, "Ошибка", "Не удалось загрузить изображение.")

    @Slot(int)
    def set_zoom_factor(self, percent: int):
        """Внешний метод/слот для изменения масштаба (1-300%)."""
        percent = max(1, min(300, percent))
        self.zoom_slider.setValue(percent)

    def get_current_pixmap(self) -> QPixmap:
        """Возвращает оригинальный QPixmap."""
        return self._original_pixmap

    # --- Внутренние приватные слоты ---

    @Slot()
    def _on_open_clicked(self):
        file_path, _ = QFileDialog.getOpenFileName(
            self,
            "Открыть изображение",
            "",
            "Изображения (*.png *.jpg *.jpeg *.bmp *.gif)"
        )
        if file_path:
            self.load_image(file_path)

    @Slot()
    def _on_save_as_clicked(self):
        if self._original_pixmap.isNull():
            QMessageBox.information(self, "Информация", "Сначала откройте изображение.")
            return

        file_path, selected_filter = QFileDialog.getSaveFileName(
            self,
            "Сохранить изображение как",
            "",
            "PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp)"
        )
        if file_path:
            self._original_pixmap.save(file_path)

    @Slot(int)
    def _on_slider_value_changed(self, value: int):
        self._current_zoom = value
        self.zoom_value_label.setText(f"{value}%")
        self._update_display()
        # Извещаем внешнее приложение об изменении зума
        self.zoomChanged.emit(value)

    def _update_display(self):
        if self._original_pixmap.isNull():
            return

        # Вычисляем новый размер на основе процента масштабирования
        new_size = self._original_pixmap.size() * (self._current_zoom / 100.0)
        self.image_label.resize(new_size)

        # Масштабируем с сохранением пропорций и сглаживанием
        scaled_pixmap = self._original_pixmap.scaled(
            new_size,
            Qt.AspectRatioMode.KeepAspectRatio,
            Qt.TransformationMode.SmoothTransformation
        )
        self.image_label.setPixmap(scaled_pixmap)# This Python file uses the following encoding: utf-8

# if __name__ == "__main__":
#     pass
