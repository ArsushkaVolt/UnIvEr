import random
import sys
from PySide6.QtCore import QAbstractListModel, QModelIndex, Qt, QTimer
from PySide6.QtWidgets import (
    QApplication,
    QFormLayout,
    QHBoxLayout,
    QHeaderView,
    QLabel,
    QListView,
    QMainWindow,
    QPushButton,
    QSlider,
    QSplitter,
    QSpinBox,
    QStatusBar,
    QTableView,
    QVBoxLayout,
    QWidget,
)


# Структура данных покупателя
class Customer:

  def __init__(self, name, drink_id, drink_name, price, budget, max_wait):
    self.name = name
    self.drink_id = drink_id
    self.drink_name = drink_name
    self.price = price
    self.budget = budget
    self.max_wait = max_wait
    self.current_wait = 0
    self.status_info = 'В очереди'


# Собственная модель данных, унаследованная от QAbstractListModel
class CustomerListModel(QAbstractListModel):

  def __init__(self, customers=None, parent=None):
    super().__init__(parent)
    self.customers = customers if customers is not None else []

  def rowCount(self, parent=QModelIndex()):
    return len(self.customers)

  def data(self, index, role=Qt.DisplayRole):
    if not index.isValid():
      return None
    row = index.row()
    if row < 0 or row >= len(self.customers):
      return None

    cust = self.customers[row]
    if role == Qt.DisplayRole:
      return (
          f'{cust.name} | Заказ: {cust.drink_name} ({cust.price} руб.) |'
          f' Бюджет: {cust.budget} руб. | Ожидание: {cust.current_wait}/'
          f'{cust.max_wait}с | Статус: {cust.status_info}'
      )
    return None

  def add_customer(self, customer):
    self.beginInsertRows(QModelIndex(), len(self.customers), len(self.customers))
    self.customers.append(customer)
    self.endInsertRows()

  def remove_customer(self, row):
    if 0 <= row < len(self.customers):
      self.beginRemoveRows(QModelIndex(), row, row)
      cust = self.customers.pop(row)
      self.endRemoveRows()
      return cust
    return None

  def clear(self):
    self.beginResetModel()
    self.customers.clear()
    self.endResetModel()


class CoffeeMachineWindow(QMainWindow):

  def __init__(self):
    super().__init__()
    self.setWindowTitle(
        'Практическая работа №4 - Автомат по продаже кофе (Вариант 21)'
    )
    self.resize(1000, 650)

    # Меню напитков (id: [название, цена, время приготовления в сек])
    self.drinks = {
        1: {'name': 'Эспрессо', 'price': 100, 'time': 2},
        2: {'name': 'Капучино', 'price': 150, 'time': 3},
        3: {'name': 'Латте', 'price': 180, 'time': 4},
        4: {'name': 'Раф', 'price': 220, 'time': 5},
    }

    # Инициализация моделей (MVC)
    self.queue_model = CustomerListModel()
    self.satisfied_model = CustomerListModel()
    self.dissatisfied_model = CustomerListModel()

    self.current_processing_customer = None
    self.processing_time_left = 0

    self.init_ui()

    # Таймер симуляции (такт = 1 секунда)
    self.timer = QTimer(self)
    self.timer.setInterval(1000)
    self.timer.timeout.connect(self.simulation_step)

  def init_ui(self):
    central_widget = QWidget()
    main_layout = QHBoxLayout(central_widget)

    # Левая панель: Управление и настройки
    control_layout = QVBoxLayout()

    form_layout = QFormLayout()
    self.prob_spin = QSpinBox()
    self.prob_spin.setRange(5, 100)
    self.prob_spin.setValue(40)
    form_layout.addRow('Вероятность нового гостя (%):', self.prob_spin)

    self.start_btn = QPushButton('Запустить симуляцию')
    self.start_btn.clicked.connect(self.toggle_simulation)
    self.start_btn.setStyleSheet(
        'background-color: #4CAF50; color: white; font-weight: bold; padding:'
        ' 8px;'
    )

    control_layout.addLayout(form_layout)
    control_layout.addWidget(self.start_btn)
    control_layout.addStretch()

    # Статус автомата
    self.status_label = QLabel('Автомат: Свободен')
    self.status_label.setStyleSheet(
        'font-size: 14px; font-weight: bold; color: #333;'
    )
    control_layout.addWidget(self.status_label)

    # Правая панель: Списки (Представления / Views)
    views_layout = QVBoxLayout()

    # Очередь
    q_label = QLabel('Очередь покупателей:')
    self.queue_view = QListView()
    self.queue_view.setModel(self.queue_model)

    # Довольные
    s_label = QLabel('Довольные покупатели (Успешно купили):')
    self.satisfied_view = QListView()
    self.satisfied_view.setModel(self.satisfied_model)

    # Недовольные
    d_label = QLabel('Недовольные покупатели (Ушли):')
    self.dissatisfied_view = QListView()
    self.dissatisfied_view.setModel(self.dissatisfied_model)

    views_layout.addWidget(q_label)
    views_layout.addWidget(self.queue_view)
    views_layout.addWidget(s_label)
    views_layout.addWidget(self.satisfied_view)
    views_layout.addWidget(d_label)
    views_layout.addWidget(self.dissatisfied_view)

    main_layout.addLayout(control_layout, 1)
    main_layout.addLayout(views_layout, 2)

    self.setCentralWidget(central_widget)
    self.setStatusBar(QStatusBar())
    self.statusBar().showMessage('Симуляция остановлена.')

  def toggle_simulation(self):
    if not self.timer.isActive():
      self.timer.start()
      self.start_btn.setText('Остановить симуляцию')
      self.start_btn.setStyleSheet(
          'background-color: #F44336; color: white; font-weight: bold; padding:'
          ' 8px;'
      )
      self.statusBar().showMessage('Симуляция запущенна...')
    else:
      self.timer.stop()
      self.start_btn.setText('Запустить симуляцию')
      self.start_btn.setStyleSheet(
          'background-color: #4CAF50; color: white; font-weight: bold; padding:'
          ' 8px;'
      )
      self.statusBar().showMessage('Симуляция остановлена.')

  def simulation_step(self):
    # 1. Генерация нового покупателя с заданной вероятностью
    if random.randint(1, 100) <= self.prob_spin.value():
      c_id = random.randint(100, 999)
      drink_id = random.choice(list(self.drinks.keys()))
      drink_info = self.drinks[drink_id]
      budget = random.randint(80, 250)
      max_wait = random.randint(5, 15)

      new_customer = Customer(
          name=f'Покупатель #{c_id}',
          drink_id=drink_id,
          drink_name=drink_info['name'],
          price=drink_info['price'],
          budget=budget,
          max_wait=max_wait,
      )
      self.queue_model.add_customer(new_customer)

    # 2. Обработка таймаутов ожидания в очереди
    i = 0
    while i < len(self.queue_model.customers):
      cust = self.queue_model.customers[i]
      # Если это не текущий обрабатываемый заказчик, увеличиваем время ожидания
      if cust != self.current_processing_customer:
        cust.current_wait += 1
        if cust.current_wait > cust.max_wait:
          cust.status_info = 'Долго стоял (ушел)'
          removed = self.queue_model.remove_customer(i)
          self.dissatisfied_model.add_customer(removed)
          continue
      i += 1

    # 3. Работа автомата
    if self.current_processing_customer is None:
      if len(self.queue_model.customers) > 0:
        # Берём первого из очереди
        candidate = self.queue_model.customers[0]
        # Проверяем бюджет
        if candidate.budget < candidate.price:
          candidate.status_info = 'Слишком дорого (ушел)'
          removed = self.queue_model.remove_customer(0)
          self.dissatisfied_model.add_customer(removed)
          self.status_label.setText('Автомат: Отказ (нет денег у клиента)')
        else:
          # Начинаем готовить
          self.current_processing_customer = self.queue_model.remove_customer(0)
          d_id = self.current_processing_customer.drink_id
          self.processing_time_left = self.drinks[d_id]['time']
          self.status_label.setText(
              f'Готовится: {self.current_processing_customer.drink_name} для'
              f' {self.current_processing_customer.name}'
          )
    else:
      # Процесс приготовления напитка
      self.processing_time_left -= 1
      if self.processing_time_left <= 0:
        self.current_processing_customer.status_info = 'Успешно куплен'
        self.satisfied_model.add_customer(self.current_processing_customer)
        self.current_processing_customer = None
        self.status_label.setText('Автомат: Свободен')

    # Обновление представлений
    self.queue_view.viewport().update()
    self.satisfied_view.viewport().update()
    self.dissatisfied_view.viewport().update()


if __name__ == '__main__':
  app = QApplication(sys.argv)
  window = CoffeeMachineWindow()
  window.show()
  sys.exit(app.exec())