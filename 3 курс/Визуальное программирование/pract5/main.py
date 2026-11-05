import sys
import random
from PySide6.QtCore import Qt
from PySide6.QtGui import QPixmap, QBrush, QFont, QColor
from PySide6.QtWidgets import (
    QApplication,
    QMainWindow,
    QWidget,
    QVBoxLayout,
    QHBoxLayout,
    QPushButton,
    QGraphicsScene,
    QGraphicsView,
    QGraphicsPixmapItem,
    QGraphicsTextItem,
    QGraphicsItem,
)


# 1. Первый объект: Изображение "За Альянс" (при клике ПКМ меняет цвет фона сцены)
class AllianceItem(QGraphicsPixmapItem):
  def __init__(self, pixmap, scene, parent=None):
    super().__init__(pixmap, parent)
    self.scene_ref = scene
    self.is_blue_bg = False
    # Включаем возможность перемещения и выбора мышью
    self.setFlag(QGraphicsItem.ItemIsMovable)
    self.setFlag(QGraphicsItem.ItemIsSelectable)

  def mousePressEvent(self, event):
    if event.button() == Qt.RightButton and self.scene_ref:
      self.is_blue_bg = not self.is_blue_bg
      if self.is_blue_bg:
        self.scene_ref.setBackgroundBrush(QBrush(QColor("blue")))
      else:
        self.scene_ref.setBackgroundBrush(QBrush(QColor("white")))
      event.accept()
    else:
      super().mousePressEvent(event)


# 2. Второй объект: Текст "978-1616558451" (при клике ПКМ поворачивается на 60°)
class BookCodeItem(QGraphicsTextItem):
  def __init__(self, text, parent=None):
    super().__init__(text, parent)
    self.setFlag(QGraphicsItem.ItemIsMovable)
    self.setFlag(QGraphicsItem.ItemIsSelectable)
    # Установка центра для корректного вращения
    self.setTransformOriginPoint(self.boundingRect().center())

  def mousePressEvent(self, event):
    if event.button() == Qt.RightButton:
      current_rotation = self.rotation()
      self.setRotation(current_rotation + 60.0)
      event.accept()
    else:
      super().mousePressEvent(event)


# 3. Третий объект: Кубик d6 (при клике ПКМ меняет грань на случайную)
class DiceItem(QGraphicsPixmapItem):
  def __init__(self, faces, parent=None):
    super().__init__(parent)
    self.faces = faces
    self.current_index = 0
    self.setFlag(QGraphicsItem.ItemIsMovable)
    self.setFlag(QGraphicsItem.ItemIsSelectable)
    if self.faces:
      self.setPixmap(self.faces[0])

  def mousePressEvent(self, event):
    if event.button() == Qt.RightButton and self.faces:
      if len(self.faces) > 1:
        new_index = self.current_index
        while new_index == self.current_index:
          new_index = random.randint(0, len(self.faces) - 1)
        self.current_index = new_index
      else:
        self.current_index = 0
      self.setPixmap(self.faces[self.current_index])
      event.accept()
    else:
      super().mousePressEvent(event)


# Главное окно приложения
class MainWindow(QMainWindow):
  def __init__(self):
    super().__init__()
    self.setWindowTitle(
        "Практическая работа 5 — Вариант 21 (Python + PySide6)"
    )
    self.resize(1000, 700)

    # Инициализация графической сцены и вида
    self.scene = QGraphicsScene(0, 0, 800, 600, self)
    self.scene.setBackgroundBrush(QBrush(QColor("white")))
    self.view = QGraphicsView(self.scene, self)

    # Загрузка ресурсов (заглушки для демонстрации)
    self.load_resources()

    # Боковая панель управления с кнопками добавления объектов
    control_widget = QWidget(self)
    control_layout = QVBoxLayout(control_widget)

    btn_alliance = QPushButton("Добавить 'За Альянс'", control_widget)
    btn_book = QPushButton("Добавить Текст (ISBN)", control_widget)
    btn_dice = QPushButton("Добавить Кубик d6", control_widget)

    control_layout.addWidget(btn_alliance)
    control_layout.addWidget(btn_book)
    control_layout.addWidget(btn_dice)
    control_layout.addStretch()
    control_widget.setMaximumWidth(220)

    # Размещение элементов в окне
    main_container = QWidget(self)
    container_layout = QHBoxLayout(main_container)
    container_layout.addWidget(control_widget)
    container_layout.addWidget(self.view)
    self.setCentralWidget(main_container)

    # Связывание кнопок со слотами
    btn_alliance.clicked.connect(self.add_alliance_object)
    btn_book.clicked.connect(self.add_book_object)
    btn_dice.clicked.connect(self.add_dice_object)

  def load_resources(self):
    # Создание программных изображений-заглушек
    self.alliance_pixmap = QPixmap("alliance.jpeg").scaled(
    100, 100, Qt.KeepAspectRatio, Qt.SmoothTransformation
)


    # Создание 6 граней кубика разного цвета для имитации d6
    self.dice_faces = []
    face_colors = ["red", "green", "blue", "orange", "purple", "brown"]
    for col in face_colors:
      px = QPixmap(70, 70)
      px.fill(QColor(col))
      self.dice_faces.append(px)

  def add_alliance_object(self):
    item = AllianceItem(self.alliance_pixmap, self.scene)
    item.setPos(100, 100)
    self.scene.addItem(item)

  def add_book_object(self):
    item = BookCodeItem("978-1616558451")
    font = QFont("Arial", 14, QFont.Bold)
    item.setFont(font)
    item.setDefaultTextColor(QColor("darkblue"))
    item.setPos(200, 200)
    self.scene.addItem(item)

  def add_dice_object(self):
    item = DiceItem(self.dice_faces)
    item.setPos(300, 300)
    self.scene.addItem(item)


if __name__ == "__main__":
  app = QApplication(sys.argv)
  window = MainWindow()
  window.show()
  sys.exit(app.exec())